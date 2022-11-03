// 1) Connect to same network (iPhone HotSpot)
// 2) Run UDP server with `node server.js`
// 3) Run arduino m3.ino program
// 4) Navigate to localhost:3000/

const { clear, time, Console } = require("node:console");
const dgram = require("node:dgram");
const express = require("express");

const app = express();
const webPort = 3000;

const udpServer = dgram.createSocket("udp4");
const udpPort = 41234;

// ******
// Setup UDP Server to listen to Arduino updates over WiFi
// ******

udpServer.on("error", (err) => {
  console.log(`server error:\n${err.stack}`);
  server.close();
});

let lastUpdatedSensorTime;
let sensorValues = {
  ax: 0,
  ay: 0,
  az: 0,
  gx: 0,
  gy: 0,
  gz: 0,
  phi: 0,
  theta: 0,
  psi: 0,
};

const alpha = 0.05;

let currentAttitudeEstimation = {
  roll: 0,
  pitch: 0,
};

udpServer.on("message", (msg, rinfo) => {
  sensorValues.ax = msg.readFloatLE();
  sensorValues.ay = msg.readFloatLE(4);
  sensorValues.az = msg.readFloatLE(8);
  sensorValues.gx = msg.readFloatLE(12);
  sensorValues.gy = msg.readFloatLE(16);
  sensorValues.gz = msg.readFloatLE(20);
  sensorValues.phi = msg.readFloatLE(24);
  sensorValues.theta = msg.readFloatLE(28);
  sensorValues.psi = msg.readFloatLE(32);

  // Basic Attitude Estimation using Complementary Filter
  const currentTime = new Date().getTime();

  if (lastUpdatedSensorTime === undefined) {
    currentAttitudeEstimation = {
      pitch: 0,
      roll: 0,
    };
  } else {
    const phi = currentAttitudeEstimation.roll;
    const theta = currentAttitudeEstimation.pitch;

    const deltaTime_ms = currentTime - lastUpdatedSensorTime;

    /*
      Get Attitude Estimation from Gyro
    */
    const phi_dot_gyro =
      sensorValues.gx +
      sensorValues.gy * Math.sin(phi) * Math.tan(theta) +
      sensorValues.gz * Math.cos(phi) * Math.tan(theta);
    const theta_dot_gyro =
      sensorValues.gy * Math.cos(phi) - sensorValues.gz * Math.sin(phi);

    const phi_gyro = phi + (deltaTime_ms / 1000) * phi_dot_gyro;
    const theta_gyro = theta + (deltaTime_ms / 1000) * theta_dot_gyro;

    /*
      Get Attitude Estimation from Accelerometer
    */
    const phi_accel = Math.atan2(sensorValues.ay, sensorValues.az);
    const theta_accel = Math.asin(sensorValues.ax / 9.8);

    /*
      Complementary
    */

    // currentAttitudeEstimation = {
    //   pitch: alpha * theta_accel + (1 - alpha) * theta_gyro,
    //   roll: alpha * phi_accel + (1 - alpha) * phi_gyro,
    // };
    currentAttitudeEstimation = {
      pitch: theta_gyro,
      roll: phi_gyro,
    };
  }

  lastUpdatedSensorTime = currentTime;

  // displayStats(
  //   sensorValues.ax,
  //   sensorValues.ay,
  //   sensorValues.az,
  //   sensorValues.gx,
  //   sensorValues.gy,
  //   sensorValues.gz
  // );

  console.log(
    `Roll: ${currentAttitudeEstimation.roll}, Pitch: ${currentAttitudeEstimation.pitch}`
  );
});

udpServer.on("listening", () => {
  const address = udpServer.address();
  console.log(`UDP Server listening ${address.address}:${address.port}`);
});

udpServer.bind(udpPort);

const intl = new Intl.NumberFormat("en-US", {
  signDisplay: "always",
  minimumFractionDigits: 4,
  minimumFractionDigits: 4,
  minimumIntegerDigits: 3,
});

const displayStats = (ax, ay, az, gx, gy, gz) => {
  clear();
  console.log("\u001B[?25l");
  ax = intl.format(ax);
  ay = intl.format(ay);
  az = intl.format(az);
  gx = intl.format(gx);
  gy = intl.format(gy);
  gz = intl.format(gz);
  console.table([{ ax, ay, az, gx, gy, gz }]);
};

// ******
// Setup HTTP Server to server webpage and api
// ******

app.use(express.static("frontend"));

// API to poll current Arduino IMU values
app.post("/", (req, res) => {
  res.json(sensorValues);
});

app.listen(webPort, () => {
  console.log(`HTTP Server listening on port ${webPort}`);
});
