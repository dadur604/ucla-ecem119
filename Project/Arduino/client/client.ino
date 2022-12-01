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

unsigned long previousMillis = 0;

const float MIN_TAP_ACCEL = 5;  // rad/s2
const float MIN_TAP_JERK = 130; // m/s2/s
const float TAP_PHI_BIAS = 0.01;
const float MIN_TAP_DELAY = 160; // Milliseconds; Min delay between two "taps" (smoothing)
const float MAX_TAP_DELAY = 500; // Milliseconds; Max delay between two taps to count as double-tap
const float MIN_SWITCH_DELAY = 1500;
unsigned long previousSwitchMillis = 0;
unsigned long previousTapMillis = 0;
unsigned long previousSwitchTapMillis = 0;
unsigned long previousTapMillisGhost = 0;
unsigned long previousSwitchMillisGhost = 0;

// Raw IMU Values
float ax,
    ay, az, gx, gy, gz = 0;

// Acceleration in Phi
float ddphi = 0;
float ddphi1 = 0;
float ddphif = 0;

// Jerk in AX
float dax, dax1, daxf, daxf1, daxfabs = 0;

// Jerk in AZ
float daz, daz1, dazf = 0;

// Low-Pass Filter
float ax1, ay1, az1, gx1, gy1, gz1 = 0;
float ax2, ay2, az2, gx2, gy2, gz2 = 0;

// Filtered IMU Values
float axf, ayf, azf, gxf, gyf, gzf = 0;
float axf1, ayf1, azf1, gxf1, gyf1, gzf1 = 0;

// Attitude Estimation
float phi, theta, psi = 0;
float phi_corrected = 0;

// Whether we had a doubletap event this tick
boolean doubletap = 0;
// Whether we had a pedal switch even this tick
boolean pedalswitch = 0;

WiFiUDP wifiUdp;

enum CALIB_STATUS
{
  PRE_CALIBRATION,
  CALIBRATING_0,
  CALIBRATING_1,
  STREAMING
};

CALIB_STATUS calibration_status = PRE_CALIBRATION;
float calibration_0, calibration_1;

const int MAX_CALIBRATION_WINDOW_SIZE = 50;
Array<float, MAX_CALIBRATION_WINDOW_SIZE> calibration_window_phi;
int skipped = 0;
int toSkip = 100;

boolean pedal0_on = true;
boolean pedal1_on = false;

int pedal0_led = 2;
int pedal1_led = 3;

// Pedal 0: Volume
// Pedal 1: Wah
byte currentPedal = 0;

unsigned long lastPedalSwitchTime = millis();

void setup()
{
  //Serial.begin(9600);
  //while (!Serial);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pedal0_led, OUTPUT);
  pinMode(pedal1_led, OUTPUT);

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

  digitalWrite(LED_BUILTIN, LOW);

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

  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);

  calibration_status = PRE_CALIBRATION;
}

const float G = 9.806;
const float alpha = 0.02;

/**
 * Low Pass Filter
 * digital Butterworth filter
 * Sampling Frequency: 100Hz
 * 3db Cuttoff: 10Hz
 * 4.078⋅y[i] = x[i] + x[i-1] + 2.078 y[i-1]
 */

