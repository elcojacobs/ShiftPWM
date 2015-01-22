/*
ShiftMatrixPWM.h - Library for Arduino to PWM many outputs using shift registers - Version 1
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

A timer interrupt is configured by ShiftMatrixPWM.Start(pwmFrequency,maxBrightness)
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

ShiftMatrixPWM.Start(int ledFrequency, int max_Brightness)		Enable ShiftMatrixPWM with desired frequency and brightness levels
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

#ifndef ShiftMatrixPWM_h
#define ShiftMatrixPWM_h

#include "pins_arduino_compile_time.h" // My own version of pins arduino, which does not define the arrays in program memory
#include <WProgram.h>
#include "CShiftMatrixPWM.h"

// These should be defined in the file where ShiftMatrixPWM.h is included.
extern const int ShiftMatrixPWM_columnLatchPin;
extern const bool ShiftMatrixPWM_invertColumnOutputs;

extern const int ShiftMatrixPWM_rowLatchPin;
extern const int ShiftMatrixPWM_rowClockPin;
extern const int ShiftMatrixPWM_rowDataPin;
extern const bool ShiftMatrixPWM_invertRowOutputs;


// The ShiftMatrixPWM object is created in the header file, instead of defining it as extern here and creating it in the cpp file.
// If the ShiftMatrixPWM object is created in the cpp file, it is separately compiled with the library.
// The compiler cannot treat it as constant and cannot optimize well: it will generate many memory accesses in the interrupt function.

#ifndef _useTimer1 //This is defined in Servo.h
CShiftMatrixPWM ShiftMatrixPWM(1);  
#else
CShiftMatrixPWM ShiftMatrixPWM(2);  // if timer1 is in use by servo, use timer 2
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


static inline void ShiftMatrixPWM_handleInterrupt(void){
	sei(); //enable interrupt nesting to prevent disturbing other interrupt functions (servo's for example).

	// Look up which bit of which output register corresponds to the pin.
	// This should be constant, so the compiler can optimize this code away and use sbi and cbi instructions
	// The compiler only knows this if this function is compiled in the same file as the pin setting.
	// That is the reason the full function is in the header file, instead of only the prototype.
	// If this function is defined in cpp files of the library, it is compiled seperately from the main file.
	// The compiler does not recognize the pins/ports as constant and sbi and cbi instructions cannot be used.

	volatile uint8_t * const rowLatchPort = port_to_output_PGM_ct[digital_pin_to_port_PGM_ct[ShiftMatrixPWM_rowLatchPin]];
	volatile uint8_t * const rowClockPort = port_to_output_PGM_ct[digital_pin_to_port_PGM_ct[ShiftMatrixPWM_rowClockPin]];
	volatile uint8_t * const rowDataPort = port_to_output_PGM_ct[digital_pin_to_port_PGM_ct[ShiftMatrixPWM_rowDataPin]];
	const uint8_t rowLatchBit =  digital_pin_to_bit_PGM_ct[ShiftMatrixPWM_rowLatchPin];
	const uint8_t rowClockBit =  digital_pin_to_bit_PGM_ct[ShiftMatrixPWM_rowClockPin];
	const uint8_t rowDataBit =  digital_pin_to_bit_PGM_ct[ShiftMatrixPWM_rowDataPin];

	volatile uint8_t * const columnLatchPort = port_to_output_PGM_ct[digital_pin_to_port_PGM_ct[ShiftMatrixPWM_columnLatchPin]];
	const uint8_t columnLatchBit =  digital_pin_to_bit_PGM_ct[ShiftMatrixPWM_columnLatchPin];

	// Define a pointer that will be used to access the values for each output. 
	// Let it point one past the last value of the row, because it is decreased before it is used.
	unsigned char * ledPtr=&ShiftMatrixPWM.m_PWMValues[ShiftMatrixPWM.m_amountOfColumns*(ShiftMatrixPWM.m_currentRow+1)];
	unsigned char counter = ShiftMatrixPWM.m_counter;
	
	// Write column shift registers latch clock low 
	
	if(ShiftMatrixPWM.m_counter<ShiftMatrixPWM.m_maxBrightness){
		bitClear(*columnLatchPort, columnLatchBit);	
		SPDR = 0; // write bogus bit to the SPI, because in the loop there is a receive before send.
		for(unsigned char i =ShiftMatrixPWM.m_amountOfColumnRegisters; i>0;--i){   // do a whole shift register at once. This unrolls the loop for extra speed
			unsigned char sendbyte;  // no need to initialize, all bits are replaced

			add_one_pin_to_byte(sendbyte, counter, --ledPtr);
			add_one_pin_to_byte(sendbyte, counter,  --ledPtr);
			add_one_pin_to_byte(sendbyte, counter,  --ledPtr);
			add_one_pin_to_byte(sendbyte, counter,  --ledPtr);

			add_one_pin_to_byte(sendbyte, counter,  --ledPtr);
			add_one_pin_to_byte(sendbyte, counter,  --ledPtr);
			add_one_pin_to_byte(sendbyte, counter,  --ledPtr);
			add_one_pin_to_byte(sendbyte, counter,  --ledPtr);
			if(ShiftMatrixPWM_invertColumnOutputs){	
				sendbyte = ~sendbyte; // Invert the byte if needed.
			}
			while (!(SPSR & _BV(SPIF)));    // wait for last send to finish and retreive answer. Retreive must be done, otherwise the SPI will not work.
			SPDR = sendbyte; // Send the byte to the SPI
		}
		while (!(SPSR & _BV(SPIF))); // wait for last send to complete.
		bitSet(*columnLatchPort, columnLatchBit);
				
		ShiftMatrixPWM.m_counter++; // Increase the counter
		return;
	}
	else{
		// before going to next row, set column outputs off
		bitClear(*columnLatchPort, columnLatchBit);
		for(unsigned char i =ShiftMatrixPWM.m_amountOfColumnRegisters; i>0;--i){
			if(ShiftMatrixPWM_invertColumnOutputs){	
				SPDR = 0xFF;
			}
			else{
				SPDR = 0x00;
			}
			while (!(SPSR & _BV(SPIF))); // wait for last send to complete.
		}
		bitSet(*columnLatchPort, columnLatchBit);
		
		// Write column shift registers latch clock low 
		bitClear(*rowLatchPort, rowLatchBit);
		if(ShiftMatrixPWM.m_currentRow>=(ShiftMatrixPWM.m_amountOfRows-1)){
			//Back to row 1, give the shift register a new databit
			if(ShiftMatrixPWM_invertRowOutputs){
				bitClear(*rowDataPort, rowDataBit); //write first bit again if all rows completed last interrupt;				
			}
			else{
				bitSet(*rowDataPort, rowDataBit); 
			}
			bitClear(*rowClockPort, rowClockBit); //clock pulse to shift all bits
			bitSet(*rowClockPort, rowClockBit);
			
			//Set databit back to OFF
			if(ShiftMatrixPWM_invertRowOutputs){
				bitSet(*rowDataPort, rowDataBit); //write first bit again if all rows completed last interrupt;
			}
			else{
				bitClear(*rowDataPort, rowDataBit); 
			}
			ShiftMatrixPWM.m_currentRow=0;
		}
		else{
			
			bitClear(*rowClockPort, rowClockBit); //clock pulse to shift all bits
			bitSet(*rowClockPort, rowClockBit);
			ShiftMatrixPWM.m_currentRow++;
		}
	
		bitSet(*rowLatchPort, rowLatchBit); //enable new column
		
		ShiftMatrixPWM.m_counter = 0;		
	}	
} 

// See table  11-1 for the interrupt vectors */
#ifndef _useTimer1 
//Install the Interrupt Service Routine (ISR) for Timer1 compare and match A.
ISR(TIMER1_COMPA_vect) {
	ShiftMatrixPWM_handleInterrupt();
}
#else
//Install the Interrupt Service Routine (ISR) for Timer2 compare and match A.
ISR(TIMER2_COMPA_vect) {
	ShiftMatrixPWM_handleInterrupt();
}
#endif

// #endif for include once.
#endif 