#include <stdio.h>
#include <stdbool.h>

//Solenoid Pin Definitions
#define S1 12
#define S2 14
#define S3 27
#define S4 26

//This can be a look up table in the future, to clean up code
#define A4 0.1 //CHANGE THIS ASSUMES THAT E4 IS IN THE MIDDLE OF THE KEYBOARD
#define B4 0.2 //CHANGE THIS ASSUMES THAT E4 IS IN THE MIDDLE OF THE KEYBOARD
#define C4 0.3 //CHANGE THIS ASSUMES THAT E4 IS IN THE MIDDLE OF THE KEYBOARD
#define D4 0.4 //CHANGE THIS ASSUMES THAT E4 IS IN THE MIDDLE OF THE KEYBOARD
#define E4 0.5 //CHANGE THIS ASSUMES THAT E4 IS IN THE MIDDLE OF THE KEYBOARD
#define F4 0.6 //CHANGE THIS ASSUMES THAT E4 IS IN THE MIDDLE OF THE KEYBOARD
#define G4 0.7 //CHANGE THIS ASSUMES THAT E4 IS IN THE MIDDLE OF THE KEYBOARD

#define BPM 120
#define SECONDS_PER_MINUTE 60

enum MOTOR_STATE {
  READING,
  MOVING,
  PLAYING,
  RESET
};

//There is a ticks per beat number in the header that needs to be read

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

const double testArray[][3]{
  //Beats, Note, Duty
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
}

//const int NUM_EVENTS = sizeof(midiEvents) / sizeof(midiEvents[0]); //Number of reads that the program needs to make
const int NUM_EVENTS = sizeof(testArray) / sizeof(testArray[0]); //Number of reads that the program needs to make

//int delay, status, note, velocity;

double beats, position, duty; // 

bool solenoidState = OFF;
enum MOTOR_STATE motorState = READY;

void setup() {
  Serial.begin(11520);
  Serial.print("Beginning Program\n");
  delay(500);

}

void loop() {
  
  int n = 0;

//Example implementation of
  while (n < NUM_EVENTS){

  switch (motorState) {
    case READY:
      //Read Variables
        beats = testArray[n][0];
        position = testArray[n][0];
        duty = testArray[n][0];
      break;

    case MOVING:
      pidMove();
      break;

    case PLAYING:
      analogWrite(S1, duty);
      break;

    case RESET:
      analogWrite(S1, 0); //Reset Solenoid
      break;

    default:
      // Fallback safety
      break;
}
  



  }

  

}

//pidMove Function: Takes a target position and moves the hand there
void pidMove() {
  Serial.print("Moving...");
  Delay(500);
  Serial.print("Finished!");
}
