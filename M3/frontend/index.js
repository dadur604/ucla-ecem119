import * as THREE from "three";

import { GLTFLoader } from "https://unpkg.com/three@0.145.0/examples/jsm/loaders/GLTFLoader.js";
import { OrbitControls } from "https://unpkg.com/three@0.145.0/examples/jsm/controls/OrbitControls";

const loader = new GLTFLoader();

const scene = new THREE.Scene();

const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.shadowMap.enabled = true;
document.body.appendChild(renderer.domElement);

const camera = new THREE.PerspectiveCamera(
  75,
  window.innerWidth / window.innerHeight,
  0.1,
  5000
);

camera.position.set(0, 500, 1000);

const controls = new OrbitControls(camera, renderer.domElement);
controls.update();

// Lights

const ambientLight = new THREE.AmbientLight(0xffffff); // soft white light
scene.add(ambientLight);
scene.add(camera);

const pointLight1 = new THREE.PointLight(0xffffff, 0.6, 2000);
pointLight1.castShadow = true;
pointLight1.shadow.camera.far = 5000;
pointLight1.position.set(0, 200, 500);
scene.add(pointLight1);

const pointLight2 = new THREE.PointLight(0xff0000, 0.6, 2000);
pointLight2.castShadow = true;
pointLight2.shadow.camera.far = 5000;
pointLight2.position.set(500, 500, 500);
scene.add(pointLight2);

const pointLight3 = new THREE.PointLight(0x0000ff, 0.6, 2000);
pointLight3.castShadow = true;
pointLight3.shadow.camera.far = 5000;
pointLight3.position.set(-500, 500, 500);
scene.add(pointLight3);

// Walls

const geometry = new THREE.PlaneGeometry(10000, 10000);
const material = new THREE.MeshStandardMaterial({
  color: 0xaaaaaa,
  side: THREE.DoubleSide,
});

const backWall = new THREE.Mesh(geometry, material);
const floor = new THREE.Mesh(geometry, material);

backWall.receiveShadow = true;
floor.receiveShadow = true;

backWall.position.set(0, 0, -500);
floor.position.set(0, -1000, 0);
floor.rotation.set(1.57, 0, 0);

scene.add(backWall);
scene.add(floor);

let animatedSensorValues = {
  ax: 0,
  ay: 0,
  az: 0,
  gx: 0,
  gy: 0,
  gz: 0,
};

let meshScene;
const arduinoGroup = new THREE.Group();
scene.add(arduinoGroup);

loader.load(
  "/models/arduino_nano_v3/scene.gltf",
  function (gltf) {
    console.log("Loaded gltf, ", gltf);
    meshScene = gltf.scene;
    gltf.scene.scale.set(50, 50, 50);
    gltf.scene.traverse((m) => {
      if (m.isMesh) m.castShadow = true;
    });
    arduinoGroup.add(gltf.scene);
    arduinoGroup.rotation.set(0, 1.57, 0);
  },
  undefined,
  function (error) {
    console.error(error);
  }
);

const xyGroup = new THREE.Group();
const xyPositionGroup = new THREE.Group();
xyGroup.add(xyPositionGroup);
scene.add(xyGroup);
const plane = new THREE.Mesh(
  new THREE.PlaneGeometry(100, 100),
  new THREE.MeshBasicMaterial({ color: 0x555555, side: THREE.DoubleSide })
);
plane.rotation.set(1.57, 0, 0);
const ball = new THREE.Mesh(
  new THREE.SphereGeometry(20),
  new THREE.MeshStandardMaterial({ color: 0xff2222 })
);
const lineGeometry = new THREE.BufferGeometry().setFromPoints([
  new THREE.Vector3(0, 0, 0),
  new THREE.Vector3(0, 50, 0),
]);
const line = new THREE.Line(
  lineGeometry,
  new THREE.LineBasicMaterial({ color: 0x000000 })
);
plane.position.set(0, 0, 0);
ball.position.set(0, 50, 0);
line.position.set(0, 0, 0);
xyGroup.add(plane);
xyPositionGroup.add(ball);
xyPositionGroup.add(line);
xyGroup.position.set(-350, 0, 650);

const animate = () => {
  requestAnimationFrame(animate);
  renderer.render(scene, camera);

  if (meshScene) {
    meshScene.rotation.set(
      -1 * animatedSensorValues.phi,
      -1 * animatedSensorValues.psi,
      -1 * animatedSensorValues.theta
    );
    // meshScene.rotation.set(
    //   (-1 * (animatedSensorValues.gx * Math.PI)) / 180,
    //   (animatedSensorValues.gz * Math.PI) / 180,
    //   (animatedSensorValues.gy * Math.PI) / 180,
    //   "XYZ"
    // );
    // meshScene.position.set(
    //   animatedSensorValues.ax * 1000,
    //   animatedSensorValues.az * 1000 - 1000,
    //   animatedSensorValues.ay * -1000
    // );

    // Update XY Map

    xyPositionGroup.position.set(
      animatedSensorValues.ay * 100,
      0,
      animatedSensorValues.ax * 100
    );
    const height = animatedSensorValues.az * 100;
    lineGeometry.setFromPoints([
      new THREE.Vector3(0, 0, 0),
      new THREE.Vector3(0, height, 0),
    ]);
    ball.position.set(0, height, 0);
  }
};
animate();

// Update code
// Call API every 0.1 second
// Use GSAP to smoothely animate
const tickApi = async () => {
  const response = await fetch("/", { method: "POST" });
  const sensorValues = await response.json();
  gsap.to(animatedSensorValues, { ...sensorValues });

  // Update Raw Debug Values
  document.getElementById("RawText").innerText = printDebug(
    sensorValues.ax,
    sensorValues.ay,
    sensorValues.az,
    sensorValues.gx,
    sensorValues.gy,
    sensorValues.gz,
    sensorValues.phi,
    sensorValues.theta,
    sensorValues.psi
  );

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
