#include <Wire.h>

// ================= I2C (AS5600) =================
#define AS5600_ADDRESS 0x36
#define AS5600_RAW_ANGLE 0x0C

// ================= MOTOR =================
#define IN1 25
#define IN2 26

// ================= SOLENOIDS =================
#define NUM_FINGERS 4
#define S1 19
#define S2 18
#define S3 16 
#define S4 17 //black key 

//int solenoidPins[NUM_FINGERS] = {S2, S1, S3, S4};

int solenoidPins[NUM_FINGERS] = {S2, S1, S3, S4}; //0, 1 ,2, 3

/*
S1 - middle 
S2 - right
S3- black
S4 -left
*/

//int solenoidPins[NUM_FINGERS] = {S4};

// ================= POSITION =================
float totalpos = 0;
float posx = 0;
float prev_pos = 0;

const float R = 6.4;

unsigned long lastUpdate = 0;
float deltaT = 0;

// ================= PID =================
float kp = 2.0;
float kd = 0.04;
float ki = 0.005;

float e = 0, eprev = 0, eintegral = 0, dedt = 0;
float u = 0;
float target = 0;

// ================= FINGERS =================
float fingerOffsets[NUM_FINGERS] = {0, 44, 75, 88}; //75
//float fingerOffsets[NUM_FINGERS] = {0};

int activeFinger = 0;

// ================= NOTES =================
// Notes (position placeholders

/*const int N_A4 = 108;
const int N_B4 = 92;
const int N_C4 = 67;
const int N_D4 = 43;
const int N_E4 = 17;
const int N_F4 = 0;
const int N_G4 = 68.2;
const int S_D4 = 104;
*/
// Top (rightmost keys)
// These keys are not present (keyboard ends at F4)
// const int N_B4 = ...;
// const int N_A4 = ...;
// const int N_G4 = ...;
// --- Top (rightmost) ---
const int N_F4 = 0;
const int N_E4 = 17;
const int N_D4 = 43;
const int N_C4 = 67;

// --- Continue left ---
const int N_B3 = 92;
const int N_A3 = 108;
const int N_G3 = 130;
const int N_F3 = 154;
const int N_E3 = 171;
const int N_D3 = 197;
const int N_C3 = 221;

// --- Keep going (same pattern) ---
const int N_B2 = 246;
const int N_A2 = 262;
const int N_G2 = 284;
const int N_F2 = 308;
const int N_E2 = 325;
const int N_D2 = 351;
const int N_C2 = 375;

const int N_B1 = 400;
const int N_A1 = 416;
const int N_G1 = 438;
const int N_F1 = 462;
const int N_E1 = 479;
const int N_D1 = 505;
const int N_C1 = 529;

const int N_B0 = 554;
const int N_A0 = 570;
const int N_G0 = 592;
const int N_F0 = 616;
const int N_E0 = 633;
const int N_D0 = 659;
const int N_C0 = 683;

//const int N_G4 = 68.2;

///
/// BLACK KEYS
///

// Missing keys (not on keyboard)
// const int S_A4; // A#4
// const int S_G4; // G#4
// const int S_F4; // F#4
// --- Top (rightmost) ---
const int S_D4 = 104; // D#4 (between D4 and E4)
const int S_C4 = 55;  // C#4 (between C4 and D4)

// --- Continue left ---
const int S_A3 = 100; // A#3
const int S_G3 = 119; // G#3
const int S_F3 = 142; // F#3
const int S_D3 = 184; // D#3
const int S_C3 = 209; // C#3

const int S_A2 = 100 + 154;
const int S_G2 = 119 + 154;
const int S_F2 = 142 + 154;
const int S_D2 = 184 + 154;
const int S_C2 = 209 + 154;

const int S_A1 = 100 + 308;
const int S_G1 = 119 + 308;
const int S_F1 = 142 + 308;
const int S_D1 = 184 + 308;
const int S_C1 = 209 + 308;

const int S_A0 = 100 + 462;
const int S_G0 = 119 + 462;
const int S_F0 = 142 + 462;
const int S_D0 = 184 + 462;
const int S_C0 = 209 + 462;

