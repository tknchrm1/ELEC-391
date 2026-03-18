#include <stdio.h>
#include <stdbool.h>

#define LEDY 12
#define LEDR 14
#define LEDB 27
#define LEDG 26

enum MOTOR_STATE {
  READY,
  MOVING,
  BUSY
};

const int midiEvents[][4] = {
  {0,   1, 61, 50},
  {384, 0, 61, 0},
  {0,   1, 60, 50},
  {384, 0, 60, 0},
  {0,   1, 59, 50},
  {384, 0, 59, 0},
  {0,   1, 60, 50},
  {384, 0, 60, 0},
  {0,   1, 61, 50},
  {384, 0, 61, 0},
  {0,   1, 62, 50},
  {384, 0, 62, 0},
  {0,   1, 61, 50},
  {384, 0, 61, 0},
  {0,   1, 60, 50},
  {384, 0, 60, 0},
  {0,   1, 59, 50},
  {384, 0, 59, 0}
};

const int NUM_EVENTS = sizeof(midiEvents) / sizeof(midiEvents[0]);

int delay, status, note, velocity;

void setup() {
  Serial.begin(11520);
  Serial.print("Beginning Program\n");
  delay(500);

  bool solenoidState = OFF;
  enum MOTOR_STATE motorState = READY;

}

void loop() {
  
  int n = 0;

  while (n < NUM_EVENTS){

    delay = midiEvent[n][0];
    status = midiEvent[n][1];
    note = midiEvent[n][2];
    velocity = midiEvent[n][3];

  }

}
