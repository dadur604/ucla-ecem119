#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Arduino_LSM6DS3.h>
#include <Array.h>

// Change to WahFi
char ssid[] = "WahFi";
char pass[] = "00000000";

int udpPort = 600;
char remoteIP[] = "192.168.4.1";
int remotePort = 600;

int wifi_status = WL_IDLE_STATUS;

WiFiUDP wifiUdp;


void setup()
{
  //Serial.begin(9600);
  //while (!Serial);

  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }

  if (!wifiUdp.begin(udpPort))
  {
    Serial.print("Failed to initialize WiFi UDP on port ");
    Serial.println(udpPort);
    while (1)
      ;
  }

  while (wifi_status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);

    wifi_status = WiFi.begin(ssid, pass);
    delay(500);
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

void loop()
{
  sendUDPPacket();
  delay(100);
}

void sendUDPPacket()
{
  if (!wifiUdp.beginPacket(remoteIP, remotePort))
  {
    Serial.print("Failed call to beginPacket() on IP ");
    Serial.print(remoteIP);
    Serial.print(": ");
    Serial.println(remotePort);
    return;
  }

  int currentMillis = millis()
  wifiUdp.write((byte *)&currentMillis, 4);

  if (!wifiUdp.endPacket())
  {
    // Serial.println("Failed call to endPacket()");
    return;
  }
}

void blink(int num)
{
  for (int i = 0; i < num; i++)
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
}
