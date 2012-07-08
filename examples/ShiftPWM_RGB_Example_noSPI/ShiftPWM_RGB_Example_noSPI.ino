/******************************************************************************
 * This example shows how to use the ShiftPWM library to PWM many outputs.
 * All shift registers are chained, so they can be driven with 3 pins from the arduino.
 * A timer interrupt updates all PWM outputs according to their duty cycle setting.
 * The outputs can be inverted by making ShiftPWM_invertOutputs true.
 * 
 * How the library works:
 * The ShiftPWM class keeps a setting for the duty cycle for each output pin, which
 * can be set using the provided functions. It also keeps a counter which it compares 
 * to these duty cycles. This timer continuously runs from 0 to the maximum duty cycle.
 * 
 * A timer interrupt is configured by ShiftPWM.Start(pwmFrequency,maxBrightness).
 * The interrupt frequency is set to pwmFrequency * (maxBrightness+1).
 * Each interrupt all duty cycles are compared to the counter and the corresponding pin
 * is written 1 or 0 based on the result. Then the counter is increased by one.
 * 
 * The duration of the interrupt depends on the number of shift registers (N). The worst case is:
 * T = 80 + 104*N
 * 
 * The load of the interrupt function on your program can be calculated:
 * L = Interrupt frequency * interrupt duration / clock frequency
 * L = F*(Bmax+1)*(80+104*N)/F_CPU
 * Quick reference for load:
 * 3 registers  255 maxBrightness 75Hz  load = 0.47
 * 6 registers  150 maxBrightness 75Hz  load = 0.50
 * 24 registers  50 maxBrightness 75Hz  load = 0.62
 * 48 registers  25 maxBrightness 75Hz  load = 0.62
 * 
 * 
 * A higher interrupt load will mean less computional power for your main program,
 * so try to keep it as low as possible and at least below 0.9.
 * 
 * The following functions are available:
 * 
 * ShiftPWM.Start(int ledFrequency, int max_Brightness)		Enable ShiftPWM with desired frequency and brightness levels
 * ShiftPWM.SetAmountOfRegisters(int newAmount)			Set or change the amount of output registers. Can be changed at runtime.
 * ShiftPWM.PrintInterruptLoad()				Print information on timer usage, frequencies and interrupt load
 * ShiftPWM.OneByOneSlow()  				        Fade in and fade out all outputs slowly
 * ShiftPWM.OneByOneFast()					Fade in and fade out all outputs fast
 * ShiftPWM.SetOne(int pin, unsigned char value)		Set the duty cycle of one output
 * ShiftPWM.SetAll(unsigned char value)				Set all outputs to the same duty cycle
 * 
 * ShiftPWM.SetGroupOf2(int group, unsigned char v0, unsigned char v1);
 * ShiftPWM.SetGroupOf3(int group, unsigned char v0, unsigned char v1, unsigned char v2);
 * ShiftPWM.SetGroupOf4(int group, unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3);
 * ShiftPWM.SetGroupOf5(int group, unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3, unsigned char v4);
 * 		--> Set a group of outputs to the given values. SetGroupOf3 is useful for RGB LED's. Each LED will be a group.
 * 
 * ShiftPWM.SetRGB(int led, unsigned char r,unsigned char g,unsigned char b);      // Set one LED to an RGB value
 * ShiftPWM.SetAllRGB(unsigned char r,unsigned char g,unsigned char b);            // Set all LED's to an RGB value
 * ShiftPWM.SetHSV(int led, unsigned int hue, unsigned int sat, unsigned int val); // Set one LED to an HSV value
 * ShiftPWM.SetAllHSV(unsigned int hue, unsigned int sat, unsigned int val);       // Set one LED to an HSV value
 * Note: the RGB and HSV functions assume that the outputs are RGBRGBRGB... without gaps. More flexibility in setup will be added soon.
 *
 * Debug information for wrong input to functions is also send to the serial port,
 * so check the serial port when you run into problems.
 * 
 * ShiftPWM v1.1, (c) Elco Jacobs, May 2012.
 * 
 *****************************************************************************/

//#include <Servo.h> <-- If you include Servo.h, which uses timer1, ShiftPWM will automatically switch to timer2
#define SHIFTPWM_NOSPI

// You can choose all pins yourself, but this version is slower than the SPI version
const int ShiftPWM_dataPin =7;
const int ShiftPWM_clockPin=8;
const int ShiftPWM_latchPin=9;

// If your LED's turn on if the pin is low, set this to 1, otherwise set it to 0.
const bool ShiftPWM_invertOutputs = 0; 

#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!

// Here you set the number of brightness levels, the update frequency and the number of shift registers.
// These values affect the load of ShiftPWM.
// Choose them wisely and use the PrintInterruptLoad() function to verify your load.
unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
int numRegisters = 3;
int numRGBleds = numRegisters*8/3;

void setup()   {                
  pinMode(ShiftPWM_latchPin, OUTPUT);
  pinMode(ShiftPWM_clockPin, OUTPUT);
  pinMode(ShiftPWM_dataPin,  OUTPUT);

// The example is the same as the SPI version below this line 

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
    ShiftPWM.SetHSV(random(numRGBleds),random(255),255,255);
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
      delay(3*(numRGBleds-currentLevel)); // go slower near the top
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