#define BPM 150

//const double testArray[][3] =
//{
  // {No. of Beats, Note, Strength}
  //{1, N_E4, 1}//, //good
  //{1, N_E4, 1}, //good
  //{1, N_F4, 1}, // good
  //{1, G4, 1},
  //{1, G4, 1},
  //{1, N_F4, 1}, //This code was made by AI (Aiman Ishmam)
  //{1, N_E4, 1},
  //{1, N_D4, 1},
  //{1, N_C4, 1},
  //{1, N_C4, 1},
  //{1, N_D4, 1},
  //{1, N_E4, 1},
  //{1, N_E4, 1},
  //{1, N_D4, 1},
  //{2, N_D4, 1},
  //{2, N_A4, 1} 
//};
/*const double testArray[][4] =
{
  // {No. of Beats, Note, Strength, KeyType}
//  {1, N_A4, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
//  {1, N_A4, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
 // {1, N_A4, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, S_A3, 1, 'B'},   // Bb3 / A#3
  {1, S_C3, 1, 'B'},   // C#3 / Db3
  {1, N_F3, 1, 'W'},
  {1, N_G3, 1, 'W'},

//  {1, N_A4, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
//  {1, N_A4, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
 // {1, N_A4, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, S_A3, 1, 'B'},   // Bb3 / A#3
  {1, S_C3, 1, 'B'},   // C#3 / Db3
  {1, N_F3, 1, 'W'},
  {1, N_G3, 1, 'W'},

  //{1, N_A4, 1, 'W'},    // hold ignored
  {1, N_C3, 1, 'W'},    // hold ignored
//  {1, N_A4, 1, 'W'},
 // {1, N_A4, 1, 'W'},
  {1, N_G3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_G3, 1, 'W'},
  {1, N_F3, 1, 'W'},
 // {1, N_A4, 1, 'W'},    // hold ignored
  {1, S_A3, 1, 'B'},   // Bb3 / A#3
//  {1, N_A4, 1, 'W'},
//  {1, N_A4, 1, 'W'},    // hold ignored
  {1, N_G3, 1, 'W'},    // hold ignored
  {1, N_F3, 1, 'W'},    // hold ignored
  {1, N_G3, 1, 'W'},
  {1, N_D3, 1, 'W'},    // D3 + Bb ignored as chord here
  {1, S_A3, 1, 'B'}    // Bb3 / A#3
}; */
/*const double testArray[][4] =
{
  // {No. of Beats, Note, Strength}
  {1, N_A4, 1, 'W'},//, //good
  {1, N_B4, 1, 'W'}, //good
  {1, N_C4, 1, 'W'}, // good
  {1, N_D4, 1, 'W'},
  {1, N_E4, 1, 'W'},
  {1, N_F4, 1, 'W'},
  {1, S_D4, 1, 'B'}
  //{1, N_F4, 1}, //This code was made by AI (Aiman Ishmam)
  //{1, N_E4, 1},
  //{1, N_D4, 1},
  //{1, N_C4, 1},
  //{1, N_C4, 1},
  //{1, N_D4, 1},
  //{1, N_E4, 1},
  //{1, N_E4, 1},
  //{1, N_D4, 1},
  //{2, N_D4, 1},
  //{2, N_A4, 1} 
}; */

