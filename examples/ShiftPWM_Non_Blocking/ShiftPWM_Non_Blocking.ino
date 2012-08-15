/************************************************************************************************************************************
 * ShiftPWM non-blocking RGB fades example, (c) Elco Jacobs, updated August 2012.
 *
 * This example for ShiftPWM shows how to control your LED's in a non-blocking way: no delay loops.
 * This example receives a number from the serial port to set the fading mode. Instead you can also read buttons or sensors.
 * It uses the millis() function to create fades. The block fades example might be easier to understand, so start there.
 * 
 * Please go to www.elcojacobs.com/shiftpwm for documentation, fuction reference and schematics.
 * If you want to use ShiftPWM with LED strips or high power LED's, visit the shop for boards.
 ************************************************************************************************************************************/
 
//#include <Servo.h> <-- If you include Servo.h, which uses timer1, ShiftPWM will automatically switch to timer2

// Clock and data pins are pins from the hardware SPI, you cannot choose them yourself.
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
unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
int numRegisters = 6;
int numOutputs = numRegisters*8;
int numRGBLeds = numRegisters*8/3;
int fadingMode = 0; //start with all LED's off.

unsigned long startTime = 0; // start time for the chosen fading mode

void setup()   {                
  Serial.begin(9600);

  // Sets the number of 8-bit registers that are used.
  ShiftPWM.SetAmountOfRegisters(numRegisters);

  // SetPinGrouping allows flexibility in LED setup. 
  // If your LED's are connected like this: RRRRGGGGBBBBRRRRGGGGBBBB, use SetPinGrouping(4).
  ShiftPWM.SetPinGrouping(1); //This is the default, but I added here to demonstrate how to use the funtion
  
  ShiftPWM.Start(pwmFrequency,maxBrightness);
  printInstructions();
}



void loop()
{    
  if(Serial.available()){
    if(Serial.peek() == 'l'){
      // Print information about the interrupt frequency, duration and load on your program
      ShiftPWM.PrintInterruptLoad();
      Serial.flush();
    }
    else{
      fadingMode = Serial.parseInt(); // read a number from the serial port to set the mode
      Serial.print("Mode set to "); 
      Serial.print(fadingMode); 
      Serial.print(": ");
      startTime = millis();
      switch(fadingMode){
      case 0:
        Serial.println("All LED's off");
        break;
      case 1:
        Serial.println("Fade in and out one by one");
        break;
      case 2:
        Serial.println("Fade in and out all LED's");
        break;
      case 3:
        Serial.println("Fade in and out 2 LED's in parallel");
        break;
      case 4:
        Serial.println("Alternating LED's in 6 different colors");
        break;
      case 5:
        Serial.println("Hue shift all LED's");
        break;
      case 6:
        Serial.println("Setting random LED's to random color");
        break;
      case 7:
        Serial.println("Fake a VU meter");
        break;
      case 8:
        Serial.println("Display a color shifting rainbow as wide as the LED's");
        break;         
      case 9:
        Serial.println("Display a color shifting rainbow wider than the LED's");
        break;                 
      default:
        Serial.println("Unknown mode!");
        break;
      }
    } 
  }
  unsigned char brightness;
  switch(fadingMode){
  case 0:
    // Turn all LED's off.
    ShiftPWM.SetAll(0);
    break;
  case 1:
    oneByOne();
    break;
  case 2:
    inOutAll();
    break;
  case 3:
    inOutTwoLeds();
    break;
  case 4:
    alternatingColors();
    break;
  case 5:
    hueShiftAll();
    break;
  case 6:
    randomColors();
    break;
  case 7:
    fakeVuMeter();
    break;
  case 8:
    rgbLedRainbow(3000,numRGBLeds);
    break;
  case 9:
    rgbLedRainbow(10000,5*numRGBLeds);    
    break;   
  default:
    Serial.println("Unknown Mode!");
    delay(1000);
    break;
  }
}

void oneByOne(void){ // Fade in and fade out all outputs one at a time
  unsigned char brightness;
  unsigned long fadeTime = 500;
  unsigned long loopTime = numOutputs*fadeTime*2;
  unsigned long time = millis()-startTime;
  unsigned long timer = time%loopTime;
  unsigned long currentStep = timer%(fadeTime*2);

  int activeLED = timer/(fadeTime*2);

  if(currentStep <= fadeTime ){
    brightness = currentStep*maxBrightness/fadeTime; ///fading in
  }
  else{
    brightness = maxBrightness-(currentStep-fadeTime)*maxBrightness/fadeTime; ///fading out;
  }
  ShiftPWM.SetAll(0);
  ShiftPWM.SetOne(activeLED, brightness);
}

void inOutTwoLeds(void){ // Fade in and out 2 outputs at a time
  unsigned long fadeTime = 500;
  unsigned long loopTime = numOutputs*fadeTime;
  unsigned long time = millis()-startTime;
  unsigned long timer = time%loopTime;
  unsigned long currentStep = timer%fadeTime;

  int activeLED = timer/fadeTime;
  unsigned char brightness = currentStep*maxBrightness/fadeTime;

  ShiftPWM.SetAll(0);
  ShiftPWM.SetOne((activeLED+1)%numOutputs,brightness);
  ShiftPWM.SetOne(activeLED,maxBrightness-brightness);
}

