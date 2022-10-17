#include <Arduino_LSM6DS3.h>

void setup() {
  Serial.begin(9600);
  while(!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while(true);
  }
}

float ax,ay,az,gx,gy,gz = 0;

void loop() {
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax,ay,az);
  }
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gx,gy,gz);
  }

  Serial.print(ax, 3);Serial.print(" ");
  Serial.print(ay, 3);Serial.print(" ");
  Serial.print(az, 3);Serial.print(" ");
  Serial.print(gx, 3);Serial.print(" ");
  Serial.print(gy, 3);Serial.print(" ");
  Serial.print(gz, 3);Serial.print(" ");
  Serial.println();
}
