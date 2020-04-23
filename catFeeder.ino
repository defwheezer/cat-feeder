
// cat feeder using two motors to turn paddles of 2 feeder tubes
// 12v motors driven by PWM on pins 10,11
// 5v arduino pro mini board
#include <SharpIR.h>

#define ir1 A0
#define ir2 A1
#define model 1080

SharpIR SharpIR_1(ir1, model);
SharpIR SharpIR_2(ir2, model);

#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment matrix = Adafruit_7segment();

int motorSpeed;
int motorMaxSpeed = 150;

int bowl_L_fills = 0; // how many times the bowl has been filled
int bowl_R_fills = 0;

const int  bowl_L_LED = 4;
const int  bowl_R_LED = 3;
const int  bowl_R_button = 8; //normally 1
const int  bowl_L_button = 7; //normally 0

bool bowl_L_Empty = false; // is TRUE while bowlmotor going
bool bowl_R_Empty = false;

const int  bowl_L_Sensor = A0; // bowlxSensors have a 10K pulldown resistor on it to decrease overall noise on all analog channels
const int  bowl_R_Sensor = A1;

int bowl_L_level;
int bowl_R_level;

const int  potentiometer1 = A2; //potentiometer 1
const int  potentiometer2 = A3; //potentiometer 2

const int  motor_L_Pin = 10;
const int  motor_R_Pin = 9;

unsigned long runTime1 = 0; //how long since last bowl 1 check
unsigned long runTime2 = 0; //how long since last bowl 2 check
unsigned long startTime1; //bowl 1
unsigned long startTime2; //bowl 2
uint16_t runSecs1 = 0;
uint16_t runMins_L = 0;
uint16_t runHrs1 = 0;
uint16_t runSecs2 = 0;
uint16_t runMins_R = 0;
uint16_t runHrs2 = 0;

//**********************************************************************

uint16_t bowl_L_EmptyLevel = 22; //IR threashold reading when bowl 1 (L) empty
uint16_t bowl_R_EmptyLevel = 21; //IR threashold reading when bowl 2 (R) empty

uint16_t bowl_1_Interval = 480; //minutes to wait between bowl fills
uint16_t bowl_2_Interval = 480; //minutes to wait between bowl fills

//**********************************************************************

uint16_t fillTries = 0; // increment each run of motor, after 3-5 stop (no food in hopper!)

int IR_1_distance;
int IR_2_distance;
  
void setup() {
  Serial.begin(9600);
  Serial.println("CatFeeder Test!");
  pinMode(motor_L_Pin, OUTPUT);
  pinMode(motor_R_Pin, OUTPUT);
  pinMode(bowl_L_LED, OUTPUT);
  pinMode(bowl_R_LED, OUTPUT);
  pinMode(bowl_R_button, INPUT);
  pinMode(bowl_L_button, INPUT);

  matrix.begin(0x70);
  matrix.setBrightness(1); //0-15
  startTime1 = millis();
  startTime2 = millis();
}

