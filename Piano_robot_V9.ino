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
#define S4 17 // black key

int solenoidPins[NUM_FINGERS] = {S2, S1, S3, S4}; // 0,1,2,3

/*
0 -> S2 (right)
1 -> S1 (middle)
2 -> S3 (black)
3 -> S4 (left)
*/

// ================= POSITION =================
float totalpos = 0;
float posx = 0;
float prev_pos = 0;

const float R = 6.4;

unsigned long lastUpdate = 0;
float deltaT = 0;

// ================= PID =================
float kp = 3.0;
float kd = 0.04;
float ki = 0.005;

float e = 0, eprev = 0, eintegral = 0, dedt = 0;
float u = 0;
float target = 0;

// ================= FINGERS =================
float fingerOffsets[NUM_FINGERS] = {0, 44, 75, 88};

// current active fingers for this event
int activeFinger1 = -1;
int activeFinger2 = -1;

// ================= NOTES =================
// --- White keys ---
const int N_F4 = 0;
const int N_E4 = 17;
const int N_D4 = 43;
const int N_C4 = 67;

const int N_B3 = 92;
const int N_A3 = 108;
const int N_G3 = 130;
const int N_F3 = 154;
const int N_E3 = 171;
const int N_D3 = 197;
const int N_C3 = 221;

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

// --- Black keys ---
const int S_D4 = 104; // D#4
const int S_C4 = 55;  // C#4

const int S_A3 = 100; // A#3 / Bb3
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

#define BPM 80

// ================= NOTE ARRAY =================
// {beats, note1, type1, note2, type2, strength}
// note2 = -1, type2 = 'N' means no second note
const double testArray[][6] =
{
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, S_A3, 'B', -1,   'N', 1},
  {1, S_C3, 'B', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},

  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, S_A3, 'B', -1,   'N', 1},
  {1, S_C3, 'B', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},

  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, S_A3, 'B', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},
  {1, N_D3, 'W', S_A3, 'B', 1}, // chord

  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, S_A3, 'B', -1,   'N', 1},
  {1, S_C3, 'B', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},

  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, S_A3, 'B', -1,   'N', 1},
  {1, S_C3, 'B', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},

  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_C3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, S_A3, 'B', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_A3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},
  {1, N_F3, 'W', -1,   'N', 1},
  {1, N_G3, 'W', -1,   'N', 1},
  {1, N_D3, 'W', S_A3, 'B', 1}  // chord
};

const int NUM_EVENTS = sizeof(testArray) / sizeof(testArray[0]);

double beats, duty;
double position1, position2;
char keyType1, keyType2;
int n = 0;

unsigned long noteDuration = 0;
unsigned long stateStartTime = 0;
unsigned long moveStartTime = 0;

bool fingersActivated = false;

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
int getBestWhiteFinger(float keyPos, int forbiddenFinger = -1);
int chooseFinger(float keyPos, char keyType, int forbiddenFinger = -1);
void activateFingers(int finger1, int finger2);
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
    case READY:
    {
      if (n >= NUM_EVENTS)
      {
        releaseAll();
        setMotor(0, 0);
        return;
      }

      beats     = testArray[n][0];
      position1 = testArray[n][1];
      keyType1  = (char)testArray[n][2];
      position2 = testArray[n][3];
      keyType2  = (char)testArray[n][4];
      duty      = testArray[n][5];

      activeFinger1 = chooseFinger(position1, keyType1);
      activeFinger2 = -1;

      if (position2 != -1 && keyType2 != 'N')
      {
        activeFinger2 = chooseFinger(position2, keyType2, activeFinger1);
      }

      // Shared target: based on first note/finger
      target = position1 - fingerOffsets[activeFinger1];

      noteDuration = (60000 / BPM) * beats;

      eprev = 0;
      eintegral = 0;
      fingersActivated = false;

      Serial.print("Event ");
      Serial.print(n);
      Serial.print(" | note1=");
      Serial.print(position1);
      Serial.print(" finger1=");
      Serial.print(activeFinger1);

      if (activeFinger2 != -1)
      {
        Serial.print(" | note2=");
        Serial.print(position2);
        Serial.print(" finger2=");
        Serial.print(activeFinger2);
      }

      Serial.print(" | target=");
      Serial.println(target);

      moveStartTime = millis();
      motorState = MOVE;
      break;
    }

    case MOVE:
    {
      releaseAll();

      if ((fabs(target - posx) < 2.5 && fabs(u) < 5.0) ||
          (millis() - moveStartTime > 150))
      {
        motorState = ENERGIZE;
        stateStartTime = millis();
      }

      break;
    }

    case ENERGIZE:
    {
      if (!fingersActivated)
      {
        activateFingers(activeFinger1, activeFinger2);
        fingersActivated = true;
      }

      if (millis() - stateStartTime >= 50)
      {
        motorState = HOLD;
        stateStartTime = millis();
      }

      break;
    }

    case HOLD:
    {
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

      if (millis() - stateStartTime >= 50)
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
int getBestWhiteFinger(float keyPos, int forbiddenFinger)
{
  int whiteFingerIndices[] = {0, 1, 3}; // exclude 2 = black finger
  int bestFinger = -1;
  float minMove = 1e9;

  for (int j = 0; j < 3; j++)
  {
    int i = whiteFingerIndices[j];

    if (i == forbiddenFinger)
      continue;

    float requiredTarget = keyPos - fingerOffsets[i];
    float moveDist = fabs(requiredTarget - posx);

    if (moveDist < minMove)
    {
      minMove = moveDist;
      bestFinger = i;
    }
  }

  if (bestFinger == -1)
    bestFinger = 0;

  return bestFinger;
}

int chooseFinger(float keyPos, char keyType, int forbiddenFinger)
{
  if (keyType == 'B')
  {
    // black keys always use S3 = index 2
    if (forbiddenFinger == 2)
      return -1; // impossible if first note already took S3
    return 2;
  }

  return getBestWhiteFinger(keyPos, forbiddenFinger);
}

// ================= SOLENOIDS =================
void activateFingers(int finger1, int finger2)
{
  for (int i = 0; i < NUM_FINGERS; i++)
    digitalWrite(solenoidPins[i], LOW);

  if (finger1 >= 0)
    digitalWrite(solenoidPins[finger1], HIGH);

  if (finger2 >= 0)
    digitalWrite(solenoidPins[finger2], HIGH);
}

void releaseAll()
{
  for (int i = 0; i < NUM_FINGERS; i++)
    digitalWrite(solenoidPins[i], LOW);
}