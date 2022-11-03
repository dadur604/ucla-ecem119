var Engine = Matter.Engine,
  Render = Matter.Render,
  Runner = Matter.Runner,
  Bodies = Matter.Bodies,
  Body = Matter.Body,
  Events = Matter.Events,
  Composite = Matter.Composite;

const width = 800;
const height = 600;
const paddleWidth = 20;
const paddleHeight = 100;
const ballRadius = 20;

// create an engine
var engine = Engine.create({
  gravity: {
    x: 0,
    y: 0,
  },
});

// create a renderer
var render = Render.create({
  element: document.body,
  engine: engine,
  options: {
    width,
    height,
    wireframes: false,
  },
});

// create player paddle
var rightPaddle = Bodies.rectangle(
  width - paddleWidth,
  height / 2,
  paddleWidth,
  paddleHeight,
  {
    isStatic: true,
    restitution: 1,
    friction: 0,
    render: { fillStyle: "0xffffff" },
  }
);

// create left "paddle"
var leftPaddle = Bodies.rectangle(
  paddleWidth,
  height / 2,
  paddleWidth,
  height,
  {
    isStatic: true,
    restitution: 1,
    friction: 0,
  }
);

// create floor and ceiling
var ceiling = Bodies.rectangle(width / 2, 0, width, 10, {
  isStatic: true,
  restitution: 1,
  friction: 0,
});
var floor = Bodies.rectangle(width / 2, height, width, 10, {
  isStatic: true,
  restitution: 1,
  friction: 0,
});

var ball = Bodies.circle(width / 2, height / 2, ballRadius, {
  isStatic: false,
  friction: 0,
  inertia: Infinity,
  frictionAir: 0,
  restitution: 1,
});

Body.applyForce(ball, { x: 0, y: 0 }, { x: 0.01, y: 0.0 });

// add all of the bodies to the world
Composite.add(engine.world, [ball, rightPaddle, leftPaddle, floor, ceiling]);

// run the renderer
Render.run(render);

// create runner
var runner = Runner.create();

// run the engine
Runner.run(runner, engine);

Events.on(runner, "beforeUpdate", () => {
  Body.setPosition(rightPaddle, {
    x: width - paddleWidth,
    y: height / 2 + animatedValues.rightPaddlePosition * (height / 2),
  });
});

let animatedValues = {
  rightPaddlePosition: 0,
};

// Update code
// Call API every 0.1 second
// Use GSAP to smoothely animate
const tickApi = async () => {
  const response = await fetch("/", { method: "POST" });
  const sensorValues = await response.json();
  gsap.to(animatedValues, { rightPaddlePosition: sensorValues.phi });

  // // Update Raw Debug Values
  // document.getElementById("RawText").innerText = printDebug(
  //   sensorValues.ax,
  //   sensorValues.ay,
  //   sensorValues.az,
  //   sensorValues.gx,
  //   sensorValues.gy,
  //   sensorValues.gz,
  //   sensorValues.phi,
  //   sensorValues.theta,
  //   sensorValues.psi
  // );

  setTimeout(tickApi, 100);
};
tickApi();

const intl = new Intl.NumberFormat("en-US", {
  signDisplay: "always",
  minimumFractionDigits: 4,
  minimumFractionDigits: 4,
  minimumIntegerDigits: 3,
});

const printDebug = (ax, ay, az, gx, gy, gz, phi, theta, psi) => {
  ax = intl.format(ax);
  ay = intl.format(ay);
  az = intl.format(az);
  gx = intl.format(gx);
  gy = intl.format(gy);
  gz = intl.format(gz);
  return `${ax}\t${ay}\t${az}\t${gx}\t${gy}\t${gz}\t${phi}\t${theta}\t${psi}`;
};
