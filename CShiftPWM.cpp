/*
CShiftPWM.cpp - ShiftPWM.h - Library for Arduino to PWM many outputs using shift registers
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

#include "CShiftPWM.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

CShiftPWM::CShiftPWM(int timerInUse, bool noSPI, int latchPin, int dataPin, int clockPin) :  // Constants are set in initializer list
					m_timer(timerInUse), m_noSPI(noSPI), m_latchPin(latchPin), m_dataPin(dataPin), m_clockPin(clockPin){
	m_ledFrequency = 0;
	m_maxBrightness = 0;
	m_amountOfRegisters = 0;    
	m_amountOfOutputs = 0;
	m_counter = 0;
	m_pinGrouping = 1; // Default = RGBRGBRGB... PinGrouping = 3 means: RRRGGGBBBRRRGGGBBB...

	unsigned char * m_PWMValues=0;
}

CShiftPWM::~CShiftPWM() {
	if(m_PWMValues>0){
		free( m_PWMValues );
	}
}

bool CShiftPWM::IsValidPin(int pin){
	if(pin<m_amountOfOutputs){
		return 1;
	}
	else{
		Serial.print(F("Error: Trying to write duty cycle of pin "));
		Serial.print(pin); 		
		Serial.print(F(" , while number of outputs is "));
		Serial.print(m_amountOfOutputs); 
		Serial.print(F(" , numbered 0-"));
		Serial.println(m_amountOfOutputs-1); 
		delay(1000);
		return 0;
	}
}


void CShiftPWM::SetOne(int pin, unsigned char value){
	if(IsValidPin(pin) ){
		m_PWMValues[pin]=value;
	}
}

void CShiftPWM::SetAll(unsigned char value){
	for(int k=0 ; k<(m_amountOfOutputs);k++){
		m_PWMValues[k]=value;
	}   
}

void CShiftPWM::SetGroupOf2(int group, unsigned char v0,unsigned char v1, int offset){
	int skip = m_pinGrouping*(group/m_pinGrouping); // is not equal to 2*group. Division is rounded down first.
	if(IsValidPin(group+skip+offset+m_pinGrouping) ){
		m_PWMValues[group+skip+offset]					=v0;
		m_PWMValues[group+skip+offset+m_pinGrouping]	=v1;
	}
}

void CShiftPWM::SetGroupOf3(int group, unsigned char v0,unsigned char v1,unsigned char v2, int offset){
	int skip = 2*m_pinGrouping*(group/m_pinGrouping); // is not equal to 2*group. Division is rounded down first.
	if(IsValidPin(group+skip+offset+2*m_pinGrouping) ){
		m_PWMValues[group+skip+offset]					=v0;
		m_PWMValues[group+skip+offset+m_pinGrouping]	=v1;
		m_PWMValues[group+skip+offset+m_pinGrouping*2]	=v2;
	}
}

void CShiftPWM::SetGroupOf4(int group, unsigned char v0,unsigned char v1,unsigned char v2,unsigned char v3, int offset){
	int skip = 3*m_pinGrouping*(group/m_pinGrouping); // is not equal to 2*group. Division is rounded down first.
	if(IsValidPin(group+skip+offset+3*m_pinGrouping) ){
		m_PWMValues[group+skip+offset]					=v0;
		m_PWMValues[group+skip+offset+m_pinGrouping]	=v1;
		m_PWMValues[group+skip+offset+m_pinGrouping*2]	=v2;
		m_PWMValues[group+skip+offset+m_pinGrouping*3]	=v3;
	}
}

void CShiftPWM::SetGroupOf5(int group, unsigned char v0,unsigned char v1,unsigned char v2,unsigned char v3,unsigned char v4, int offset){
	int skip = 4*m_pinGrouping*(group/m_pinGrouping); // is not equal to 2*group. Division is rounded down first.
	if(IsValidPin(group+skip+offset+4*m_pinGrouping) ){
		m_PWMValues[group+skip+offset]					=v0;
		m_PWMValues[group+skip+offset+m_pinGrouping]	=v1;
		m_PWMValues[group+skip+offset+m_pinGrouping*2]	=v2;
		m_PWMValues[group+skip+offset+m_pinGrouping*3]	=v3;
		m_PWMValues[group+skip+offset+m_pinGrouping*4]	=v4;
	}
}

void CShiftPWM::SetRGB(int led, unsigned char r,unsigned char g,unsigned char b, int offset){
	int skip = 2*m_pinGrouping*(led/m_pinGrouping); // is not equal to 2*led. Division is rounded down first.
	if(IsValidPin(led+skip+offset+2*m_pinGrouping) ){
		m_PWMValues[led+skip+offset]					=( (unsigned int) r * m_maxBrightness)>>8;
		m_PWMValues[led+skip+offset+m_pinGrouping]		=( (unsigned int) g * m_maxBrightness)>>8;
		m_PWMValues[led+skip+offset+2*m_pinGrouping]	=( (unsigned int) b * m_maxBrightness)>>8;
	}
}

void CShiftPWM::SetAllRGB(unsigned char r,unsigned char g,unsigned char b){
	for(int k=0 ; (k+3*m_pinGrouping-1) < m_amountOfOutputs; k+=3*m_pinGrouping){
		for(int l=0; l<m_pinGrouping;l++){
			m_PWMValues[k+l]				=	( (unsigned int) r * m_maxBrightness)>>8;
			m_PWMValues[k+l+m_pinGrouping]	=	( (unsigned int) g * m_maxBrightness)>>8;
			m_PWMValues[k+l+m_pinGrouping*2]	=	( (unsigned int) b * m_maxBrightness)>>8;
		}
	}
}

void CShiftPWM::SetHSV(int led, unsigned int hue, unsigned int sat, unsigned int val, int offset){
	unsigned char r,g,b;
	unsigned int H_accent = hue/60;
	unsigned int bottom = ((255 - sat) * val)>>8;
	unsigned int top = val;
	unsigned char rising  = ((top-bottom)  *(hue%60   )  )  /  60  +  bottom;
	unsigned char falling = ((top-bottom)  *(60-hue%60)  )  /  60  +  bottom;

	switch(H_accent) {
	case 0:
		r = top;
		g = rising;
		b = bottom;
		break;

	case 1:
		r = falling;
		g = top;
		b = bottom;
		break;

	case 2:
		r = bottom;
		g = top;
		b = rising;
		break;

	case 3:
		r = bottom;
		g = falling;
		b = top;
		break;

	case 4:
		r = rising;
		g = bottom;
		b = top;
		break;

	case 5:
		r = top;
		g = bottom;
		b = falling;
		break;
	}
	SetRGB(led,r,g,b,offset);
}

void CShiftPWM::SetAllHSV(unsigned int hue, unsigned int sat, unsigned int val){
	// Set the first LED
	SetHSV(0, hue, sat, val);
	// Copy RGB values all LED's.
	SetAllRGB(m_PWMValues[0],m_PWMValues[m_pinGrouping],m_PWMValues[2*m_pinGrouping]);
}

// OneByOne functions are usefull for testing all your outputs
void CShiftPWM::OneByOneSlow(void){
	OneByOne_core(1024/m_maxBrightness);
}

void CShiftPWM::OneByOneFast(void){
	OneByOne_core(1);
}

void CShiftPWM::OneByOne_core(int delaytime){
	int pin,brightness;
	SetAll(0);
	for(int pin=0;pin<m_amountOfOutputs;pin++){
		for(brightness=0;brightness<m_maxBrightness;brightness++){
			m_PWMValues[pin]=brightness;
			delay(delaytime);          
		} 
		for(brightness=m_maxBrightness;brightness>=0;brightness--){
			m_PWMValues[pin]=brightness;
			delay(delaytime);
		} 
	}
}

void CShiftPWM::SetAmountOfRegisters(unsigned char newAmount){
	cli(); // Disable interrupt
	unsigned char oldAmount = m_amountOfRegisters;
	m_amountOfRegisters = newAmount;
	m_amountOfOutputs=m_amountOfRegisters*8;

	if(LoadNotTooHigh() ){ //Check if new amount will not result in deadlock
		m_PWMValues = (unsigned char *) realloc(m_PWMValues, newAmount*8); //resize array for PWMValues

		for(int k=oldAmount; k<(newAmount*8);k++){
			m_PWMValues[k]=0; //set new values to zero
		}
		sei(); //Re-enable interrupt
	}
	else{
		// New value would result in deadlock, keep old values and print an error message
		m_amountOfRegisters = oldAmount;
		m_amountOfOutputs=m_amountOfRegisters*8;
		Serial.println(F("Amount of registers is not increased, because load would become too high"));
		sei();
	}
}

void CShiftPWM::SetPinGrouping(int grouping){
	// Sets the number of pins per color that are used after eachother. RRRRGGGGBBBBRRRRGGGGBBBB would be a grouping of 4.
	m_pinGrouping = grouping;
}

bool CShiftPWM::LoadNotTooHigh(void){
	// This function calculates if the interrupt load would become higher than 0.9 and prints an error if it would.
	// This is with inverted outputs, which is worst case. Without inverting, it would be 42 per register.
	float interruptDuration;
	if(m_noSPI){
		interruptDuration = 96+108*(float) m_amountOfRegisters;
	}
	else{
		interruptDuration = 97+43* (float) m_amountOfRegisters;
	}
	float interruptFrequency = (float) m_ledFrequency* ((float) m_maxBrightness + 1);
	float load = interruptDuration*interruptFrequency/F_CPU;

	if(load > 0.9){
		Serial.print(F("New interrupt duration =")); Serial.print(interruptDuration); Serial.println(F("clock cycles"));
		Serial.print(F("New interrupt frequency =")); Serial.print(interruptFrequency); Serial.println(F("Hz"));
		Serial.print(F("New interrupt load would be "));
		Serial.print(load);
		Serial.println(F(" , which is too high."));
		return 0;
	}
	else{
		return 1;
	}

}

void CShiftPWM::Start(int ledFrequency, unsigned char maxBrightness){
	// Configure and enable timer1 or timer 2 for a compare and match A interrupt.    
	m_ledFrequency = ledFrequency;
	m_maxBrightness = maxBrightness;
	
	pinMode(m_dataPin, OUTPUT);
	pinMode(m_clockPin, OUTPUT);
	pinMode(m_latchPin, OUTPUT);
	
	digitalWrite(m_clockPin, LOW);
	digitalWrite(m_dataPin, LOW);
  	
	if(!m_noSPI){ // initialize SPI when used
		// The least significant bit shoult be sent out by the SPI port first.
		// equals SPI.setBitOrder(LSBFIRST);
		SPCR |= _BV(DORD);
		
		// Here you can set the clock speed of the SPI port. Default is DIV4, which is 4MHz with a 16Mhz system clock.
		// If you encounter problems due to long wires or capacitive loads, try lowering the SPI clock.
		// equals SPI.setClockDivider(SPI_CLOCK_DIV4); 
		
		SPCR = (SPCR & 0b11111000);
		SPSR = (SPSR & 0b11111110);
		
		// Set clock polarity and phase for shift registers (Mode 3)
		SPCR |= _BV(CPOL);
		SPCR |= _BV(CPHA);
		
		// When the SS pin is set as OUTPUT, it can be used as
		// a general purpose output port (it doesn't influence
		// SPI operations).
		pinMode(SS, OUTPUT);
		digitalWrite(SS, HIGH);

		// Warning: if the SS pin ever becomes a LOW INPUT then SPI 
		// automatically switches to Slave, so the data direction of 
		// the SS pin MUST be kept as OUTPUT.
		SPCR |= _BV(MSTR);
		SPCR |= _BV(SPE);
	}
	
	if(LoadNotTooHigh() ){
		if(m_timer==1){ 
			InitTimer1();
		}
		else if(m_timer==2){
			InitTimer2();
		}
	}
	else{
		Serial.println(F("Interrupts are disabled because load is too high."));
		cli(); //Disable interrupts
	}
}

void CShiftPWM::InitTimer1(void){
	/* Configure timer1 in CTC mode: clear the timer on compare match 
	* See the Atmega328 Datasheet 15.9.2 for an explanation on CTC mode.
	* See table 15-4 in the datasheet. */

	bitSet(TCCR1B,WGM12);
	bitClear(TCCR1B,WGM13);
	bitClear(TCCR1A,WGM11);
	bitClear(TCCR1A,WGM10);


	/*  Select clock source: internal I/O clock, without a prescaler
	*  This is the fastest possible clock source for the highest accuracy.
	*  See table 15-5 in the datasheet. */

	bitSet(TCCR1B,CS10);
	bitClear(TCCR1B,CS11);
	bitClear(TCCR1B,CS12);

	/* The timer will generate an interrupt when the value we load in OCR1A matches the timer value.
	* One period of the timer, from 0 to OCR1A will therefore be (OCR1A+1)/(timer clock frequency).
	* We want the frequency of the timer to be (LED frequency)*(number of brightness levels)
	* So the value we want for OCR1A is: timer clock frequency/(LED frequency * number of bightness levels)-1 */
	m_prescaler = 1;
	OCR1A = round((float) F_CPU/((float) m_ledFrequency*((float) m_maxBrightness+1)))-1;
	/* Finally enable the timer interrupt 
	/* See datasheet  15.11.8) */
	bitSet(TIMSK1,OCIE1A);
}

