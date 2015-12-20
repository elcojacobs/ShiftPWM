/*
ShiftPWM.h - Library for Arduino to PWM many outputs using shift registers
Copyright (c) 2011-2012 Elco Jacobs, www.elcojacobs.com
All right reserved.

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


#ifndef ShiftPWM_H
#define ShiftPWM_H

#include "pins_arduino_compile_time.h" // My own version of pins arduino, which does not define the arrays in program memory
#include <Arduino.h>
#include "CShiftPWM.h"


// These should be defined in the file where ShiftPWM.h is included.
extern const int ShiftPWM_latchPin;
extern const bool ShiftPWM_invertOutputs;
extern const bool ShiftPWM_balanceLoad;

// The ShiftPWM object is created in the header file, instead of defining it as extern here and creating it in the cpp file.
// If the ShiftPWM object is created in the cpp file, it is separately compiled with the library.
// The compiler cannot treat it as constant and cannot optimize well: it will generate many memory accesses in the interrupt function.

#if defined(SHIFTPWM_USE_TIMER2)
	#if !defined(OCR2A)
		#error "The avr you are using does not have a timer2"
	#endif
#elif defined(SHIFTPWM_USE_TIMER3)
	#if !defined(OCR3A)
		#error "The avr you are using does not have a timer3"
	#endif
#endif


#ifndef SHIFTPWM_NOSPI
	// Use SPI
	#if defined(SHIFTPWM_USE_TIMER3)
		CShiftPWM ShiftPWM(3,false,ShiftPWM_latchPin,MOSI,SCK);
	#elif defined(SHIFTPWM_USE_TIMER2)
		CShiftPWM ShiftPWM(2,false,ShiftPWM_latchPin,MOSI,SCK);
	#else
		CShiftPWM ShiftPWM(1,false,ShiftPWM_latchPin,MOSI,SCK);
	#endif
#else
	// Don't use SPI
	extern const int ShiftPWM_clockPin;
	extern const int ShiftPWM_dataPin;
	#if defined(SHIFTPWM_USE_TIMER3)
		CShiftPWM ShiftPWM(3,true,ShiftPWM_latchPin,ShiftPWM_dataPin,ShiftPWM_clockPin);
	#elif defined(SHIFTPWM_USE_TIMER2)
		CShiftPWM ShiftPWM(2,true,ShiftPWM_latchPin,ShiftPWM_dataPin,ShiftPWM_clockPin);
	#else
		CShiftPWM ShiftPWM(1,true,ShiftPWM_latchPin,ShiftPWM_dataPin,ShiftPWM_clockPin);
	#endif
#endif

// The macro below uses 3 instructions per pin to generate the byte to transfer with SPI
// Retrieve duty cycle setting from memory (ldd, 2 clockcycles)
// Compare with the counter (cp, 1 clockcycle) --> result is stored in carry
// Use the rotate over carry right to shift the compare result into the byte. (1 clockcycle).

// len is the number of 8-bit bytes to send out the SPI
// buf points to the last byte of a buffer of values to compare and send. The last byte in the buffer is sent first.
// counter is threshold. Each byte in the buff is compared to count and if it is greater, then a 1 bit is sent.

static __attribute__ ((always_inline)) void send_spi_bytes( unsigned len, unsigned char counter, const unsigned char *buf, const unsigned char imask, const unsigned char step ) 
{ 
    register unsigned char sendchar;

    asm volatile (

        "LOOP_LEN_%=: \n\t"
        
        // Shift PWM Bit #0
        "ld __tmp_reg__, -%a[buf]    \n\t"
        "cp %[counter], __tmp_reg__  \n\t"
        "ror %[sendchar]             \r\n"
                
        // Shift PWM Bit #1
        "ld __tmp_reg__, -%a[buf]    \n\t"
        "cp %[counter], __tmp_reg__  \n\t"
        "ror %[sendchar]             \r\n"
        
        // Shift PWM Bit #2
        "ld __tmp_reg__, -%a[buf]    \n\t"
        "cp %[counter], __tmp_reg__  \n\t"
        "ror %[sendchar]             \r\n"
        
        // Shift PWM Bit #3
        "ld __tmp_reg__, -%a[buf]    \n\t"
        "cp %[counter], __tmp_reg__  \n\t"
        "ror %[sendchar]             \r\n"
        
        // Shift PWM Bit #4
        "ld __tmp_reg__, -%a[buf]    \n\t"
        "cp %[counter], __tmp_reg__  \n\t"
        "ror %[sendchar]             \r\n"
        
        // Shift PWM Bit #5
        "ld __tmp_reg__, -%a[buf]    \n\t"
        "cp %[counter], __tmp_reg__  \n\t"
        "ror %[sendchar]             \r\n"
        
        // Shift PWM Bit #6
        "ld __tmp_reg__, -%a[buf]    \n\t"
        "cp %[counter], __tmp_reg__  \n\t"
        "ror %[sendchar]             \r\n"
        
        // Shift PWM Bit #7
        "ld __tmp_reg__, -%a[buf]    \n\t"
        "cp %[counter], __tmp_reg__  \n\t"
        "ror %[sendchar]             \n\t"

        // Potentially invert bits 
        "eor %[sendchar],%[imask]   \n\t"
        
        // Check that previous send completed
        "WAIT_SPI_%=:                \n\t"        
        "in	__tmp_reg__,%[spsr]      \n\t"
        "sbrs __tmp_reg__,%[spif]    \n\t"
        "rjmp WAIT_SPI_%=            \n\t"
                
        // Send new byte
        "out %[spdr], %[sendchar]    \n\t"

        // repeat until all shift registers are filled
        "sbiw %[len], 1              \n\t"
        "brne LOOP_LEN_%=            \n\t"

        : // Outputs: 
        [sendchar] "=&r" (sendchar) // working register

        : // Inputs:
        [buf] "e" (buf),         // pointer to buffer
        [len] "r" (len),         // length of buffer
        [counter] "r" (counter), // current threshold
        [imask] "r" (imask),     // XORed to final byte before sent to potentially invert bits
        [step] "r" (step),       // increment counter by this much for each byte sent for load balancing 
        
          // Constants
        [spdr] "I" (_SFR_IO_ADDR(SPDR)), // SPI data register
        [spsr] "I" (_SFR_IO_ADDR(SPSR)), // SPI status register
        [spif] "I" (SPIF)                // SPI interrupt flag (send complete) (should really be a constraint for 0-7 value)

        : // Clobbers
        "cc" // special name that indicates that flags may have been clobbered

    );

}
// The inline function below uses normal output pins to send one bit to the SPI port.
// This function is used in the noSPI mode and is useful if you need the SPI port for something else.
// It is a lot 2.5x slower than the SPI version.
static inline void pwm_output_one_pin(volatile uint8_t * const clockPort, volatile uint8_t * const dataPort,\
                                  const uint8_t clockBit, const uint8_t dataBit, \
                                  unsigned char counter, unsigned char * ledPtr){
    bitClear(*clockPort, clockBit);
    if(ShiftPWM_invertOutputs){
      bitWrite(*dataPort, dataBit, *(ledPtr)<=counter );
    }
    else{
      bitWrite(*dataPort, dataBit, *(ledPtr)>counter );
    }
    bitSet(*clockPort, clockBit);
}

static inline void ShiftPWM_handleInterrupt(void){
	sei(); //enable interrupt nesting to prevent disturbing other interrupt functions (servo's for example).

	// Look up which bit of which output register corresponds to the pin.
	// This should be constant, so the compiler can optimize this code away and use sbi and cbi instructions
	// The compiler only knows this if this function is compiled in the same file as the pin setting.
	// That is the reason the full function is in the header file, instead of only the prototype.
	// If this function is defined in cpp files of the library, it is compiled seperately from the main file.
	// The compiler does not recognize the pins/ports as constant and sbi and cbi instructions cannot be used.


	volatile uint8_t * const latchPort = port_to_output_PGM_ct[digital_pin_to_port_PGM_ct[ShiftPWM_latchPin]];
	const uint8_t latchBit =  digital_pin_to_bit_PGM_ct[ShiftPWM_latchPin];

	#ifdef SHIFTPWM_NOSPI
	volatile uint8_t * const clockPort = port_to_output_PGM_ct[digital_pin_to_port_PGM_ct[ShiftPWM_clockPin]];
	volatile uint8_t * const dataPort  = port_to_output_PGM_ct[digital_pin_to_port_PGM_ct[ShiftPWM_dataPin]];
	const uint8_t clockBit =  digital_pin_to_bit_PGM_ct[ShiftPWM_clockPin];
	const uint8_t dataBit =   digital_pin_to_bit_PGM_ct[ShiftPWM_dataPin];   
	#endif

	// Define a pointer that will be used to access the values for each output. 
	// Let it point one past the last value, because it is decreased before it is used.

	unsigned char * ledPtr=&ShiftPWM.m_PWMValues[ShiftPWM.m_amountOfOutputs];

	// Write shift register latch clock low 
	bitClear(*latchPort, latchBit);
	unsigned char counter = ShiftPWM.m_counter;
	
	#ifndef SHIFTPWM_NOSPI
	//Use SPI to send out all bits
    
    // write bogus byte to the SPI because the SPI hardware only signals "transmit complete" rather than "transmitter idle".
    // This byte will harmlessly be shifted off the edge of the final shift register
	SPDR = 0; 
    
    // If we are inverting outputs, then XORing the output byte with 0xff will flip all bits. XORing with 0x00 does nothing (except waste a cycle).
    // TODO: Invert the bits when they are put into the buffer so we only do it once rather than on every INT
    const unsigned char imask =  ShiftPWM_invertOutputs? 0xff: 0x00;
    
    // This value gets added to the counter after each byte is sent to avoid having all loads turning on and off at the same time.     
    // TODO: Is there any use case where you would *not* want to balance the output? If not, get rid of the option and calculate the best step value to most evenly distribute load over all outputs
    const unsigned char step =  ShiftPWM_balanceLoad ? 8 : 0;
    
    send_spi_bytes(ShiftPWM.m_amountOfRegisters,counter,ledPtr, imask, step);
    
	while (!(SPSR & _BV(SPIF))); // wait for last send to complete.
    
    
	#else
	//Use port manipulation to send out all bits
	for(unsigned char i = ShiftPWM.m_amountOfRegisters; i>0;--i){   // do one shift register at a time. This unrolls the loop for extra speed
		if(ShiftPWM_balanceLoad){
			counter +=8; // distribute the load by using a shifted counter per shift register
		}
		pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);  // This takes 12 or 13 clockcycles
		pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
		pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
		pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
		pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
		pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
		pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
		pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
	}
	#endif

	// Write shift register latch clock high
	bitSet(*latchPort, latchBit);
    
    // TODO: spin backwards to save one cycle on compare 
	if(ShiftPWM.m_counter<ShiftPWM.m_maxBrightness){
		ShiftPWM.m_counter++; // Increase the counter
	}
	else{
		ShiftPWM.m_counter=0; // Reset counter if it maximum brightness has been reached
	}
}

// See table  11-1 for the interrupt vectors */
#if defined(SHIFTPWM_USE_TIMER3)
	//Install the Interrupt Service Routine (ISR) for Timer3 compare and match A.
	ISR(TIMER3_COMPA_vect) {
		ShiftPWM_handleInterrupt();
	}
#elif defined(SHIFTPWM_USE_TIMER2)
	//Install the Interrupt Service Routine (ISR) for Timer1 compare and match A.
	ISR(TIMER2_COMPA_vect) {
		ShiftPWM_handleInterrupt();
	}
#else
	//Install the Interrupt Service Routine (ISR) for Timer1 compare and match A.
	ISR(TIMER1_COMPA_vect) {
		ShiftPWM_handleInterrupt();
	}
#endif

// #endif for include once.
#endif
