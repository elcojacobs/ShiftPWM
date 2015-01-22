/******************************************************************************
 * //Comments yet to to be updated 
 * 
 * (c) Elco Jacobs, Sept 2011.
 * 
 *****************************************************************************/
//#include <Servo.h>
#include <SPI.h>
#include "hsv2rgb.h"


//Data pin is MOSI (atmega168/328: pin 11. Mega: 51) 
//Clock pin is SCK (atmega168/328: pin 13. Mega: 52)
const int ShiftMatrixPWM_columnLatchPin=9;
const int ShiftMatrixPWM_rowDataPin=6;
const int ShiftMatrixPWM_rowClockPin=7;
const int ShiftMatrixPWM_rowLatchPin=8;

const bool ShiftMatrixPWM_invertColumnOutputs = 1; // if invertColumnOutputs is 1, outputs will be active low. Usefull for common anode RGB led's.

const bool ShiftMatrixPWM_invertRowOutputs = 1; // if invertOutputs is 1, outputs will be active low. Used for PNP transistors.

#include <ShiftMatrixPWM.h>   // include ShiftMatrixPWM.h after setting the pins!


unsigned char maxBrightness = 63;
unsigned char pwmFrequency = 75;
int numColumnRegisters = 1;
int numRows=8;

int numColumns = numColumnRegisters*8;
int numOutputs = numColumns*numRows;


void setup()   {                
  pinMode(ShiftMatrixPWM_columnLatchPin, OUTPUT); 
  pinMode(ShiftMatrixPWM_rowDataPin, OUTPUT); 
  pinMode(ShiftMatrixPWM_rowClockPin, OUTPUT); 
  pinMode(ShiftMatrixPWM_rowLatchPin, OUTPUT); 
 
 
  
  SPI.setBitOrder(LSBFIRST);
  // SPI_CLOCK_DIV2 is only a tiny bit faster in sending out the last byte. 
  // SPI transfer and calculations overlap for the other bytes.
  SPI.setClockDivider(SPI_CLOCK_DIV4); 
  SPI.begin(); 

  Serial.begin(9600);


  ShiftMatrixPWM.SetMatrixSize(numRows, numColumnRegisters);
  ShiftMatrixPWM.Start(pwmFrequency,maxBrightness);  
}



void loop()
{    
  // Print information about the interrupt frequency, duration and load on your program
  ShiftMatrixPWM.SetAll(0);
  ShiftMatrixPWM.PrintInterruptLoad();

  // Fade in and fade out all outputs one by one fast. Usefull for testing your circuit
  ShiftMatrixPWM.OneByOneFast();


  // Fade in all outputs
  for(int j=0;j<maxBrightness;j++){
    ShiftMatrixPWM.SetAll(j);  
    delay(20);
  }
  // Fade out all outputs
  for(int j=maxBrightness;j>=0;j--){
    ShiftMatrixPWM.SetAll(j);  
    delay(20);
  }


  // Fade in and out 2 outputs at a time
  for(int row=0;row<numRows;row++){
    for(int col=0;col<numColumns-1;col++){
      ShiftMatrixPWM.SetAll(0);
      for(int brightness=0;brightness<maxBrightness;brightness++){
        ShiftMatrixPWM.SetOne(row, col+1,brightness);
        ShiftMatrixPWM.SetOne(row,col,maxBrightness-brightness);
        delay(10);
      }
    }
  }

  //  A moving rainbow for RGB leds (number of column registers should be a multiple of 3)
  if(numColumnRegisters%3==0){
    rgbLedRainbow(numOutputs/3, 5, 3, maxBrightness, numColumns/3); // Fast, over all LED's, 1 column wide rainbow
    rgbLedRainbow(numOutputs/3, 10, 3, maxBrightness, numOutputs/3); //slower, over all LED's, as wide as the whole matrix
  }

  // Fade in and fade out all outputs slowly. Usefull for testing your circuit
  ShiftMatrixPWM.OneByOneSlow();  

}

void rgbLedRainbow(int numRGBLeds, int delayVal, int numCycles, int maxBrightness, int rainbowWidth){
  // Displays a rainbow spread over all LED's, which shifts in hue.
  int hue, sat, val; 
  unsigned char red, green, blue;

  ShiftMatrixPWM.SetAll(0);
  for(int cycle=0;cycle<numCycles;cycle++){ // shift the raibom numCycles times
    for(int colorshift=0;colorshift<360;colorshift++){ // Shift over full color range (like the hue slider in photoshop)
      for(int led=0;led<numRGBLeds;led++){ // loop over all LED's
        hue = ((led)*360/(rainbowWidth-1)+colorshift)%360; // Set hue from 0 to 360 from first to last led and shift the hue
        sat = 255;
        val = 255;
        hsv2rgb(hue, sat, val, &red, &green, &blue, maxBrightness); // convert hsv to rgb values
        int group = led%(numColumns/3);
        int row = (led - group)/(numColumns/3);
        ShiftMatrixPWM.SetGroupOf3(row, group, red, green, blue); // write rgb values
      }
      delay(delayVal);
    } 
  }  
}
