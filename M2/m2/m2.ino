#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Arduino_LSM6DS3.h>

char ssid[] = "Narek";
char pass[] = "00000000";

int udpPort = 600;
char remoteIP[] = "172.20.10.2";
int remotePort = 41234;

int status = WL_IDLE_STATUS;
int ledState = LOW;
unsigned long previousMillisInfo = 0;
unsigned long previousMillisLED = 0;
const int intervalInfo = 100;

float ax,ay,az,gx,gy,gz = 0;

WiFiUDP wifiUdp;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(LED_BUILTIN, OUTPUT);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while(1);
  }

  if (!wifiUdp.begin(udpPort)) {
    Serial.print("Failed to initialize WiFi UDP on port ");
    Serial.println(udpPort);
    while(1);
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);

    delay(10000);
  }

  Serial.println("Connected to network");
  Serial.println("--------------------");

  Serial.println("Information: ");
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println();

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  Serial.println("--------------------");
}

void loop() {
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax,ay,az);
  }
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gx,gy,gz);
  }

  unsigned long currentMillisInfo = millis();

  if (currentMillisInfo - previousMillisInfo >= intervalInfo) {
    previousMillisInfo = currentMillisInfo;

    sendUDPPacket();
  }

  unsigned long currentMillisLED = millis();

  int intervalLED = WiFi.RSSI() * -10;

  if (currentMillisLED - previousMillisLED >= intervalLED) {
    previousMillisLED = currentMillisLED;

    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    digitalWrite(LED_BUILTIN, ledState);
  }
}

void sendUDPPacket() {
  Serial.println("Writing UDP packet...");

  if (!wifiUdp.beginPacket(remoteIP, remotePort)) {
    Serial.print("Failed call to beginPacket() on IP ");
    Serial.print(remoteIP);
    Serial.print(": ");
    Serial.println(remotePort);
    return;
  }

  wifiUdp.write((byte *) &ax, 4);
  wifiUdp.write((byte *) &ay, 4);
  wifiUdp.write((byte *) &az, 4);
  wifiUdp.write((byte *) &gx, 4);
  wifiUdp.write((byte *) &gy, 4);
  wifiUdp.write((byte *) &gz, 4);

  if (!wifiUdp.endPacket()) {
    Serial.println("Failed call to endPacket()");
    return;
  }
}