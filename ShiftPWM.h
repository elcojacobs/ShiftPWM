/*
ShiftPWM.h - Library for Arduino to PWM many outputs using shift registers - Version 1
Copyright (c) 2011 Elco Jacobs, Technical University of Eindhoven, department of 
Industrial Design, Electronics Atelier. All right reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/* 

This library is intended to control the outputs of a chain of shift registers. 
It uses software PWM to control the duty cycle of all shift register outputs.

This code is optimized for speed: the interrupt duration is minimal, so that 
the load of the interrupt on your program is minimal (or the amount of registers
and brightness levels is maximal. The SPI is used to send data to the shift regisers,
while the CPU is already calculating the next byte to send out.

Timer1 (16 bit) is used, unless it is already in use by the servo library. 
Then, timer2 (8 bit) will be configured as the interrupt timer, with a prescaler for 
the highest possible precision.

A timer interrupt is configured by ShiftPWM.Start(pwmFrequency,maxBrightness)
The interrupt frequency is set to pwmFrequency * (maxBrightness+1)
Each interrupt all duty cycles are compared to the counter and the corresponding pin
is written 1 or 0 based on the result. Then the counter is increased by one.

The duration of the interrupt depends on the number of shift registers (N).
T = 97 + 43*N (worst case)

The load of the interrupt function on your program can be calculated:
L = Interrupt frequency * interrupt duration / clock frequency
L = F*(Bmax+1)*(97+43*N)/F_CPU 
The duration also depends on the number of brightness levels, but the impact is minimal.


The following functions are used:

ShiftPWM.Start(int ledFrequency, int max_Brightness)		Enable ShiftPWM with desired frequency and brightness levels
SetAmountOfRegisters(int newAmount)						Set or change the amount of output registers. Can be changed at runtime.
PrintInterruptLoad(void)									Print information on timer usage, frequencies and interrupt load
void OneByOne(void)										Fade in and fade out all outputs slowly
void OneByOneFast(void)									Fade in and fade out all outputs fast
void SetOne(int pin, unsigned char value)					Set the duty cycle of one output
void SetAll(unsigned char value)							Set all outputs to the same duty cycle

SetGroupOf2(int group, unsigned char v0, unsigned char v1);
SetGroupOf3(int group, unsigned char v0, unsigned char v1, unsigned char v2);
SetGroupOf4(int group, unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3);
SetGroupOf5(int group, unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3, unsigned char v4);
--> Set a group of outputs to the given values. SetGroupOf3 is useful for RGB LED's. Each LED will be a group.

*/

#ifndef ShiftPWM_H
#define ShiftPWM_H

#include "pins_arduino_compile_time.h" // My own version of pins arduino, which does not define the arrays in program memory
#if defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif

#include "CShiftPWM.h"


// These should be defined in the file where ShiftPWM.h is included.
extern const int ShiftPWM_latchPin;
extern const bool ShiftPWM_invertOutputs;

// The ShiftPWM object is created in the header file, instead of defining it as extern here and creating it in the cpp file.
// If the ShiftPWM object is created in the cpp file, it is separately compiled with the library.
// The compiler cannot treat it as constant and cannot optimize well: it will generate many memory accesses in the interrupt function.

#ifndef _useTimer1 //This is defined in Servo.h
CShiftPWM ShiftPWM(1);  
#else
CShiftPWM ShiftPWM(2);  // if timer1 is in use by servo, use timer 2
#endif


// The macro below uses 3 instructions per pin to generate the byte to transfer with SPI
// Retreive duty cycle setting from memory (ldd, 2 clockcycles)
// Compare with the counter (cp, 1 clockcycle) --> result is stored in carry
// Use the rotate over carry right to shift the compare result into the byte. (1 clockcycle).
#define add_one_pin_to_byte(sendbyte, counter, ledPtr) \
{ \ 
unsigned char pwmval=*ledPtr; \ 
	asm volatile ("cp %0, %1" : /* No outputs */ : "r" (counter), "r" (pwmval): ); \
	asm volatile ("ror %0" : "+r" (sendbyte) : "r" (sendbyte) : ); 			\
}


static inline void ShiftPWM_handleInterrupt(void){
	sei(); //enable interrupt nesting to prevent disturbing other interrupt functions (servo's for example).

	// Look up which bit of which output register corresponds to the pin.
	// This should be constant, so the compiler can optimize this code away and use sbi and cbi instructions
	// The compiler only knows this if this function is compiled in the same file as the pin setting.
	// That is the reason the full funcion is in the header file, instead of only the prototype.
	// If this function is defined in cpp files of the library, it is compiled seperately from the main file.
	// The compiler does not recognize the pins/ports as constant and sbi and cbi instructions cannot be used.


	volatile uint8_t * const latchPort = port_to_output_PGM_ct[digital_pin_to_port_PGM_ct[ShiftPWM_latchPin]];
	const uint8_t latchBit =  digital_pin_to_bit_PGM_ct[ShiftPWM_latchPin];


	// Define a pointer that will be used to access the values for each output. 
	// Let it point one past the last value, because it is decreased before it is used.

	unsigned char * ledPtr=&ShiftPWM.m_PWMValues[ShiftPWM.m_amountOfOutputs];

	// Write shift register latch clock low 
	bitClear(*latchPort, latchBit);
	unsigned char counter = ShiftPWM.counter;
	SPDR = 0; // write bogus bit to the SPI, because in the loop there is a receive before send.
	for(unsigned char i =ShiftPWM.m_amountOfRegisters; i>0;--i){   // do a whole shift register at once. This unrolls the loop for extra speed
		unsigned char sendbyte;  // no need to initialize, all bits are replaced

		add_one_pin_to_byte(sendbyte, counter, --ledPtr);
		add_one_pin_to_byte(sendbyte, counter,  --ledPtr);
		add_one_pin_to_byte(sendbyte, counter,  --ledPtr);
		add_one_pin_to_byte(sendbyte, counter,  --ledPtr);

		add_one_pin_to_byte(sendbyte, counter,  --ledPtr);
		add_one_pin_to_byte(sendbyte, counter,  --ledPtr);
		add_one_pin_to_byte(sendbyte, counter,  --ledPtr);
		add_one_pin_to_byte(sendbyte, counter,  --ledPtr);

		while (!(SPSR & _BV(SPIF)));    // wait for last send to finish and retreive answer. Retreive must be done, otherwise the SPI will not work.
		if(ShiftPWM_invertOutputs){	
			sendbyte = ~sendbyte; // Invert the byte if needed.
		}
		SPDR = sendbyte; // Send the byte to the SPI
	}
	while (!(SPSR & _BV(SPIF))); // wait for last send to complete.

	// Write shift register latch clock high 
	bitSet(*latchPort, latchBit);

	if(ShiftPWM.counter<ShiftPWM.m_maxBrightness){
		ShiftPWM.counter++; // Increase the counter
	}
	else{
		ShiftPWM.counter=0; // Reset counter if it maximum brightness has been reached
	} 	
} 

// See table  11-1 for the interrupt vectors */
#ifndef _useTimer1 
//Install the Interrupt Service Routine (ISR) for Timer1 compare and match A.
ISR(TIMER1_COMPA_vect) {
	ShiftPWM_handleInterrupt();
}
#else
//Install the Interrupt Service Routine (ISR) for Timer2 compare and match A.
ISR(TIMER2_COMPA_vect) {
	ShiftPWM_handleInterrupt();
}
#endif

// #endif for include once.
#endif 