#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Arduino_LSM6DS3.h>
#include <MCP42.h>

// Wifi Access Point Setup
char ssid[] = "WahFi";
char pass[] = "00000000";

int udpPort = 600;

int wifi_status = WL_IDLE_STATUS;

unsigned long previousMillis = 0;

WiFiUDP wifiUdp;

// UDP Packet receive buffer
byte udpPacketBuffer[7];

boolean pedal0_on_int = false;
boolean pedal1_on_int = false;
int wahPedalOnRelayPin = 3;

// Digital Potentiometer Setup
// Note: Actually using an MCP42100
int digiPot_CS = 2; // ChipSelect Pin
boolean volPotNumber = 0;
boolean wahPotNumber = 1;

MCP42 digitalPot(digiPot_CS);

void setup()
{
  Serial.begin(9600);
  // while (!Serial);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(wahPedalOnRelayPin, OUTPUT);

  Serial.print("Starting Access Point: ");
  Serial.println(ssid);

  wifi_status = WiFi.beginAP(ssid, pass, 14);
  if (wifi_status == WL_CONNECT_FAILED)
  {
    Serial.println("Failed to initialize Wifi AP");
    while (1)
      ;
  }

  delay(500);

  if (!wifiUdp.begin(udpPort))
  {
    Serial.print("Failed to initialize WiFi UDP on port ");
    Serial.println(udpPort);
    while (1)
      ;
  }

  delay(1);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("Created Network");
  Serial.println("--------------------");

  Serial.println("Information: ");
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println();

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  Serial.println("--------------------");

  Serial.println("Starting Digital Potentiometer");
  digitalPot.begin();

  delay(20);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
  int size = wifiUdp.parsePacket();
  if (size > 0)
  {

    // UDP Packet Format:
    // |----|-|-|-|
    // Pedal Angle - 4 Bytes
    // Current Pedal - 1 Byte
    // Pedal 1 On - 1 Byte
    // Pedal 2 On - 1 Byte

    wifiUdp.read(udpPacketBuffer, 7);
    // Read first 4 bytes of packet as value
    float value = *(float *)udpPacketBuffer;
    // flip 0<->1
    value *= -1;
    value += 1;
    value *= 255;
    if (value < 0)
      value = 0;
    if (value > 255)
      value = 255;

    byte currentPedal = udpPacketBuffer[4];
    boolean pedal0_on = udpPacketBuffer[5];
    boolean pedal1_on = udpPacketBuffer[6];

    Serial.print(static_cast<uint8_t>(value));
    Serial.print(" ");
    Serial.print(currentPedal);
    Serial.print(" ");
    Serial.print(pedal0_on);
    Serial.print(" ");
    Serial.println(pedal1_on);

    // Enable/Disable Wah
    if (pedal1_on != pedal1_on_int)
    {
      pedal1_on_int = pedal1_on;
      digitalWrite(wahPedalOnRelayPin, pedal1_on ? HIGH : LOW);
    }

    if (pedal0_on != pedal0_on_int)
    {
      pedal0_on_int = pedal0_on;

      // If Volume pedal is off, write a default of 255
      if (!pedal0_on)
      {
        digitalPot.DigitalPotSetWiperPosition(volPotNumber, 255);
      }
    }

    // Volume Control
    if (currentPedal == 0 && pedal0_on_int)
    {
      float logPosition;
      if (value >= 255)
      {
        logPosition = 255;
      }
      else
      {
        logPosition = (255 * pow(10, ((value - 255) / 64.0)));
      }

      digitalPot.DigitalPotSetWiperPosition(volPotNumber, static_cast<uint8_t>(logPosition));
      Serial.println(static_cast<uint8_t>(logPosition));
    }
    // Wah Control
    else if (currentPedal == 1)
    {
      digitalPot.DigitalPotSetWiperPosition(wahPotNumber, static_cast<uint8_t>(value));
    }

    // Serial.println(static_cast<uint8_t>(value));
    // Serial.println(pedalOn);
    // Serial.print("Got packet of size ");Serial.println(size);
    // Serial.print("Wah: "); Serial.print(*(float*)udpPacketBuffer);
    // Serial.print(", DT: "); Serial.println(*(boolean*)udpPacketBuffer[4]);
  }
}