void CShiftPWM::InitTimer2(void){
	/* Configure timer2 in CTC mode: clear the timer on compare match 
	* See the Atmega328 Datasheet 15.9.2 for an explanation on CTC mode.
	* See table 17-8 in the datasheet. */

	bitClear(TCCR2B,WGM22);
	bitSet(TCCR2A,WGM21);
	bitClear(TCCR2A,WGM20);

	/*  Select clock source: internal I/O clock, calculate most suitable prescaler
	*  This is only an 8 bit timer, so choose the prescaler so that OCR2A fits in 8 bits.
	*  See table 15-5 in the datasheet. */
	int compare_value =  round((float) F_CPU/((float) m_ledFrequency*((float) m_maxBrightness+1))-1);
	if(compare_value <= 255){
		m_prescaler = 1;
		bitClear(TCCR2B,CS22); bitClear(TCCR2B,CS21); bitClear(TCCR2B,CS20);
	}
	else if(compare_value/8 <=255){
		m_prescaler = 8;
		bitClear(TCCR2B,CS22); bitSet(TCCR2B,CS21); bitClear(TCCR2B,CS20);
	}
	else 
		if(compare_value/32 <=255){
			m_prescaler = 32;
			bitClear(TCCR2B,CS22); bitSet(TCCR2B,CS21); bitSet(TCCR2B,CS20);
		}
		else if(compare_value/64 <= 255){
			m_prescaler = 64;
			bitSet(TCCR2B,CS22); bitClear(TCCR2B,CS21); bitClear(TCCR2B,CS20);
		}
		else if(compare_value/128 <= 255){
			m_prescaler = 128;
			bitSet(TCCR2B,CS22); bitClear(TCCR2B,CS21); bitSet(TCCR2B,CS20);
		}
		else if(compare_value/256 <= 255){
			m_prescaler = 256;
			bitSet(TCCR2B,CS22); bitSet(TCCR2B,CS21); bitClear(TCCR2B,CS20);
		}

		/* The timer will generate an interrupt when the value we load in OCR2A matches the timer value.
		* One period of the timer, from 0 to OCR2A will therefore be (OCR2A+1)/(timer clock frequency).
		* We want the frequency of the timer to be (LED frequency)*(number of brightness levels)
		* So the value we want for OCR2A is: timer clock frequency/(LED frequency * number of bightness levels)-1 */
		OCR2A = round(   (  (float) F_CPU / (float) m_prescaler ) /  ( (float) m_ledFrequency*( (float) m_maxBrightness+1) ) -1);
		/* Finally enable the timer interrupt 
		/* See datasheet  15.11.8) */
		bitSet(TIMSK2,OCIE2A);
}


