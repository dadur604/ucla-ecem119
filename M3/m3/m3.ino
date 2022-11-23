#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Arduino_LSM6DS3.h>

char ssid[] = "Narek";
char pass[] = "00000000";

int udpPort = 600;
char remoteIP[] = "192.168.137.1";
// char remoteIP[] = "172.20.10.2";
int remotePort = 41234;

int status = WL_IDLE_STATUS;
int ledState = LOW;
unsigned long previousMillisInfo = 0;
unsigned long previousMillisLED = 0;
unsigned long previousMillis = 0;
const int intervalInfo = 100;

float ax,ay,az,gx,gy,gz = 0;
float ax1,ay1,az1,gx1,gy1,gz1 = 0;
float axf,ayf,azf,gxf,gyf,gzf = 0;

float phi,theta,psi = 0;

WiFiUDP wifiUdp;

void setup() {
  Serial.begin(9600);
  //while (!Serial);

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
  digitalWrite(LED_BUILTIN, HIGH);
  delay(5000);
  digitalWrite(LED_BUILTIN, LOW);
}

const float G = 9.806;
const float alpha = 0.02;
//const float DEG_TO_RAD = 3.1415 / 180;

// ******
// What: Sensor Fusion / Complementary Filter Math / Digital Low Pass Filter
// Where: youtube.com/watch?v=whSw42XddsU
// Where: youtube.com/watch?v=BUW2OdAtzBw
// Where youtube.com/watch?v=HJ-C4Incgpw
// Why: I didn't know about how to correctly do sensor fusion 
// ALL CODE was written by me, I only used the above videos for general system block diagrams, math, and psuedocode
// *****

void loop() {
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax,ay,az);
    // Convert G's to m/s2
    ax *= G; ay *= G; az *= G;

    // Lowpass Filter
    axf = 0.95*axf + 0.025*ax + 0.025*ax1;
    ayf = 0.95*ayf + 0.025*ay + 0.025*ay1;
    azf = 0.95*azf + 0.025*az + 0.025*az1;
    ax1 = ax;
    ay1 = ay;
    az1 = az;
  }
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gx,gy,gz);
    // Convert deg/s to rad/s
    gx *= DEG_TO_RAD; gy *= DEG_TO_RAD; gz *= DEG_TO_RAD;

    // Lowpass Filter
    gxf = 0.95*gxf + 0.025*gx + 0.025*gx1;
    gyf = 0.95*gyf + 0.025*gy + 0.025*gy1;
    gzf = 0.95*gzf + 0.025*gz + 0.025*gz1;
    gx1 = gx;
    gy1 = gy;
    gz1 = gz;
  }

  unsigned long currentMillisInfo = millis();
  unsigned long dt = currentMillisInfo - previousMillis;
  previousMillis = currentMillisInfo;

  if (previousMillis == 0) {

  } else {
    // *Attitude Estimation from Gyro* //

    // Convert from body-space to world-space
    float dphi_gyro = gxf + 
                      gyf * sin(phi) * tan(theta) +
                      gzf * cos(phi) * tan(theta);
    float dtheta_gyro = gyf * cos(phi) -
                        gzf * sin(phi);
    float dpsi_gyro = (gyf * sin(psi) / cos(theta)) +
                      (gzf * cos(psi) / cos(theta));

    dpsi_gyro *= -1;

    // Numerical Integration
    float phi_gyro = phi + (dt / 1000.0f) * dphi_gyro;
    float theta_gyro = theta + (dt / 1000.0f) * dtheta_gyro;
    float psi_gyro = psi + (dt / 1000.0f) * dpsi_gyro;

    // *Attitude Estimation from Accel* //

    float phi_accel = atan2(ayf, azf);
    float theta_accel = asin(max(-1, min(1, axf / G)));

    // *Complementary Filter* //
    phi = alpha * phi_accel + (1-alpha) * phi_gyro;
    theta = alpha * theta_accel + (1-alpha) * theta_gyro;
    psi = 0.995 * psi_gyro;

    Serial.print("Phi: "); Serial.print(phi);
    Serial.print(",Theta: "); Serial.print(theta);
    Serial.print(",Psi: "); Serial.println(psi);
  }


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
  //Serial.println("Writing UDP packet...");

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
  wifiUdp.write((byte *) &phi, 4);
  wifiUdp.write((byte *) &theta, 4);
  wifiUdp.write((byte *) &psi, 4);


  if (!wifiUdp.endPacket()) {
    Serial.println("Failed call to endPacket()");
    return;
  }
}