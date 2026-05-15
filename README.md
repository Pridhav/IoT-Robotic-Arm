# IoT-Enabled Robotic Arm for Industrial Dyeing Simulation
A 4-DOF robotic arm utilizing an ESP32-based Distributed Control System (DCS) for industrial dyeing simulation. Features real-time Computer Vision integration, WiFi-based asynchronous triggering, and software-defined kinematic force estimation


## Project Overview
This project was developed as a multidisciplinary engineering prototype for an industrial dyeing elective. It demonstrates the integration of a **Computer Vision (CV) Vision Node** with an **IoT-enabled Actuator Node** (ESP32) to automate the handling of textile samples.

The system identifies specific material types via a remote camera feed and executes a smooth, high-displacement kinematic routine to transport samples into a simulated dyeing vat.

## Technical Engineering Features

### 1. Software-Defined Force Estimation (Virtual Sensing)
To maintain a lightweight end-effector and reduce mechanical failure points, I implemented **Current Signature Inference**. By monitoring the PWM pulse stabilization against target angles, the system estimates the gripping force exerted on the textile, fulfilling grasping-status requirements without external FSR sensors.

### 2. Distributed Control System (DCS) Architecture
The system utilizes a decoupled architecture:
* **Vision Node:** A Python environment running OpenCV/ML models to identify samples.
* **Actuator Node:** An ESP32 running a Web Server to receive asynchronous triggers via a local WiFi network.
* **Handshake:** Communication is handled via mDNS (`dye-robot.local`) to ensure seamless connectivity in dynamic network environments.

### 3. Kinematic Smoothing & Torque Management
To prevent servo gear stripping (a common failure in SG90-based systems), I developed a custom `moveSmooth` algorithm. This manages:
* **Dead-band Filtering:** Eliminates servo "hunting" and jitter.
* **Sequential Tucking:** Optimizes the elbow-shoulder movement sequence to minimize the moment of inertia and protect the shoulder joint.

## 📐 Hardware Specifications
* **Controller:** ESP32 (WROOM-32)
* **Actuators:** 4x SG90 Servos (Base, Shoulder, Elbow, Gripper)
* **Displacement:** >600mm lateral reach (180° rotation)
* **Power:** External 5V DC Rail with Common Ground architecture

## Logic Flow
1. **Detection:** CV Node identifies the textile sample.
2. **Trigger:** Python script sends an HTTP GET request to the ESP32.
3. **Execution:** ESP32 initiates the `executeDyeRoutine()` with real-time feedback to the Serial Monitor.
4. **Safety:** System monitors internal kinematic limits before returning to the "Home" state.

## Challenges & Solutions
* **Mechanical Stress:** Optimized the elbow trajectory to 160° to avoid "crunch zones" that were previously stripping gears.
* **Latency:** Implemented a non-blocking `handleClient()` loop to ensure the robot responds instantly to WiFi triggers while maintaining manual Serial override.

---
**Developed by:** Team RoboCop - Pridhav Krishna
**Focus:** Electronics & Communication Engineering (ECE) - Cyber Physical Systems
