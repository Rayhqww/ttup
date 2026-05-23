#include <Arduino.h>

// ПИНЫ С ПРАВОЙ СТОРОНЫ ПЛАТЫ 
#define ENA 22    // D22 - PWM левые моторы
#define IN1 4     // D4  - направление левые
#define IN2 16    // D16 - направление левые1ww
#define IN3 17    // D17 - направление правые
#define IN4 5     // D5  - направление правые
#define ENB 23    // D23 - PWM правые моторы

#define PWM_LEFT  0
#define PWM_RIGHT 1

int speed = 150;

void setMotors(int left, int right) {
  // ЛЕВЫЕ колёса
  if (left > 0) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  } else if (left < 0) {
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  } else {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  }
  ledcWrite(PWM_LEFT, abs(left));

  // ПРАВЫЕ колёса
  if (right > 0) {
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  } else if (right < 0) {
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  } else {
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  }
  ledcWrite(PWM_RIGHT, abs(right));
}

void stop() {
  setMotors(0, 0);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  ledcSetup(PWM_LEFT, 5000, 8);
  ledcSetup(PWM_RIGHT, 5000, 8);
  ledcAttachPin(ENA, PWM_LEFT);
  ledcAttachPin(ENB, PWM_RIGHT);

  stop();

  Serial.println("=== 4WD ROBOT READY ===");
  Serial.println("w=forward  s=backward  a=turn_left  d=turn_right  x=stop");
  Serial.println("1=slow(100)  2=medium(175)  3=fast(255)");
  Serial.println("Current speed: " + String(speed));
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch(cmd) {
      case 'd':
      case 'D':
        Serial.println(">> TURN RIGHT | Speed: " + String(speed));
        setMotors(speed, speed); // ЛЕВЫЙ назад, ПРАВЫЙ вперёд = поворот НАПРАВО
        break;
        
      case 'a':
      case 'A':
        Serial.println(">> TURN LEFT | Speed: " + String(speed));
        setMotors(-speed, -speed); // ЛЕВЫЙ вперёд, ПРАВЫЙ назад = поворот НАЛЕВО
        break;
        
      case 'w':
      case 'W':
        Serial.println(">> FORWARD | Speed: " + String(speed));
        setMotors(speed, -speed);   
        break;
        
      case 's':
      case 'S':
        Serial.println(">> BACKWARD | Speed: " + String(speed));
        setMotors(-speed, speed);   
        break;
        
      case 'x':
      case 'X':
      case ' ':
        Serial.println(">> STOP");
        stop();
        break;
        
      case '1':
        speed = 100;
        Serial.println("Speed set: SLOW (100)");
        break;
        
      case '2':
        speed = 175;
        Serial.println("Speed set: MEDIUM (175)");
        break;
        
      case '3':
        speed = 255;
        Serial.println("Speed set: FAST (255)");
        break;
    }
  }
}