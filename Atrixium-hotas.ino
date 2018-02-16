/*
  Thrustmaster HOTAS based on NicoHood Gamepad API - Rob Street 2016
  
  Copyright (c) 2014-2015 NicoHood
  See the readme for credit to other people.

  See HID Project documentation for more infos
  https://github.com/NicoHood/HID/wiki/Gamepad-API
*/

#include "HID-Project.h"
#include <SPI.h>

//### Debugging Variables ###
unsigned long StartTime = 0;
unsigned long EndTime = 0;

const bool DebugButtons = 0; // enable or disable debug messages, very slow!
const bool DebugAxes = 0; // enable or disable debug messages, very slow!


//### Define Pins ####
const int Load = 10; // Shift register load pin
const int StickX = A0;
const int StickY = A2;
const int POVHat = A1;
const int Rudder = A3;
const int Throttle = A9;
const int Analog5 = A5;


//### input variables ###

byte HighByte = 0b01010101;
byte LowByte = 0b10101010;
unsigned int Input = 0xCCCC;

int xRaw = 0; // 16BIT
int yRaw = 0; // 16BIT
int zRaw = 0; // 8BIT

int rxRaw = 0; // 16BIT
int ryRaw = 0; // 16BIT
int rzRaw = 0; // 8BIT

long xAxis = 0; // 16BIT
long yAxis = 0; // 16BIT
int zAxis = 0; // 8BIT

long rxAxis = 0; // 16BIT
long ryAxis = 0; // 16BIT
int rzAxis = 0; // 8BIT

int JOYSTICK_DEADZONE = 310;

//### Analog Averaging Variables ###
int samples = 5;

//###### Define Button Bits ########

unsigned int Button[16] = {0x1,0x2,0x4,0x8,
                   0x10,0x20,0x40,0x80,
                   0x100,0x200,0x400,0x800,
                   0x1000,0x2000,0x4000,0x8000};

//##################################

void setup() {
  pinMode(Load, OUTPUT);

  // Start SPI functions
  SPI.begin();

  // Debug console
  {
    if(DebugButtons || DebugAxes)
      Serial.begin(9600);
  }

  // Sends a clean report to the host. This is important on any Arduino type.
  Gamepad.begin();
}

void loop() {

  digitalWrite(Load, LOW); // Load shift registers
  digitalWrite(Load, HIGH); // Set shift registers to shifting mode
  HighByte = SPI.transfer(0xff); // Grab the first byte
  LowByte = SPI.transfer(0xff); // Grab the second byte
  
  if(DebugButtons)
  {
    Serial.print("HighByte: ");
    Serial.println(HighByte,BIN);
    Serial.print("LowByte: ");
    Serial.println(LowByte,BIN);
  }
  
  Input = HighByte * 256 + LowByte; // Combine high byte and low byte into one 16bit variable
  
  if(DebugButtons)
  {
    Serial.print("Input=  ");
    Serial.println(Input,BIN);
  }

    // Press buttons
    if(DebugButtons)
    {
      StartTime = micros(); // Get the start time for the button polling
    }
      
    for(int i=0; i<=15; i++)
    {
      if(Input & Button[i]) // Compare input to mask
      {
        Gamepad.press(i);
        if(DebugButtons)
        {
          Serial.print("Pressing: ");
          Serial.println(i);
        }
      }
      else
      {
        Gamepad.release(i);
        if(DebugButtons)
        {
          Serial.print("Releasing: ");
          Serial.println(i);
        }
      }
    }

    if(DebugButtons)
    {
      EndTime = micros(); // Get the end time for the button polling
      Serial.print("Polling took: ");
      Serial.println(EndTime - StartTime);
    }
    

    // Get X, Y, Z, RX, RY and RZ axes

    xRaw = GetAxis(StickX);
    yRaw = GetAxis(StickY);
    zRaw = GetAxis(POVHat); // 8bit
    
    rxRaw = GetAxis(Throttle);
    ryRaw = GetAxis(Rudder);
    //rzRaw = GetAxis(Analog5);// 8bit

    //Map X,Y,RX,RY to 16 Bit
    
    xAxis = map(xRaw, 0, 1023, -32767, 38000);
    xAxis = abs(xAxis) < JOYSTICK_DEADZONE ? 0 : xAxis;
    xAxis = constrain(xAxis, -32767,32767);
    
    yAxis = map(yRaw, 0, 1023, -32767, 40000);
    yAxis = abs(yAxis) < JOYSTICK_DEADZONE ? 0 : yAxis;
    yAxis = constrain(yAxis, -32767,32767);
    
    rxAxis = map(rxRaw, 0, 768, -32767, 32767);
    rxAxis = abs(rxAxis) < JOYSTICK_DEADZONE ? 0 : rxAxis;
    rxAxis = constrain(rxAxis, -32767,32767);
    
    ryAxis = map(ryRaw, 0, 521, -32767, 32767);
    // ryAxis = abs(ryAxis) < JOYSTICK_DEADZONE ? 0 : ryAxis;
    ryAxis = constrain(ryAxis, -32767,32767);
    
    // Map Z and RZ axes to 8 bit
    zAxis = map(zRaw, 210, 1023, -255, 254);
    //rzAxis = map(zRaw, 0, 1023, -255, 254);


    // Move Axes to a new position
    Gamepad.xAxis(xAxis);
    Gamepad.yAxis(yAxis);

    Gamepad.rxAxis(rxAxis);
    Gamepad.ryAxis(ryAxis);
    
    Gamepad.zAxis(zAxis);
    //Gamepad.rzAxis(rzAxis);

    
    if(DebugAxes)
    {
      Serial.print("X Raw: ");  
      Serial.println(xRaw);
      Serial.print("X Mapped: ");  
      Serial.println(xAxis);
      Serial.print("Y Raw: ");  
      Serial.println(yRaw);
      Serial.print("Y Mapped: ");  
      Serial.println(yAxis);
      Serial.print("Z Raw: ");  
      Serial.println(zRaw);
      Serial.print("Z Mapped: ");  
      Serial.println(zAxis);
      
      Serial.print("RX Raw: ");  
      Serial.println(rxRaw);
      Serial.print("RX Mapped: ");  
      Serial.println(rxAxis);
      Serial.print("RY Raw: ");  
      Serial.println(ryRaw);
      Serial.print("RY Mapped: ");  
      Serial.println(ryAxis);
      Serial.print("RZ Raw: ");  
      Serial.println(rzRaw);
      Serial.print("RZ Mapped: ");
      Serial.println(rzAxis);}
    // Functions above only set the values.
    
    // This writes the report to the host.
    Gamepad.write();

    // Simple debounce
    //delay(300);
}

int GetAxis(int Axis){
  int result = 0;
  for(int i=0; i<samples; i++){
    result = result + analogRead(Axis);  // Read an Axis
  }
  result = result / samples; // Get Average
  return result;
}

