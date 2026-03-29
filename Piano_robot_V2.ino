//////////////////////
//   I2C PROTOCOL  ///
//////////////////////

#include <Wire.h>
#define AS5600_ADDRESS 0x36
#define AS5600_RAW_ANGLE 0x0C

///////////////////////
// PIN DEFINITIONS   //
///////////////////////

#define IN1 25 // 25 or 32 PWM pins 
#define IN2 26 // 26 or 35 PWM pins

/////////////////////
// POSITION VARS   //
/////////////////////

float totalpos = 0;
float posx = 0;
float prev_pos = 0;
unsigned long lastUpdate = 0;
const unsigned long interval = 1000; // 1ms

const unsigned long R = 6.4; //mm

//////////////////////
//    PID VARS      //
//////////////////////

float e = 0.0;
float eprev = 0.0;
float eintegral = 0.0;
float dedt = 0.0;
float u = 0;
float kp = 1.0; // Ajusta estos valores
float kd = 0.04;
float ki = 0.005;
float deltaT = 0.0;
float target = 0.0;

#include <stdio.h>
#include <stdbool.h>

// Solenoid Pin Definitions
#define S1 12
#define ON 1
#define OFF 0
#define PRESS 255

// Notes (position placeholders)
#define A4 10
#define B4 20
#define C4 30
#define D4 40
#define E4 50
#define F4 60
#define G4 70

#define BPM 100

enum MOTOR_STATE 
{
  READY,
  MOVING,
  PLAYING,
  RESET
};

enum MOTOR_STATE motorState = READY;


/*
const double testArray[][3] =
{
  // {No. of Beats, Note, Strength}
  {1, E4, 1},
  {1, E4, 1},
  {1, F4, 1},
  {1, G4, 1},
  {1, G4, 1},
  {1, F4, 1},
  {1, E4, 1},
  {1, D4, 1},
  {1, C4, 1},
  {1, C4, 1},
  {1, D4, 1},
  {1, E4, 1},
  {1, E4, 1},
  {1, D4, 1},
  {2, D4, 1},
  {2, A4, 1}
};

*/


const double testArray[][3] =
{
  {1, E4, 1}
  //{1, G4, 1}
};


const int NUM_EVENTS = sizeof(testArray) / sizeof(testArray[0]);

double beats, position, duty;

int n = 0;

unsigned long noteStartTime = 0;
unsigned long noteDuration = 0;

void setup() 
{
  Serial.begin(115200);
  Wire.begin();
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  ledcAttach(IN1, 5000, 8);
  ledcAttach(IN2, 5000, 8);
  prev_pos = readAngleDegrees(); // Inicializar posición
  pinMode(S1, OUTPUT);
  Serial.println("Beginning Program");
}

void loop() 
{
  unsigned long now = micros();

  if (now - lastUpdate >= interval) 
  {
    
    deltaT = (now - lastUpdate) / 1.0e6; // Tiempo real en segundos
    lastUpdate = now;

    // 1. LEER POSICIÓN Y ACUMULAR
    float current_pos = readAngleDegrees();
    float deltapos = current_pos - prev_pos;

    if (deltapos > 180) deltapos -= 360;
    if (deltapos < -180) deltapos += 360;

    totalpos += deltapos;
    prev_pos = current_pos;

    posx = (totalpos * R * 3.14159)/180;

    // PID
    //target = 0;
    u = pid(target, posx, deltaT);

    float pwr = fabs(u);

    if (pwr > 255) pwr = 255;

    int dir = (u >= 0) ? 1 : -1; // 1 adelante, -1 atrás
    setMotor(dir, (int)pwr);

    // Graficar
    //Serial.print(target); Serial.print(",");
    //Serial.print(totalpos); Serial.print(",");
    //Serial.print(posx);// Serial.print(",");
    //Serial.println(e);
    //Serial.print(e); Serial.print(","); Serial.print(dedt);Serial.print(","); Serial.println(eintegral);

    //For matlab
    //Serial.println(posx);
  }

  //if (n >= NUM_EVENTS) return; // stop when done


  switch (motorState)
  {
    case READY:
      // Load next note

      if (n >= NUM_EVENTS)
      {
        //Serial.println("done");
        analogWrite(S1, OFF);
        break;
      }

      //Serial.println("ready");
      //analogWrite(S1, OFF);
      beats    = testArray[n][0];
      position = testArray[n][1];
      duty     = testArray[n][2];
      // Convert beats to time (ms)
      noteDuration = (60000 / BPM) * beats;

      /*
      if(n < NUM_EVENTS)
      {
      motorState = MOVING;
      }
      else
      motorState = READY;
      */

      motorState = MOVING;
      break;

    case MOVING:
      // For now: instant move (no PID yet)
      //Serial.print("Moving to position: ");
      //Serial.println(position);
      //Serial.println("move"); 
      target = position * 2; //in mm
      analogWrite(S1, OFF);

      // Reset PID error terms for new target
      eprev = 0;
      eintegral = 0;

      if(fabs(target - posx) < 1.5) motorState = PLAYING;
      //else
      //motorState = MOVING;
      break;

    //case PLAYING:
      //Serial.println("Playing note");

      //analogWrite(S1, ON);  // scale duty

      //noteStartTime = millis();
      //motorState = RESET;
      //break;

    case PLAYING:
      //Serial.println("playing");
      analogWrite(S1, PRESS);

      //if (motorState != RESET) {
      //noteStartTime = millis();
      //}

      noteStartTime = millis();

      motorState = RESET;
      break;
    
    case RESET:
      // Wait until note duration is done
      //Serial.println("reset");
      if (millis() - noteStartTime >= noteDuration)
      {
        analogWrite(S1, 0); // turn off solenoid
        //Serial.println("Note done");

        n++; 
        motorState = READY;
      }
      //motorState = READY;
      break;
  }
}

void setMotor(int dir, int pwmVal) 
{
  if (dir == 1) 
  {
    ledcWrite(IN1, pwmVal);
    ledcWrite(IN2, 0);
  } 
  else if (dir == -1) 
  {
    ledcWrite(IN1, 0);
    ledcWrite(IN2, pwmVal);
  } 
  else 
  {
    ledcWrite(IN1, 0);
    ledcWrite(IN2, 0);
  }
}

float readAngleDegrees() 
{
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(AS5600_RAW_ANGLE);
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDRESS, 2);
  if (Wire.available() >= 2) {
    uint16_t raw = (Wire.read() << 8) | Wire.read();
    return (raw & 0x0FFF) * 360.0 / 4096.0;
  }
  return prev_pos; // Retorna la última si falla la lectura
}

float pid(float setpoint, float measurement, float dt)
{
    e = setpoint - measurement;

    if (fabs(e) < 0.5) e = 0;

    // --- Derivative ---
    static float dedtF = 0;
    float alpha = 0.8;
    dedt = (e - eprev) / dt;
  
    dedtF = alpha * dedtF + (1 - alpha) * dedt;

    // --- Integral ---
    eintegral += e * dt;

    // Clamp integral (anti-windup)
    if (eintegral > 500) eintegral = 500;
    if (eintegral < -500) eintegral = -500;

    // --- PID ---
    u = (kp * e) + (kd * dedtF) + (ki * eintegral);

    // --- Saturation ---
    if (u > 255) u = 255;
    if (u < -255) u = -255;

    // --- Deadband ---
    if (fabs(u) < 5) u = 0;

    // --- Save state ---
    eprev = e;

    return u;
}