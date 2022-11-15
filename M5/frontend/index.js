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

let score = [0, 0];

// create an engine
var engine = Engine.create({
  gravity: {
    x: 0,
    y: 0,
  },
});
const world = engine.world;

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

// create player paddles
var leftPaddle = Bodies.rectangle(
  paddleWidth,
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

// create left sensor
var leftSensor = Bodies.rectangle(0, height / 2, 10, height, {
  isStatic: true,
  isSensor: true,
  label: "leftsensor",
});

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

// create right sensor
var rightSensor = Bodies.rectangle(width, height / 2, 10, height, {
  isStatic: true,
  isSensor: true,
  label: "rightsensor",
});

Events.on(engine, "collisionStart", function (event) {
  var pairs = event.pairs;

  // change object colours to show those starting a collision
  for (var i = 0; i < pairs.length; i++) {
    var pair = pairs[i];
    if (pair.bodyA.label === "rightsensor" && pair.bodyB.label === "ball") {
      Body.setPosition(ball, { x: width / 2, y: height / 2 });
      score[0]++;
      document.getElementById("LeftScore").innerText = score[0];
    } else if (
      pair.bodyA.label === "leftsensor" &&
      pair.bodyB.label === "ball"
    ) {
      Body.setPosition(ball, { x: width / 2, y: height / 2 });
      score[1]++;
      document.getElementById("RightScore").innerText = score[1];
    }
  }
});

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
  restitution: 1.005,
  label: "ball",
});

Body.applyForce(ball, { x: 0, y: 0 }, { x: 0.01, y: 0.0 });

// add all of the bodies to the world
Composite.add(engine.world, [
  rightSensor,
  leftSensor,
  ball,
  rightPaddle,
  leftPaddle,
  floor,
  ceiling,
]);

// run the renderer
Render.run(render);

// create runner
var runner = Runner.create();

// run the engine
Runner.run(runner, engine);

Events.on(runner, "beforeUpdate", () => {
  Body.setPosition(leftPaddle, {
    x: paddleWidth,
    y: height / 2 + animatedValues.leftPaddlePosition * (height / 2),
  });
  Body.setPosition(rightPaddle, {
    x: width - paddleWidth,
    y: height / 2 + animatedValues.rightPaddlePosition * (height / 2),
  });
});

let animatedValues = {
  leftPaddlePosition: 0,
  rightPaddlePosition: 0,
};

// Update code
// Call API every 0.1 second
// Use GSAP to smoothely animate
const tickApi = async () => {
  const response = await fetch("/", { method: "POST" });
  const sensorValues = await response.json();
  gsap.to(animatedValues, {
    leftPaddlePosition: sensorValues.p1.phi,
    duration: 50 / 1000.0,
  });
  gsap.to(animatedValues, {
    rightPaddlePosition: sensorValues.p2.phi,
    duration: 50 / 1000.0,
  });
  // animatedValues.leftPaddlePosition = sensorValues.p1.phi;
  // animatedValues.rightPaddlePosition = sensorValues.p2.phi;

  setTimeout(tickApi, 20);
};
tickApi();
