#include <ArduinoBLE.h>

long previousMillis = 0;
int interval = 0;
int ledState = LOW;

BLEService ledService("180A");
BLEByteCharacteristic switchCharacteristic("2A57", BLERead | BLEWrite);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(LED_BUILTIN, OUTPUT);

 if (!BLE.begin()) {
   Serial.println("Starting BLE Failed!");
   while(1);
 }

 BLE.setLocalName("Narek's 33 IoT");
 BLE.setAdvertisedService(ledService);

 ledService.addCharacteristic(switchCharacteristic);

 BLE.addService(ledService);

 switchCharacteristic.writeValue(0);

 BLE.advertise();

 Serial.println("BLE LED Peripheral Advertising...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to BLE Central: ");
    Serial.println(central.address());

    while (central.connected()) {
      if (switchCharacteristic.written()) {
        switch (switchCharacteristic.value()) {
          case 01:
            Serial.println("LED on");
            digitalWrite(LED_BUILTIN, HIGH);
            break;
          case 02:
            Serial.println("LED fast blink");
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
            break;
          case 03:
            Serial.println("LED slow blink");
            digitalWrite(LED_BUILTIN, HIGH);
            delay(1000);
            digitalWrite(LED_BUILTIN, LOW);
            delay(1000);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(1000);
            digitalWrite(LED_BUILTIN, LOW);
            delay(1000);
            break;
          default:
            Serial.println("LED OFF");
            digitalWrite(LED_BUILTIN, LOW);
            break;
        }
      }
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, LOW);
  } 
}