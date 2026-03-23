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
float kp = 1.2; // Ajusta estos valores
float kd = 0.04;
float ki = 0.005;
float deltaT = 0.0;

//////////////////
// TRYOUT VARS //
/////////////////

unsigned long stepTimer = 0;
float step = 0;

/////////////
// SETUP  //
////////////

void setup() 
{
  Serial.begin(115200); // 9600 es muy lento para graficar PID
  Wire.begin();
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  ledcAttach(IN1, 5000, 8);
  ledcAttach(IN2, 5000, 8);

  prev_pos = readAngleDegrees(); // Inicializar posición
}

/////////////
//   LOOP  // 
/////////////

void loop() 
{
  unsigned long now = micros();

  // --- CONTROL DE TIEMPO (ESTABILIDAD) ---
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
    float target = 100 + step;
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
    Serial.println(posx);
  }

  if (millis() - stepTimer > 250)//1 second = 1000 ms 
  {
    step += 0;

    if (step > 100)
      step = 0;

    stepTimer = millis();
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
    if (fabs(u) < 1.5) u = 0;

    // --- Save state ---
    eprev = e;

    return u;
}