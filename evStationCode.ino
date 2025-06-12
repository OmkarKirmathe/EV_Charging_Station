#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

// Pin Definitions
#define IR_SENSOR_PIN 2
#define SERVO_PIN 3
#define RELAY_PIN 4
#define SS_PIN 10
#define RST_PIN 9

// RFID Tag for Authorization (Replace with your tag UID)
byte authorizedUID[] = {131, 170, 92, 245}; // Example UID

// Objects for peripherals
Servo gateServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);

unsigned long chargingStartTime = 0;
bool isCharging = false;

void setup() {
  // Initialize peripherals
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  gateServo.attach(SERVO_PIN);
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.begin(16, 2);
  lcd.backlight();

  // Set initial states
  gateServo.write(0);  // Gate closed
  digitalWrite(RELAY_PIN, LOW);  // Charging off
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("EV Charging Station");
  delay(2000);
}

void loop() {
  // Step 1: Check if car is detected
  if (digitalRead(IR_SENSOR_PIN) == LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Car Detected...");
    openGate();
  }

  // Step 2: Wait for RFID Scan
  if (!isCharging) {  // Only display if not charging
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan RFID to");
    lcd.setCursor(0, 1);
    lcd.print("start charging");
  }

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    if (isAuthorized()) {
      if (!isCharging) {
        startCharging();
      } else {
        stopCharging();
        generateBill();
      }
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wrong RFID!");
      delay(2000);
    }
    mfrc522.PICC_HaltA();
  }

  // Step 3: Continuously display "Charging..." while charging
  if (isCharging) {
    lcd.setCursor(0, 0);
    lcd.print("Charging.. ");
    lcd.setCursor(0, 1);
    lcd.print("Time: ");
    lcd.print((millis() - chargingStartTime) / 1000); // Display charging time in seconds
    lcd.print(" s   ");
  }
}

void openGate() {
  gateServo.write(90);  // Open gate
  delay(3000);          // Wait for the car to enter
  gateServo.write(0);   // Close gate
}

bool isAuthorized() {
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] != authorizedUID[i]) {
      return false;
    }
  }
  return true;
}

void startCharging() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Charging Started");
  digitalWrite(RELAY_PIN, HIGH);  // Start charging
  chargingStartTime = millis();
  isCharging = true;
  delay(2000);  // Display "Charging Started" for 2 seconds
}

void stopCharging() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Charging Stopped");
  digitalWrite(RELAY_PIN, LOW);  // Stop charging
  isCharging = false;
  delay(2000);
}

void generateBill() {
  unsigned long chargingTime = (millis() - chargingStartTime) / 1000;  // Time in seconds
  float cost = chargingTime * 0.05;  // Example rate: 0.05 per second

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Charging Time:");
  lcd.setCursor(0, 1);
  lcd.print(chargingTime);
  lcd.print(" s");

  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bill: Rs ");
  lcd.print(cost);
  delay(5000);
}
