// 1) Connect to same network (iPhone HotSpot)
// 2) Run UDP server with `node server.js`
// 3) Run arduino m2.ino program
// 4) Navigate to localhost:3000/

const { clear } = require("node:console");
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

let sensorValues = {
  ax: 0,
  ay: 0,
  az: 0,
  gx: 0,
  gy: 0,
  gz: 0,
};

udpServer.on("message", (msg, rinfo) => {
  sensorValues.ax = msg.readFloatLE();
  sensorValues.ay = msg.readFloatLE(4);
  sensorValues.az = msg.readFloatLE(8);
  sensorValues.gx = msg.readFloatLE(12);
  sensorValues.gy = msg.readFloatLE(16);
  sensorValues.gz = msg.readFloatLE(20);

  // displayStats(
  //   sensorValues.ax,
  //   sensorValues.ay,
  //   sensorValues.az,
  //   sensorValues.gx,
  //   sensorValues.gy,
  //   sensorValues.gz
  // );
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
