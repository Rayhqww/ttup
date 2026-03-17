#!/usr/bin/env python3
import cv2
import numpy as np
import serial
import time
from picamera2 import Picamera2

# Settings (change for specific line and behaviour!)
THRESHOLD = 200       # brightness of white line (0-255)
MIN_AREA = 500        # minimal size of image
CENTER_X = 320        # middle of captured image (ширина 640/2)
TURN_THRESHOLD = 50   # turn radius

# Connection to ESP32 (check port!)
try:
    ser = serial.Serial('/dev/ttyAMA0', 115200, timeout=1)
    # If doesnt work, try other serial port /dev/ttyUSB0 or /dev/serial0
except:
    ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)

time.sleep(2)  # Wait for ESP32

# Camera initialization
picam2 = Picamera2()
config = picam2.create_preview_configuration(main={"size": (640, 480), "format": "RGB888"})
picam2.configure(config)
picam2.start()

print("Robot launched! Ctrl+C to interrupt.")

def send_command(cmd):
    ser.write((cmd + '\n').encode())
    print(f"Command: {cmd}")

try:
    while True:
        # capture image
        frame = picam2.capture_array()
        
        # Look for lower part of picture (road)
        height, width = frame.shape[:2]
        roi = frame[int(height*0.6):, :]  # Нижние 40%
        
        # Tranform grayscale and looking for white
        gray = cv2.cvtColor(roi, cv2.COLOR_RGB2GRAY)
        _, mask = cv2.threshold(gray, THRESHOLD, 255, cv2.THRESH_BINARY)
        
        # finding contours
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        if contours:
            # biggest contour = line
            largest = max(contours, key=cv2.contourArea)
            area = cv2.contourArea(largest)
            
            if area > MIN_AREA:
                # line center
                M = cv2.moments(largest)
                if M["m00"] != 0:
                    cx = int(M["m10"] / M["m00"])
                    
                    # Servo follows line
                    # Mapping 0-640 in 45-135 degrees
                    servo_angle = int(45 + (cx / width) * 90)
                    send_command(f"A:{servo_angle}")
                    
                    # Moving controlls
                    error = cx - CENTER_X
                    
                    if abs(error) < TURN_THRESHOLD:
                        send_command("F")  # forward
                    elif error < 0:
                        send_command("L")  # left
                        time.sleep(0.1)
                        send_command("F")
                    else:
                        send_command("R")  # right
                        time.sleep(0.1)
                        send_command("F")
                    
                    # Visualization (use monitor to see)
                    cv2.circle(roi, (cx, roi.shape[0]//2), 10, (0,255,0), -1)
            else:
                send_command("S")  # line too small
        else:
            send_command("S")  # line not found
            print("line lost!")
        
        # obstacle check (read from Serial)
        if ser.in_waiting:
            line = ser.readline().decode().strip()
            if line.startswith("D:"):
                dist = int(line.split(":")[1])
                if dist < 20:  # 20 sm
                    print(f"Obstacle! {dist}cm")
                    send_command("S")
                    time.sleep(0.5)
        
        time.sleep(0.05)  # 20 FPS

except KeyboardInterrupt:
    print("Stop...")
    send_command("S")
    ser.close()
    cv2.destroyAllWindows()
