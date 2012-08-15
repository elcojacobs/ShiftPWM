/************************************************************************************************************************************
 * ShiftPWM blocking RGB fades example, (c) Elco Jacobs, updated August 2012.
 *
 * ShiftPWM blocking RGB fades example. This example uses simple delay loops to create fades.
 * If you want to change the fading mode based on inputs (sensors, buttons, serial), use the non-blocking example as a starting point.
 * Please go to www.elcojacobs.com/shiftpwm for documentation, fuction reference and schematics.
 * If you want to use ShiftPWM with LED strips or high power LED's, visit the shop for boards.
 ************************************************************************************************************************************/
 
//#include <Servo.h> <-- If you include Servo.h, which uses timer1, ShiftPWM will automatically switch to timer2

// Clock and data pins are pins from the hardware SPI, you cannot choose them yourself if you use the hardware SPI.
// Data pin is MOSI (Uno and earlier: 11, Leonardo: ICSP 4, Mega: 51, Teensy 2.0: 2, Teensy 2.0++: 22) 
// Clock pin is SCK (Uno and earlier: 13, Leonardo: ICSP 3, Mega: 52, Teensy 2.0: 1, Teensy 2.0++: 21)

// You can choose the latch pin yourself.
const int ShiftPWM_latchPin=8;

// ** uncomment this part to NOT use the SPI port and change the pin numbers. This is 2.5x slower **
// #define SHIFTPWM_NOSPI
// const int ShiftPWM_dataPin = 11;
// const int ShiftPWM_clockPin = 13;


// If your LED's turn on if the pin is low, set this to true, otherwise set it to false.
const bool ShiftPWM_invertOutputs = false; 

// You can enable the option below to shift the PWM phase of each shift register by 8 compared to the previous.
// This will slightly increase the interrupt load, but will prevent all PWM signals from becoming high at the same time.
// This will be a bit easier on your power supply, because the current peaks are distributed.
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!

// Here you set the number of brightness levels, the update frequency and the number of shift registers.
// These values affect the load of ShiftPWM.
// Choose them wisely and use the PrintInterruptLoad() function to verify your load.
// There is a calculator on my website to estimate the load.

unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
int numRegisters = 6;
int numRGBleds = numRegisters*8/3;

void setup(){
  Serial.begin(9600);

  // Sets the number of 8-bit registers that are used.
  ShiftPWM.SetAmountOfRegisters(numRegisters);

  // SetPinGrouping allows flexibility in LED setup. 
  // If your LED's are connected like this: RRRRGGGGBBBBRRRRGGGGBBBB, use SetPinGrouping(4).
  ShiftPWM.SetPinGrouping(1); //This is the default, but I added here to demonstrate how to use the funtion
  
  ShiftPWM.Start(pwmFrequency,maxBrightness);
}



void loop()
{    
  // Turn all LED's off.
  ShiftPWM.SetAll(0);

  // Print information about the interrupt frequency, duration and load on your program
  ShiftPWM.PrintInterruptLoad();

  // Fade in and fade out all outputs one by one fast. Usefull for testing your hardware. Use OneByOneSlow when this is going to fast.
  ShiftPWM.OneByOneFast();

  // Fade in all outputs
  for(int j=0;j<maxBrightness;j++){
    ShiftPWM.SetAll(j);  
    delay(20);
  }
  // Fade out all outputs
  for(int j=maxBrightness;j>=0;j--){
    ShiftPWM.SetAll(j);  
    delay(20);
  }


  // Fade in and out 2 outputs at a time
  for(int output=0;output<numRegisters*8-1;output++){
    ShiftPWM.SetAll(0);
    for(int brightness=0;brightness<maxBrightness;brightness++){
      ShiftPWM.SetOne(output+1,brightness);
      ShiftPWM.SetOne(output,maxBrightness-brightness);
      delay(1);
    }
  }

  // Hue shift all LED's
  for(int hue = 0; hue<360; hue++){
    ShiftPWM.SetAllHSV(hue, 255, 255); 
    delay(50);
  }

  // Alternate LED's in 6 different colors
  for(int shift=0;shift<6;shift++){
    for(int led=0; led<numRGBleds; led++){
      switch((led+shift)%6){
      case 0:
        ShiftPWM.SetRGB(led,255,0,0);    // red
        break;
      case 1:
        ShiftPWM.SetRGB(led,0,255,0);    // green
        break;
      case 2:
        ShiftPWM.SetRGB(led,0,0,255);    // blue
        break;
      case 3:
        ShiftPWM.SetRGB(led,255,128,0);  // orange
        break;
      case 4:
        ShiftPWM.SetRGB(led,0,255,255);  // turqoise
        break;
      case 5:
        ShiftPWM.SetRGB(led,255,0,255);  // purple
        break;
      }
    }
    delay(2000);
  }

  // Update random LED to random color. Funky!
  for(int i=0;i<1000;i++){
    ShiftPWM.SetHSV(random(numRGBleds),random(360),255,255);
    delay(15);
  }


  // Immitate a VU meter
  int peak=0;
  int prevPeak=0;

  int currentLevel = 0;
  for(int i=0;i<40;i++){
    prevPeak = peak;
    while(abs(peak-prevPeak)<5){
      peak =  random(numRGBleds); // pick a new peak value that differs at least 5 from previous peak
    }
    // animate to new top
    while(currentLevel!=peak){
      if(currentLevel<peak){
        currentLevel++;
      }
      else{
        currentLevel--;
      }
      for(int led=0;led<numRGBleds;led++){
        if(led<=currentLevel){
          int hue = (numRGBleds-1-led)*120/numRGBleds; // From green to red
          ShiftPWM.SetHSV(led,hue,255,255); 
        }
        else{
          ShiftPWM.SetRGB(led,0,0,0);
        }
      }
      delay((64/numRGBleds)*(numRGBleds-currentLevel)); // go slower near the top
    }
  }

  //  A moving rainbow for RGB leds:
  rgbLedRainbow(numRGBleds, 5, 3, numRegisters*8/3); // Fast, over all LED's
  rgbLedRainbow(numRGBleds, 10, 3, numRegisters*8/3*4); //slower, wider than the number of LED's
}

void rgbLedRainbow(int numRGBLeds, int delayVal, int numCycles, int rainbowWidth){
  // Displays a rainbow spread over a few LED's (numRGBLeds), which shifts in hue. 
  // The rainbow can be wider then the real number of LED's.

  ShiftPWM.SetAll(0);
  for(int cycle=0;cycle<numCycles;cycle++){ // loop through the hue shift a number of times (numCycles)
    for(int colorshift=0;colorshift<360;colorshift++){ // Shift over full color range (like the hue slider in photoshop)
      for(int led=0;led<numRGBLeds;led++){ // loop over all LED's
        int hue = ((led)*360/(rainbowWidth-1)+colorshift)%360; // Set hue from 0 to 360 from first to last led and shift the hue
        ShiftPWM.SetHSV(led, hue, 255, 255); // write the HSV values, with saturation and value at maximum
      }
      delay(delayVal); // this delay value determines the speed of hue shift
    } 
  }  
}
