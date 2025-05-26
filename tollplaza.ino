#include <LiquidCrystal.h>
#include <Servo.h>

// Define IR sensor logic: active-low
#define IR_ACTIVE_LOW 1

// Pins
const int irPin = 7;
const int servoPin = 10;
const int contrastPin = 6;

// Gate servo
Servo gateServo;

// LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int contrast = 100;

// Variables
bool lastState = (IR_ACTIVE_LOW ? HIGH : LOW);
int vehicleCount = 0;

void setup() {
  Serial.begin(9600); // For communication with ESP32

  pinMode(irPin, INPUT);
  pinMode(contrastPin, OUTPUT);
  analogWrite(contrastPin, contrast);

  gateServo.attach(servoPin);
  gateServo.write(90); // Gate initially closed

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Toll Plaza Ready");
  lcd.setCursor(0, 1);
  lcd.print("Count: 0");
}

void loop() {
  int irState = digitalRead(irPin);

  if (IR_ACTIVE_LOW ? (irState == LOW && lastState == HIGH) : (irState == HIGH && lastState == LOW)) {
    handleVehicle();  // Triggered only once per car
  }

  // Keep gate open while vehicle is detected
  if (IR_ACTIVE_LOW ? irState == LOW : irState == HIGH) {
    openGate();
  } else {
    delay(1500);
    closeGate();
  }

  lastState = irState;
  delay(100); // Short delay for stability
}

void handleVehicle() {
  vehicleCount++;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Don Toll PLaza");
  lcd.setCursor(0, 1);
  lcd.print("Count: ");
  lcd.print(vehicleCount);

  Serial.println("CAR_DETECTED");
}

void openGate() {
  gateServo.write(145); // Adjust if needed
}

void closeGate() {
  gateServo.write(90);
}
