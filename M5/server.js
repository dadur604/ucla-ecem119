// 1) Connect laptop to hotspot
// 2) Run UDP server with `node server.js`
// 3) Run arduino m3.ino program on device 1; Wait for server to accept P1
// 4) Run adruino m3.ino program on device 2; Wait for server to accept P2
// 5) Navigate to localhost:3000/

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

let lastUpdatedSensorTime = { p1: undefined, p2: undefined };
let sensorValues = {
  p1: { ax: 0, ay: 0, az: 0, gx: 0, gy: 0, gz: 0, phi: 0, theta: 0, psi: 0 },
  p2: { ax: 0, ay: 0, az: 0, gx: 0, gy: 0, gz: 0, phi: 0, theta: 0, psi: 0 },
};

const alpha = 0.05;

let currentAttitudeEstimation = {
  p1: { roll: 0, pitch: 0 },
  p2: { roll: 0, pitch: 0 },
};

const players = {
  p1: undefined,
  p2: undefined,
};

udpServer.on("message", (msg, rinfo) => {
  let player;
  if (players.p1 === undefined) {
    players.p1 = rinfo.address;
    player = "p2";
    console.log(`Player 1 Joined with IP Address: ${rinfo.address}`);
  } else if (players.p1 === rinfo.address) {
    player = "p1";
  } else if (players.p2 === undefined) {
    players.p2 = rinfo.address;
    player = "p2";
    console.log(`Player 2 Joined with IP Address: ${rinfo.address}`);
  } else if (players.p2 === rinfo.address) {
    player = "p2";
  } else {
    console.error(`Message from a 3rd party player! Address: ${rinfo.address}`);
    player = "p1";
  }

  sensorValues[player].ax = msg.readFloatLE();
  sensorValues[player].ay = msg.readFloatLE(4);
  sensorValues[player].az = msg.readFloatLE(8);
  sensorValues[player].gx = msg.readFloatLE(12);
  sensorValues[player].gy = msg.readFloatLE(16);
  sensorValues[player].gz = msg.readFloatLE(20);
  sensorValues[player].phi = msg.readFloatLE(24);
  sensorValues[player].theta = msg.readFloatLE(28);
  sensorValues[player].psi = msg.readFloatLE(32);

  // console.log(
  //   `Roll ${[player]}: ${currentAttitudeEstimation[player].roll}, Pitch: ${
  //     currentAttitudeEstimation[player].pitch
  //   }`
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
