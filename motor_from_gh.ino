/*******************************************************
 * Copyright (C) Chris Kang {- All Rights Reserved
 * 
 * Written by Chris Kang <danhwee9@gmail.com>, April 2021
 *******************************************************/

//import library
#include <Dynamixel2Arduino.h>
#define DXL_SERIAL   Serial3 //OpenCM9.04 EXP Board's DXL port Serial. (Serial1 for the DXL port on the OpenCM 9.04 board)
#define DEBUG_SERIAL Serial

const uint8_t DXL_DIR_PIN = 22; //OpenCM9.04 EXP Board's DIR PIN. (28 for the DXL port on the OpenCM 9.04 board)

#define BOARD_BUTTON_PIN 23  

//unique motor ID
const uint8_t DXL_3 = 3;
const uint8_t DXL_4 = 4;
const uint8_t DXL_5 = 5;

const float DXL_PROTOCOL_VERSION = 2.0;

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

//This namespace is required to use Control table item names
using namespace ControlTableItem;

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];
boolean newData = false;
char angleThree[32];
char angleFour[32];
char angleFive[32];
char boolMag[32];

float temp3 = 0;
float temp4 = 0;
float temp5 = 0;
float tempMag = 0;

int magnet = 18;
int push_state = 0;
int dir = 0;

void setup() {

  // Use UART port of DYNAMIXEL Shield to debug.
  DEBUG_SERIAL.begin(115200);
  while(!DEBUG_SERIAL);
  
  pinMode(BOARD_BUTTON_PIN, INPUT_PULLDOWN);
  pinMode(magnet, OUTPUT);
  
  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(57600);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  // Get DYNAMIXEL information
  initialize();
  setOpMode();

  
  //dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_3, 30);
  dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_4, 30);
  //dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_5, 30);

  dxl.writeControlTableItem(DRIVE_MODE, DXL_4, 1);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  printPosition();

  if (DEBUG_SERIAL.available() != 0) {
    recvWithStartEndMarkers();
    if(newData == true) {
      strcpy(tempChars, receivedChars);
      parseData();

      temp3 = atoi(angleThree);
      temp4 = atoi(angleFour);
      temp5 = atoi(angleFive);
      tempMag = atoi(boolMag);
      
      dxl.setGoalPosition(DXL_3, temp3, UNIT_DEGREE);
      dxl.setGoalPosition(DXL_4, temp4, UNIT_DEGREE);
      dxl.setGoalPosition(DXL_5, temp5, UNIT_DEGREE);
   
      if (tempMag == 1) {
        digitalWrite(magnet, HIGH);
      }else {
        digitalWrite(magnet, LOW);
      }
      
      showParsedData();
      
      
      newData = false;
     
    }
  }
  
}


void printPosition() {
  
  DEBUG_SERIAL.print("Current position - ID 3 = ");
  DEBUG_SERIAL.print(dxl.getPresentPosition(DXL_3,UNIT_DEGREE));
  DEBUG_SERIAL.print(" ID 4 = ");
  DEBUG_SERIAL.print(dxl.getPresentPosition(DXL_4,UNIT_DEGREE));
  DEBUG_SERIAL.print(" ID 5 = ");
  DEBUG_SERIAL.println(dxl.getPresentPosition(DXL_5,UNIT_DEGREE));
}

void initialize() {
  dxl.ping(DXL_3);
  dxl.ping(DXL_4);
  dxl.ping(DXL_5);

  dxl.ledOn(DXL_3);
  dxl.ledOn(DXL_4);
  dxl.ledOn(DXL_5);
  
  DEBUG_SERIAL.println("shit's bussin");


}

void setOpMode() {
   // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(DXL_3);
  dxl.torqueOff(DXL_4);
  dxl.torqueOff(DXL_5);
  
  dxl.setOperatingMode(DXL_3, OP_POSITION);
  dxl.setOperatingMode(DXL_4, OP_EXTENDED_POSITION);
  dxl.setOperatingMode(DXL_5, OP_POSITION);
  
  dxl.torqueOn(DXL_3);
  dxl.torqueOn(DXL_4);
  dxl.torqueOn(DXL_5);
}


void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
    while (DEBUG_SERIAL.available() > 0 && newData == false) {
        rc = DEBUG_SERIAL.read();
        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }
        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}


void parseData() {

    char * strtokIndx;
    strtokIndx = strtok(tempChars,",");
    strcpy(angleThree, strtokIndx);
    strtokIndx = strtok(NULL,",");
    strcpy(angleFour, strtokIndx);
    strtokIndx = strtok(NULL,",");
    strcpy(angleFive, strtokIndx);
    strtokIndx = strtok(NULL,",");
    strcpy(boolMag, strtokIndx);
    
}

void showParsedData() {
  DEBUG_SERIAL.print("first angle : ");
  DEBUG_SERIAL.println(angleThree);
  DEBUG_SERIAL.print("second angle : ");
  DEBUG_SERIAL.println(angleFour);
  DEBUG_SERIAL.print("third angle : ");
  DEBUG_SERIAL.println(angleFive);
  DEBUG_SERIAL.print("magnet : ");
  DEBUG_SERIAL.println(boolMag);

}

void goToPos(int id, float value) {
  dxl.setGoalPosition(id, value,UNIT_DEGREE);
  delay(10);
}