void loop()
{
  if (IMU.accelerationAvailable())
  {
    IMU.readAcceleration(ax, ay, az);
    // Convert G's to m/s2
    ax *= G;
    ay *= G;
    az *= G;

    // Lowpass Filter

    axf = 0.5 * axf + 0.25 * ax + 0.25 * ax1;
    ayf = 0.5 * ayf + 0.25 * ay + 0.25 * ay1;
    azf = 0.5 * azf + 0.25 * az + 0.25 * az1;

    ax1 = ax;
    ay1 = ay;
    az1 = az;
  }
  if (IMU.gyroscopeAvailable())
  {
    IMU.readGyroscope(gx, gy, gz);
    // Convert deg/s to rad/s
    gx *= DEG_TO_RAD;
    gy *= DEG_TO_RAD;
    gz *= DEG_TO_RAD;

    // Lowpass Filter
    gxf = 0.5 * gxf + 0.25 * gx + 0.25 * gx1;
    gyf = 0.5 * gyf + 0.25 * gy + 0.25 * gy1;
    gzf = 0.5 * gzf + 0.25 * gz + 0.25 * gz1;

    gx1 = gx;
    gy1 = gy;
    gz1 = gz;
  }

  unsigned long currentMillis = millis();
  unsigned long dt = currentMillis - previousMillis;
  previousMillis = currentMillis;

  if (previousMillis == 0)
  {
  }
  else
  {
    float dphi_gyro = gxf;
    float dtheta_gyro = gyf * cos(phi) -
                        gzf * sin(phi);

    // Numerican Differentiation and Low Pass Filter for Acceleration in Phi
    ddphi = (gxf - gxf1) / (dt / 1000.0f);
    ddphif = 0.95 * ddphif + 0.025 * abs(ddphi) + 0.025 * ddphi1;
    ddphi1 = ddphi;

    // Numerical Differentiation and Low Pass Filter for Jerk in X
    dax = (axf - axf1) / (dt / 1000.0f);
    daxf = 0.95 * daxf + 0.025 * dax + 0.025 * dax1;
    daxfabs = 0.95 * daxfabs + 0.025 * abs(dax) + 0.025 * abs(dax1);
    dax1 = dax;

    // Numerical Differentiation and Low Pass Filter for Jerk in Z
    daz = (azf - azf1) / (dt / 1000.0f);
    dazf = 0.95 * dazf + 0.025 * abs(daz) + 0.025 * daz1;
    daz1 = daz;

    // Numerical Integration
    float phi_gyro = phi + (dt / 1000.0f) * dphi_gyro;
    float theta_gyro = theta + (dt / 1000.0f) * dtheta_gyro;

    // *Attitude Estimation from Accel* //
    float phi_accel = atan2(ayf, azf);
    float theta_accel = asin(max(-1, min(1, axf / G)));

    // *Complementary Filter* //
    phi = alpha * phi_accel + (1 - alpha) * phi_gyro;
    theta = alpha * theta_accel + (1 - alpha) * theta_gyro;

    // Correct phi using calibration
    phi_corrected = (phi - calibration_0) / (calibration_1 - calibration_0);

    // *Double-Tap Detection* //
    pedalswitch = 0;
    doubletap = 0;

    // First check for pedal switch
    if (daxf1 > 0 && daxf <= 0 && daxfabs >= 100)
    {

      float dt = currentMillis - previousSwitchTapMillis;
      float dt_ghost = currentMillis - previousSwitchMillisGhost;

      // If it's a "ghost" tap, ignore it completely (don't even update last tap time)
      if (dt_ghost >= MIN_TAP_DELAY)
      {

        // Simple logistic classifier model trained using Weka
        float isTap = -228.0437 * theta +
                      10.7874 * dtheta_gyro -
                      7.1141 * dphi_gyro -
                      35.8177 * phi -
                      2.6505 * axf -
                      3.6468 * azf -
                      0.3069 * dazf -
                      88.4768;

        // Positive class corresponds to a tap
        if (isTap > 0)
        {
          if (dt <= MAX_TAP_DELAY && currentMillis - previousSwitchMillis >= MIN_SWITCH_DELAY)
          {
            pedalswitch = 1;
            // Serial.println("PEDAL SWITCH");
            previousSwitchTapMillis = 0;
            previousSwitchMillis = currentMillis;
          }
          else
          {
            previousSwitchTapMillis = currentMillis;
          }
        }
        previousSwitchMillisGhost = currentMillis;
      }
    }

    // If acceleration went from negative to positive;
    // Don't check for tap if pedal switch was tapped
    if (gxf1 < 0 && gxf >= 0 && ddphif >= 5)
    {

      float dt = currentMillis - previousTapMillis;
      float dt_ghost = currentMillis - previousTapMillisGhost;

      // If it's a "ghost" tap, ignore it completely (don't even update last tap time)
      if (dt_ghost >= MIN_TAP_DELAY)
      {

        // Simple logistic classifier model trained using Weka
        float isTap =
            0.0071 * ddphif +
            3.7296 * phi - calibration_0 +
            0.0347 * dazf -
            0.7854 * gxf +
            0.1258 * azf -
            6.6645;

        // Positive class corresponds to a tap
        if (isTap > 0)
        {
          if (dt <= MAX_TAP_DELAY && !pedalswitch)
          {
            doubletap = 1;
            // Serial.println("DOUBLE TAP");
            previousTapMillis = 0;
          }
          else
          {
            previousTapMillis = currentMillis;
          }
          previousTapMillisGhost = currentMillis;
        }
      }
    }

    if (pedalswitch)
    {
      if (currentPedal == 0)
      {
        currentPedal = 1;
      }
      else
      {
        currentPedal = 0;
      }
    }
    if (doubletap)
    {
      if (currentPedal == 0)
        pedal0_on = !pedal0_on;
      else
        pedal1_on = !pedal1_on;
    }

    // Serial.print("phi: "); Serial.print(phi);
    // Serial.print(",ddphi: "); Serial.print(ddphif/100.f);
    // Serial.print(",dt: "); Serial.println(doubletap);

    if (calibration_status == STREAMING)
    {
      sendUDPPacket();

      // dimly light the current pedal
      // strongly light the current pedal if it's active
      if (currentPedal == 0)
      {
        analogWrite(pedal0_led, pedal0_on ? 50 : 2);
        analogWrite(pedal1_led, 0);
      }
      else
      {
        analogWrite(pedal1_led, pedal1_on ? 50 : 2);
        analogWrite(pedal0_led, 0);
      }

      // Switch pedals every ten seconds
      // if (millis() - lastPedalSwitchTime > 10000)
      // {
      //   lastPedalSwitchTime = millis();
      //   if (currentPedal == 1)
      //     currentPedal = 0;
      //   else
      //     currentPedal = 1;
      // }
    }
    else if (calibration_status == CALIBRATING_0 || calibration_status == CALIBRATING_1)
    {
      if (skipped >= toSkip)
      {
        calibration_window_phi.push_back(phi);
      }
      else
      {
        skipped++;
      }

      if (calibration_window_phi.size() == MAX_CALIBRATION_WINDOW_SIZE)
      {
        float avg = 0;
        for (int i = 0; i < MAX_CALIBRATION_WINDOW_SIZE; i++)
        {
          avg += calibration_window_phi[i];
        }
        avg /= MAX_CALIBRATION_WINDOW_SIZE;

        if (calibration_status == CALIBRATING_0)
        {
          calibration_0 = avg;
          calibration_window_phi.clear();
          skipped = 0;
          calibration_status = CALIBRATING_1;

          Serial.print("Calibration 0: ");
          Serial.println(calibration_0);

          // Delay Before Starting
          digitalWrite(LED_BUILTIN, LOW);
          delay(1000);

          // Blink LED 3 times before starting calibration 2
          blink(3);
        }
        else
        {
          calibration_1 = avg;
          calibration_status = STREAMING;

          Serial.print("Calibration 1: ");
          Serial.println(calibration_1);

          // Delay Before Starting
          digitalWrite(LED_BUILTIN, LOW);
          delay(1000);

          // Blink LED 3 times before starting streaming
          blink(3);
        }
      }
    }
    else if (calibration_status == PRE_CALIBRATION)
    {
      // Blink LED 3 times before starting calibration
      blink(3);
      calibration_status = CALIBRATING_0;
    }

    gxf1 = gxf;
    azf1 = azf;
    axf1 = axf;
    daxf1 = daxf;
    delay(1);
  }
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

  // UDP Packet Format:
  // |----|-|-|-|
  // Pedal Angle - 4 Bytes
  // Current Pedal - 1 Byte
  // Pedal 1 On - 1 Byte
  // Pedal 2 On - 1 Byte

  wifiUdp.write((byte *)&phi_corrected, 4);
  wifiUdp.write((byte *)&currentPedal, 1);
  wifiUdp.write((byte *)&pedal0_on, 1);
  wifiUdp.write((byte *)&pedal1_on, 1);

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
