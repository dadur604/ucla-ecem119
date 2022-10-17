// 1) Connect to same network (iPhone HotSpot)
// 2) Run UDP server with `node server.js`
// 3) Run arduino m2.ino program

const { clear } = require("node:console");
const dgram = require("node:dgram");

const server = dgram.createSocket("udp4");

server.on("error", (err) => {
  console.log(`server error:\n${err.stack}`);
  server.close();
});

server.on("message", (msg, rinfo) => {
  const ax = msg.readFloatLE();
  const ay = msg.readFloatLE(4);
  const az = msg.readFloatLE(8);
  const gx = msg.readFloatLE(12);
  const gy = msg.readFloatLE(16);
  const gz = msg.readFloatLE(20);

  displayStats(ax, ay, az, gx, gy, gz);
});

server.on("listening", () => {
  const address = server.address();
  console.log(`server listening ${address.address}:${address.port}`);
});

server.bind(41234);

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