void CShiftPWM::PrintInterruptLoad(void){
	//This function prints information on the interrupt settings for ShiftPWM
	//It runs a delay loop 2 times: once with interrupts enabled, once disabled.
	//From the difference in duration, it can calculate the load of the interrupt on the program.

	unsigned long start1,end1,time1,start2,end2,time2,k;
	double load, cycles_per_int, interrupt_frequency;


	if(m_timer==1){
		if(TIMSK1 & (1<<OCIE1A)){
			// interrupt is enabled, continue
		}
		else{
			// interrupt is disabled
			Serial.println(F("Interrupt is disabled."));
			return;
		}
	}
	else if(m_timer==2){
		if(TIMSK2 & (1<<OCIE2A)){
			// interrupt is enabled, continue
		}
		else{
			// interrupt is disabled
			Serial.println(F("Interrupt is disabled."));
			return;
		}
	}

	//run with interrupt enabled
	start1 = micros();
	for(k=0; k<100000; k++){
		delayMicroseconds(1); 
	}
	end1 = micros();  
	time1 = end1-start1; 

	//Disable Interrupt
	if(m_timer==1){ 
		bitClear(TIMSK1,OCIE1A);
	}
	else if(m_timer==2){
		bitClear(TIMSK2,OCIE2A);
	}


	// run with interrupt disabled
	start2 = micros();
	for(k=0; k<100000; k++){
		delayMicroseconds(1); 
	}
	end2 = micros();
	time2 = end2-start2;

	// ready for calculations
	load = (double)(time1-time2)/(double)(time1);
	if(m_timer==1){   
		interrupt_frequency = (F_CPU/m_prescaler)/(OCR1A+1);
	}
	else if(m_timer==2){
		interrupt_frequency = (F_CPU/m_prescaler)/(OCR2A+1);  
	}
	cycles_per_int = load*(F_CPU/interrupt_frequency);

	//Ready to print information
	Serial.print(F("Load of interrupt: "));   Serial.println(load,10); 
	Serial.print(F("Clock cycles per interrupt: "));   Serial.println(cycles_per_int); 
	Serial.print(F("Interrupt frequency: ")); Serial.print(interrupt_frequency);   Serial.println(F(" Hz"));
	Serial.print(F("PWM frequency: ")); Serial.print(interrupt_frequency/(m_maxBrightness+1)); Serial.println(F(" Hz"));

	if(m_timer==1){   
		Serial.println(F("Timer1 in use for highest precision.")); 
		Serial.println(F("Include servo.h to use timer2."));
		Serial.print(F("OCR1A: ")); Serial.println(OCR1A, DEC);
		Serial.print(F("Prescaler: ")); Serial.println(m_prescaler);

		//Re-enable Interrupt	
		bitSet(TIMSK1,OCIE1A); 
	}
	else if(m_timer==2){
		Serial.println(F("Timer2 in use, because Timer1 is used by servo library."));
		Serial.print(F("OCR2A: ")); Serial.println(OCR2A, DEC);
		Serial.print(F("Presclaler: ")); Serial.println(m_prescaler);  

		//Re-enable Interrupt	
		bitSet(TIMSK2,OCIE2A); 
	}
}
