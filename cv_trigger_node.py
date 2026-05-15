"""
Project: IoT-Enabled Robotic Arm for Industrial Dyeing
Module: Computer Vision (CV) Vision Node & Trigger
--------------------------------------------------
Credits:
- Core CV Logic: [Teammate's Name] (HSV Masking, Contour Detection)
- System Integration: Pridhav Krishna (WiFi REST API Trigger, ESP32 Handshake)

Description:
This script identifies textile samples based on color-range detection 
and asynchronously triggers the ESP32 actuator node over a local network.
"""

import cv2
import time
import requests  # Added for the trigger

# --- CONFIGURATION ---
ESP32_URL = "http://dye-robot.local/go" # Or use the IP address
cap = cv2.VideoCapture(0)

last_detection_time = 0
detection_interval = 4  # seconds
trigger_cooldown = 10   # Prevent multiple triggers for the same object
last_trigger_time = 0

# Store last detected box
box = None
label = "No Cloth"

def send_trigger():
    try:
        print(">>> [SYSTEM INTEGRATION] Sending trigger to ESP32...")
        response = requests.get(ESP32_URL, timeout=5)
        print(f"ESP32 Response: {response.text}")
    except Exception as e:
        print(f"Trigger failed: Ensure laptop and ESP32 are on the same hotspot.")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    lower = (35, 40, 40)
    upper = (85, 255, 255)
    mask = cv2.inRange(hsv, lower, upper)

    current_time = time.time()

    if current_time - last_detection_time > detection_interval:
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        found = False

        for cnt in contours:
            area = cv2.contourArea(cnt)
            if area > 1500:
                x, y, w, h = cv2.boundingRect(cnt)
                box = (x, y, w, h)
                label = "Cloth Detected"
                found = True
                
                # TRIGGER LOGIC (Contribution by Pridhav Krishna)
                if current_time - last_trigger_time > trigger_cooldown:
                    send_trigger()
                    last_trigger_time = current_time
                break

        if not found:
            box = None
            label = "No Cloth Detected"

        print(label)
        last_detection_time = current_time

    if box is not None:
        x, y, w, h = box
        cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 0, 255), 2)
        cv2.putText(frame, label, (x, y-10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0,0,255), 2)

    cv2.imshow("Frame", frame)
    if cv2.waitKey(1) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()