void inOutAll(void){  // Fade in all outputs
  unsigned char brightness;
  unsigned long fadeTime = 2000;
  unsigned long time = millis()-startTime;
  unsigned long currentStep = time%(fadeTime*2);

  if(currentStep <= fadeTime ){
    brightness = currentStep*maxBrightness/fadeTime; ///fading in
  }
  else{
    brightness = maxBrightness-(currentStep-fadeTime)*maxBrightness/fadeTime; ///fading out;
  }
  ShiftPWM.SetAll(brightness);
}

void alternatingColors(void){ // Alternate LED's in 6 different colors
  unsigned long holdTime = 2000;
  unsigned long time = millis()-startTime;
  unsigned long shift = (time/holdTime)%6;
  for(int led=0; led<numRGBLeds; led++){
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
}

void hueShiftAll(){  // Hue shift all LED's
  unsigned long cycleTime = 10000;
  unsigned long time = millis()-startTime;
  unsigned long hue = (360*time/cycleTime)%360;
  ShiftPWM.SetAllHSV(hue, 255, 255); 
}

void randomColors(){  // Update random LED to random color. Funky!
  unsigned long updateDelay = 100;
  static unsigned long previousUpdateTime;
  if(millis()-previousUpdateTime > updateDelay){
    previousUpdateTime = millis();
    ShiftPWM.SetHSV(random(numRGBLeds),random(360),255,255);
  }
}

void fakeVuMeter(void){ // immitate a VU meter
  static int peak = 0;
  static int prevPeak = 0;
  static int currentLevel = 0;
  static unsigned long fadeStartTime = startTime;

  unsigned char brightness;
  unsigned long fadeTime = (currentLevel*2);// go slower near the top

  unsigned long time = millis()-fadeStartTime;
  unsigned long currentStep = time%(fadeTime);

  if(currentLevel==peak){
    // get a new peak value
    prevPeak = peak;
    while(abs(peak-prevPeak)<5){
      peak =  random(numRGBLeds); // pick a new peak value that differs at least 5 from previous peak
    }
  }

  if(millis()-fadeStartTime > fadeTime){
    fadeStartTime = millis();
    if(currentLevel<peak){ //fading in
      currentLevel++;
    }
    else{ //fading out
      currentLevel--;
    }
  }
  // animate to new top
  for(int led=0;led<numRGBLeds;led++){
    if(led<currentLevel){
      int hue = (numRGBLeds-1-led)*120/numRGBLeds; // From green to red
      ShiftPWM.SetHSV(led,hue,255,255); 
    }
    else if(led==currentLevel){
      int hue = (numRGBLeds-1-led)*120/numRGBLeds; // From green to red
      int value;
      if(currentLevel<peak){ //fading in        
        value = time*255/fadeTime;
      }
      else{ //fading out
        value = 255-time*255/fadeTime;
      }
      ShiftPWM.SetHSV(led,hue,255,value);       
    }
    else{
      ShiftPWM.SetRGB(led,0,0,0);
    }
  }
}

void rgbLedRainbow(unsigned long cycleTime, int rainbowWidth){
  // Displays a rainbow spread over a few LED's (numRGBLeds), which shifts in hue. 
  // The rainbow can be wider then the real number of LED's.
  unsigned long time = millis()-startTime;
  unsigned long colorShift = (360*time/cycleTime)%360; // this color shift is like the hue slider in Photoshop.

  for(int led=0;led<numRGBLeds;led++){ // loop over all LED's
    int hue = ((led)*360/(rainbowWidth-1)+colorShift)%360; // Set hue from 0 to 360 from first to last led and shift the hue
    ShiftPWM.SetHSV(led, hue, 255, 255); // write the HSV values, with saturation and value at maximum
  }
}

void printInstructions(void){
  Serial.println("---- ShiftPWM Non-blocking fades demo ----");
  Serial.println("");
  
  Serial.println("Type 'l' to see the load of the ShiftPWM interrupt (the % of CPU time the AVR is busy with ShiftPWM)");
  Serial.println("");
  Serial.println("Type any of these numbers to set the demo to this mode:");
  Serial.println("  0. All LED's off");
  Serial.println("  1. Fade in and out one by one");
  Serial.println("  2. Fade in and out all LED's");
  Serial.println("  3. Fade in and out 2 LED's in parallel");
  Serial.println("  4. Alternating LED's in 6 different colors");
  Serial.println("  5. Hue shift all LED's");
  Serial.println("  6. Setting random LED's to random color");
  Serial.println("  7. Fake a VU meter");
  Serial.println("  8. Display a color shifting rainbow as wide as the LED's");
  Serial.println("  9. Display a color shifting rainbow wider than the LED's");  
  Serial.println("");
  Serial.println("Type 'm' to see this info again");  
  Serial.println("");
  Serial.println("----");
}






