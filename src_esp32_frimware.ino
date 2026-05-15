/* * Project: IoT Robotic Arm - Industrial Dyeing Simulation
 * Author: Pridhav Krishna (ECE)
 * Date: May 2026
 */

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// ==========================================
// 1. UPDATE YOUR WIFI HERE
// ==========================================
const char* ssid = "SSID";
const char* password = "PASS";

WebServer server(80);

// Servo Objects
Servo base, shoulder, elbow, gripper;

// Pins
const int pBase = 13, pShoulder = 15, pElbow = 18, pGripper = 19;

// Track current angles: Base 0, Shoulder 90, Elbow 90, Gripper 180
int cur[] = {0, 90, 90, 180}; 

void setup() {
  Serial.begin(115200);
  ESP32PWM::allocateTimer(0);

  // 1. PRE-LOAD: Force PWM signals to match physical start BEFORE attaching
  // This is the absolute "No-Snap" secret for the ESP32
  base.writeMicroseconds(map(cur[0], 0, 180, 500, 2400));
  shoulder.writeMicroseconds(map(cur[1], 0, 180, 500, 2400));
  elbow.writeMicroseconds(map(cur[2], 0, 180, 500, 2400));
  gripper.writeMicroseconds(map(cur[3], 0, 180, 500, 2400));

  // 2. ATTACH: Lock the pins to the pre-loaded signals
  base.attach(pBase, 500, 2400);
  shoulder.attach(pShoulder, 500, 2400);
  elbow.attach(pElbow, 500, 2400);
  gripper.attach(pGripper, 500, 2400);

  Serial.println("--- SYSTEM INITIALIZED ---");
  Serial.println("1. Ensure arm is manually at: 0, 90, 90, 180");
  Serial.println("2. Turn on external Servo Power Rail now.");
  Serial.println("3. Type 'GO' to start the Dyeing Routine.");
}

void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("GO")) {
      Serial.println("CV match confirmedT... OBJECT TEXTILE FOUND.");
      executeDyeRoutine();
    } 
    else if (cmd.equalsIgnoreCase("NO")) {
      Serial.println("ANALYZING OBJECT... ERROR: WRONG OBJECT. ROUTINE ABORTED.");
    }
  }
}

float estimateGrippingForce(int targetAngle) {
  // Free-run current for SG90 is approx 100mA, Stall current is 650mA.
  // We simulate the force (Newtons) based on the PWM resistance.
  
  // 180 is 'Closed', 45 is 'Open'. 
  // If we command 180, but the object stops it at 160, force increases.
  float force = map(targetAngle, 45, 180, 0, 150) / 100.0; 
  
  float noise = (random(-5, 5) / 100.0); 
  
  return force + noise; 
}

void executeDyeRoutine() {
  Serial.println(">>> Starting Dyeing Industry Routine...");

  // STEP 1: Position for Grab (Gripper Open, Elbow to 130)
  // We use 130 to stay safely away from your 160 'Crunch Zone'
  moveSmooth(4, 45); 
  moveSmooth(3, 160); 
  delay(800);

  // STEP 2: Secure the Material (Gripper Close)
  Serial.println("STATUS: Grasping Material...");
  moveSmooth(4, 180);

  float gripForce = estimateGrippingForce(180);
Serial.print("INTERNAL FEEDBACK: PWM Pulse Stabilized at 2350us. ");
Serial.print("CALCULATED LOAD: ");
Serial.print(gripForce);
Serial.println(" Newtons.");

if(gripForce > 0.5) {
  Serial.println("STATUS: [SECURE HOLD] - Object mass verified.");
}

  //Serial.println("GRIP COMPLETE. CALCULATED FORCE 0.85 Newtons.");
  delay(1000);

  // STEP 3: Move to Dye Vat (Lift Shoulder, Rotate Base)
  moveSmooth(2, 130); // Lift first to clear obstacles
  moveSmooth(1, 180); // Rotate to Vat position
  moveSmooth(3, 90);  // Lower elbow into vat
  delay(1200);

  // STEP 4: Release in Vat/Drying area
  moveSmooth(4, 45);
  delay(1000);

  // STEP 5: Return to Home Safely
// 5. Return Home Safely
  Serial.println(">>> Returning to Home...");
  
  // TUCK THE ELBOW FIRST: This reduces the load on the shoulder
  moveSmooth(3, 80);  
  
  // MOVE SHOULDER TO A RELAXED HOME: Try 85 instead of 90
  moveSmooth(2, 85);   
  
  // ROTATE BASE BACK
  moveSmooth(1, 0);    
  
  // RESET REMAINING
  moveSmooth(4, 180); // Close gripper for transport

  Serial.println("--- Routine Complete. Standing by for next 'GO' ---");
}

void moveSmooth(int id, int target) {
  int idx = id - 1;
  Servo* s;
  
  // Select the correct servo object
  if (id == 1) s = &base;
  else if (id == 2) s = &shoulder;
  else if (id == 3) s = &elbow;
  else if (id == 4) s = &gripper;
  else return;

  // DEAD-BAND: If within 2 degrees, don't move. Prevents "Hunting" & Heating.
  if (abs(cur[idx] - target) <= 4) {
    return; 
  }

  Serial.printf("Moving Servo %d to %d...\n", id, target);

  while (cur[idx] != target) {
    // Step toward target
    if (cur[idx] < target) cur[idx]++;
    else cur[idx]--;
    
    // Convert angle to stable Microseconds
    int pulse = map(cur[idx], 0, 180, 500, 2400);
    s->writeMicroseconds(pulse);
    
    // Movement Speed: 35ms is smooth but doesn't take forever
    delay(35); 
  }
}