const double testArray[][4] =
{
  // {No. of Beats, Note, Strength, KeyType}
  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, S_A3, 1, 'B'},   // Bb3 / A#3
  {1, S_C3, 1, 'B'},   // C#3 / Db3
  {1, N_F3, 1, 'W'},
  {1, N_G3, 1, 'W'},

  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, S_A3, 1, 'B'},
  {1, S_C3, 1, 'B'},
  {1, N_F3, 1, 'W'},
  {1, N_G3, 1, 'W'},

  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_G3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_G3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, S_A3, 1, 'B'},
  {1, N_A3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_G3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_G3, 1, 'W'},
  {1, N_D3, 1, 'W'},
  {1, S_A3, 1, 'B'},

  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, S_A3, 1, 'B'},   // Bb3 / A#3
  {1, S_C3, 1, 'B'},   // C#3 / Db3
  {1, N_F3, 1, 'W'},
  {1, N_G3, 1, 'W'},

  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, S_A3, 1, 'B'},
  {1, S_C3, 1, 'B'},
  {1, N_F3, 1, 'W'},
  {1, N_G3, 1, 'W'},

  {1, N_A3, 1, 'W'},
  {1, N_C3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_G3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_G3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, S_A3, 1, 'B'},
  {1, N_A3, 1, 'W'},
  {1, N_A3, 1, 'W'},
  {1, N_G3, 1, 'W'},
  {1, N_F3, 1, 'W'},
  {1, N_G3, 1, 'W'},
  {1, N_D3, 1, 'W'},
  {1, S_A3, 1, 'B'}
};

double beats, position, duty;
char keyType;
const int NUM_EVENTS = sizeof(testArray) / sizeof(testArray[0]);
int n = 0;

unsigned long noteDuration = 0;
unsigned long stateStartTime = 0;
unsigned long moveStartTime = 0;

// ================= FSM =================
enum MOTOR_STATE
{
  READY,
  MOVE,
  ENERGIZE,
  HOLD,
  RELEASE
};

MOTOR_STATE motorState = READY;

// ================= FUNCTION DECL =================
float readAngleDegrees();
float pid(float setpoint, float measurement, float dt);
void setMotor(int dir, int pwmVal);
//int getBestFinger(float keyPos);
int getBestWhiteFinger(float keyPos);
void activateFinger(int index);
void releaseAll();

// ================= SETUP =================
void setup()
{
  Serial.begin(115200);
  Wire.begin();

  prev_pos = readAngleDegrees();

  ledcAttach(IN1, 5000, 8);
  ledcAttach(IN2, 5000, 8);

  for (int i = 0; i < NUM_FINGERS; i++)
    pinMode(solenoidPins[i], OUTPUT);

  releaseAll();

  Serial.println("System Ready");
}

// ================= LOOP =================
void loop()
{
  // ===== Update Position =====
  float current_pos = readAngleDegrees();
  float deltapos = current_pos - prev_pos;

  if (deltapos > 180) deltapos -= 360;
  if (deltapos < -180) deltapos += 360;

  totalpos += deltapos;
  prev_pos = current_pos;

  posx = (totalpos * R * PI) / 180.0;

  // ===== PID (always running) =====
  unsigned long now = micros();
  if (now - lastUpdate >= 1000)
  {
    deltaT = (now - lastUpdate) / 1e6;
    lastUpdate = now;

    u = pid(target, posx, deltaT);

    int dir = (u > 0) ? 1 : (u < 0 ? -1 : 0);
    setMotor(dir, abs(u));
  }

  // ===== FSM =====
  switch (motorState)
{
  case READY: // Load next note
  {
    if (n >= NUM_EVENTS)
    {
      releaseAll();
      setMotor(0, 0);
      return;   // stop loop execution
    }

    beats    = testArray[n][0];
    position = testArray[n][1];
    duty     = testArray[n][2];
    keyType  = (char)testArray[n][3];

    // Choose best finger for this note
    /*if (keyType == 'B'){
      activeFinger = 2;
    }
    else{
      activeFinger = getBestFinger(position);
    }*/
    if (keyType == 'B')
{
  activeFinger = 2;   // S3 only
  Serial.println("Black key -> S3");
}
else
{
  activeFinger = getBestWhiteFinger(position);
}

    // Set carriage target so chosen finger aligns to key
    target = position - fingerOffsets[activeFinger];

    // Convert beats to time (ms)
    noteDuration = (60000 / BPM) * beats;

    // Reset PID memory
    eprev = 0;
    eintegral = 0;

    moveStartTime = millis();
    motorState = MOVE;
    break;
  }

  case MOVE:
  {
    //Serial.println("move");
    releaseAll();

    if ((fabs(target - posx) < 2.5 && fabs(u) < 5.0) ||
        (millis() - moveStartTime > 500))
    {
      motorState = ENERGIZE;
      stateStartTime = millis();
    }

    break;
  }

  case ENERGIZE:
  {
    activateFinger(activeFinger);

    if (millis() - stateStartTime >= 50) // strike time
    {
      motorState = HOLD;
      stateStartTime = millis();
    }

    break;
  }

  case HOLD:
  {
    activateFinger(activeFinger);

    //if (millis() - stateStartTime >= 50) // hold duration
    if (millis() - stateStartTime >= noteDuration)
    {
      motorState = RELEASE;
      stateStartTime = millis();
    }

    break;
  }

  case RELEASE:
  {
    releaseAll();

    if (millis() - stateStartTime >= 400) // relax time
    {
      n++;
      motorState = READY;
    }

    break;
  }
}
}