void loop() {
  runTime1 = millis() - startTime1;
  runTime2 = millis() - startTime2;
  runSecs1 = runTime1 / 1000;
  runMins_L = runSecs1 / 60;
  runHrs1 = runMins_L / 60;
  runSecs2 = runTime2 / 1000;
  runMins_R = runSecs2 / 60;
  runHrs2 = runMins_R / 60;

  readButtons(); // will read pots when activated
 
  if (bowl_L_level > bowl_L_EmptyLevel) {
    digitalWrite(bowl_L_LED, HIGH);
  }
  else digitalWrite(bowl_L_LED, LOW);

  if (bowl_R_level > bowl_R_EmptyLevel) {
    digitalWrite(bowl_R_LED, HIGH);
  }
  else digitalWrite(bowl_R_LED, LOW);

  readLevels();
  
  //run motor if bowls empty AND multiple of 12hrs AND at least 12 hrs from start
  //BOWL 1
  while (bowl_L_level >= bowl_L_EmptyLevel && runMins_L >= bowl_1_Interval && fillTries <= 2) {
    runMotor(motor_L_Pin);
    bowl_L_Empty = true;
    delay(500); //let level detector stablize before reading
    readLevels();
    delay(500);
    Serial.print("fillTries: ");
    Serial.print(fillTries);
    Serial.print(", bowl_L_level: ");
    Serial.println(bowl_L_level);
    fillTries = fillTries + 1;
  }
  if (bowl_L_Empty) {
    //reset start time so bowl only fills once per time period
    startTime1 = millis();
    bowl_L_Empty = false;
    fillTries = 0;
    bowl_L_fills = bowl_L_fills + 1;
    if (bowl_L_fills >= 10) {
      bowl_L_fills = 0; //reset so we always only have one digit to display
    }
  }
  //BOWL 2
  while (bowl_R_level >= bowl_R_EmptyLevel && runMins_R >= bowl_2_Interval && fillTries <= 2) {
    runMotor(motor_R_Pin);
    bowl_R_Empty = true;
    delay(500); //let level detector stablize before reading
    readLevels();
    delay(500);
    Serial.print("fillTries: ");
    Serial.print(fillTries);
    Serial.print(", bowl_R_level: ");
    Serial.println(bowl_R_level);
    fillTries = fillTries + 1;
  }
  if (bowl_R_Empty) {
    //reset start time so bowl only fills once per time period
    startTime2 = millis();
    bowl_R_Empty = false;
    fillTries = 0;
    bowl_R_fills = bowl_R_fills + 1;
    if (bowl_R_fills >= 10) {
      bowl_R_fills = 0; //reset so we always only have one digit to display
    }
  }

//  Serial.print("ave_bowl_1_Interval: ");
// Serial.print(ave_bowl_1_Interval);
// Serial.print(",");
// Serial.print(", ave_bowl_2_Interval: ");
// Serial.println(ave_bowl_2_Interval);
 //Serial.print(analogRead(bowl_L_Sensor));
 //Serial.print(",");
Serial.print("Left: Min = "); 
Serial.print(runMins_L);
Serial.print(", Level / Set = ");
Serial.print(bowl_L_level);
Serial.print(" / ");
Serial.print(bowl_L_EmptyLevel);
Serial.print(" , Right: Min = ");
Serial.print(runMins_R);
Serial.print(", Level / Set = ");
Serial.print(bowl_R_level);
Serial.print(" / ");
Serial.println(bowl_R_EmptyLevel);
 //Serial.print(",");
 //Serial.println(bowl1median);
//  Serial.print(", runMins_L:");
//  Serial.print(runMins_L);
//  Serial.print(", runMins_R:");
//  Serial.print(runMins_R);
//  Serial.print(", bowl_R_EmptyLevel:");
//  Serial.print(bowl_R_EmptyLevel);
//  Serial.print(", bowl_L_EmptyLevel:");
//  Serial.println(bowl_L_EmptyLevel);
  //  matrix.clear();
  //matrix.println(bowl_1_Interval-runMins_L);
  matrix.writeDigitNum(0, bowl_L_fills);
  matrix.writeDigitNum(4, bowl_R_fills);
  matrix.writeDisplay();
}

void readButtons() {
  if (!digitalRead(bowl_L_button)) {
    //readPots();
    digitalWrite(bowl_L_LED, HIGH);
    Serial.println("bowl_L_button");
    runMotor(motor_L_Pin);
  }
  if (digitalRead(bowl_R_button)) {
    digitalWrite(bowl_R_LED, HIGH);
    Serial.println("bowl_R_button");
    runMotor(motor_R_Pin);
  }

}

void readPots() {
  int pot_1 = analogRead(potentiometer1);
  pot_1 = (pot_1+1)/10;
  int pot_2 = analogRead(potentiometer2);
  pot_2 = (pot_2+1)/10;
  bowl_L_EmptyLevel = pot_1;
  bowl_R_EmptyLevel = pot_2;
  //Serial.print("pot_1: ");
  Serial.print(pot_1);
  Serial.print(",");
  //Serial.print("pot_2: ");
  Serial.print(pot_2);
  Serial.println("*");
}

void readLevels() {
  bowl_L_level=SharpIR_1.distance();
  delay(50);
  bowl_R_level=SharpIR_2.distance();
  
  // convert to distance from sensor on dispensor
  //bowl_L_level = 36 - bowl_L_level;
  //bowl_R_level = 36 - bowl_R_level;
  
  delay(50);
}


void displayDigits() {

  // draw each digit
  // writeDigitNum(location, number) - this will write the number (0-9) to a single location.
  // Location #0 is all the way to the left
  // Location #2 is the colon dots;  use drawColon(true or false)
  // Location #4 is all the way to the right.
  matrix.clear();
  //matrix.writeDigitNum(0,0);
  matrix.writeDigitNum(1, 0);
  matrix.drawColon(false);
  //matrix.writeDigitNum(3,0);
  matrix.writeDigitNum(4, 0);
  matrix.writeDisplay();
}

void runMotor(int m) {
  uint8_t i;
  for (i = 50; i < motorMaxSpeed; i++) {
    motorSpeed = i;
    analogWrite(m, motorSpeed);
    delay(5);
  }
  analogWrite(m, 0);
  delay(2000);
}