// ================= MOTOR =================
void setMotor(int dir, int pwmVal)
{
  if (dir == 1)
  {
    ledcWrite(IN2, pwmVal);
    ledcWrite(IN1, 0);
  }
  else if (dir == -1)
  {
    ledcWrite(IN2, 0);
    ledcWrite(IN1, pwmVal);
  }
  else
  {
    ledcWrite(IN2, 0);
    ledcWrite(IN1, 0);
  }
}

// ================= PID =================
float pid(float setpoint, float measurement, float dt)
{
  e = setpoint - measurement;

  if (fabs(e) < 0.5) e = 0;

  static float dedtF = 0;
  float alpha = 0.8;

  dedt = (e - eprev) / dt;
  dedtF = alpha * dedtF + (1 - alpha) * dedt;

  eintegral += e * dt;
  eintegral = constrain(eintegral, -300, 300);

  float output = kp * e + kd * dedtF + ki * eintegral;
  output = constrain(output, -255, 255);

  if (fabs(output) < 5) output = 0;

  eprev = e;
  return output;
}

// ================= ENCODER =================
float readAngleDegrees()
{
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(AS5600_RAW_ANGLE);
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDRESS, 2);

  if (Wire.available() >= 2)
  {
    uint16_t raw = (Wire.read() << 8) | Wire.read();
    return (raw & 0x0FFF) * 360.0 / 4096.0;
  }

  return prev_pos;
}

// ================= FINGER SELECTION =================
/*int getBestFinger(float keyPos)
{
  int bestFinger = 0;
  float minMove = fabs((keyPos - fingerOffsets[0]) - posx);

  for (int i = 1; i < NUM_FINGERS; i++)
  {
    float requiredTarget = keyPos - fingerOffsets[i];
    float moveDist = fabs(requiredTarget - posx);

    if (moveDist < minMove)
    {
      minMove = moveDist;
      bestFinger = i;
    }
  }
  Serial.println("start");
  Serial.println(bestFinger);
  return bestFinger;
}*/
int getBestWhiteFinger(float keyPos)
{
  int whiteFingerIndices[] = {0, 1, 3};  // exclude 2 = S3
  int bestFinger = whiteFingerIndices[0];

  float minMove = fabs((keyPos - fingerOffsets[bestFinger]) - posx);

  for (int j = 1; j < 3; j++)
  {
    int i = whiteFingerIndices[j];
    float requiredTarget = keyPos - fingerOffsets[i];
    float moveDist = fabs(requiredTarget - posx);

    if (moveDist < minMove)
    {
      minMove = moveDist;
      bestFinger = i;
    }
  }

  Serial.print("White key finger = ");
  Serial.println(bestFinger);
  return bestFinger;
}

// ================= SOLENOIDS =================
void activateFinger(int index)
{
  for (int i = 0; i < NUM_FINGERS; i++)
    digitalWrite(solenoidPins[i], LOW);

  digitalWrite(solenoidPins[index], HIGH);
}

void releaseAll()
{
  for (int i = 0; i < NUM_FINGERS; i++)
    digitalWrite(solenoidPins[i], LOW);
}