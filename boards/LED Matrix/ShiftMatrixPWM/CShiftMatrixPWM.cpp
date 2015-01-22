/*
ShiftPWM.h - Library for Arduino to PWM many outputs using shift registers - Version 1
Copyright (c) 2011 Elco Jacobs, Technical University of Eindhoven, department of 
Industrial Design, Electronics Atelier.
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


---> See ShiftPWM.h for more info
*/

#include "CShiftMatrixPWM.h"
#include <WProgram.h>

extern const bool ShiftPWM_invertOutputs;

CShiftMatrixPWM::CShiftMatrixPWM(int timerInUse) : m_timer(timerInUse){ //Timer is set in initializer list, because it is const
	m_ledFrequency = 0;
	m_maxBrightness = 0;
	m_amountOfColumnRegisters = 0;    
	m_amountOfColumns=0;
	m_amountOfRows=0;
	m_amountOfOutputs=0;
	m_counter=0;
	m_currentRow=0;
	m_PWMValues=0;
}

CShiftMatrixPWM::~CShiftMatrixPWM() {
	if(m_PWMValues!=0){
		free( m_PWMValues );
	}
}

bool CShiftMatrixPWM::IsValidPin(int row, int col){
	if(row>=m_amountOfRows){
		Serial.print("Error: Trying to write duty cycle of row ");
		Serial.print(row);
		Serial.print(" , while number of rows is ");
		Serial.print(m_amountOfRows,DEC); 
		Serial.print(" , numbered 0-");
		Serial.println(m_amountOfRows-1, DEC); 
		delay(1000);
		return 0;	
	}
	if(col >= m_amountOfColumns){
		Serial.print("Error: Trying to write duty cycle of column ");
		Serial.print(col);
		Serial.print(" , while number of columns is ");
		Serial.print(m_amountOfColumns); 
		Serial.print(" , numbered 0-");
		Serial.println(m_amountOfColumns-1); 
		delay(1000);
		return 0;	
	}
	return 1;
}


void CShiftMatrixPWM::SetOne(int row, int col, unsigned char value){
	if(IsValidPin(row,col) ){
		m_PWMValues[row*m_amountOfColumns+col]=value;
	}
}

void CShiftMatrixPWM::SetAll(unsigned char value){
	for(int k=0 ; k<(m_amountOfOutputs);k++){
		m_PWMValues[k]=value;
	}   
}

void CShiftMatrixPWM::SetGroupOf2(int row, int group, unsigned char v0,unsigned char v1){
	if(IsValidPin(row, group*2+1) ){
		m_PWMValues[row*m_amountOfColumns+group*2]=v0;
		m_PWMValues[row*m_amountOfColumns+group*2+1]=v1;
	}
}

void CShiftMatrixPWM::SetGroupOf3(int row, int group, unsigned char v0,unsigned char v1,unsigned char v2){
	if(IsValidPin(row,group*3+2) ){
		m_PWMValues[row*m_amountOfColumns+group*3]=v0;
		m_PWMValues[row*m_amountOfColumns+group*3+1]=v1;
		m_PWMValues[row*m_amountOfColumns+group*3+2]=v2;
	}
}

void CShiftMatrixPWM::SetGroupOf4(int row, int group, unsigned char v0,unsigned char v1,unsigned char v2,unsigned char v3){
	if(IsValidPin(row,group*4+3) ){
		m_PWMValues[row*m_amountOfColumns+group*4]=v0;
		m_PWMValues[row*m_amountOfColumns+group*4+1]=v1;
		m_PWMValues[row*m_amountOfColumns+group*4+2]=v2;
		m_PWMValues[row*m_amountOfColumns+group*4+3]=v3;
	}
}

void CShiftMatrixPWM::SetGroupOf5(int row, int group, unsigned char v0,unsigned char v1,unsigned char v2,unsigned char v3,unsigned char v4){
	if(IsValidPin(row,group*5+4) ){
		m_PWMValues[row*m_amountOfColumns+group*5]=v0;
		m_PWMValues[row*m_amountOfColumns+group*5+1]=v1;
		m_PWMValues[row*m_amountOfColumns+group*5+2]=v2;
		m_PWMValues[row*m_amountOfColumns+group*5+3]=v3;
		m_PWMValues[row*m_amountOfColumns+group*5+4]=v4;
	}
}

// OneByOne functions are usefull for testing all your outputs
void CShiftMatrixPWM::OneByOneSlow(void){
	OneByOne_core(1024/m_maxBrightness);
}

void CShiftMatrixPWM::OneByOneFast(void){
	OneByOne_core(1);
}

void CShiftMatrixPWM::OneByOne_core(int delaytime){
	int pin,brightness;
	SetAll(0);
	for(int row=0; row<m_amountOfRows; row++){
		for(int col=0;col<m_amountOfColumns;col++){
			for(brightness=0;brightness<m_maxBrightness;brightness++){
				m_PWMValues[row*m_amountOfColumns+col]=brightness;
				delay(delaytime);          
			} 
			for(brightness=m_maxBrightness;brightness>=0;brightness--){
				m_PWMValues[row*m_amountOfColumns+col]=brightness;
				delay(delaytime);
			} 
		}
	}
}

void CShiftMatrixPWM::SetMatrixSize(int newAmountOfRows, int newAmountOfColumnRegisters){
	cli(); // Disable interrupt
	unsigned char oldAmountofColumnRegisters = m_amountOfColumnRegisters;
	unsigned char oldAmountOfRows= m_amountOfRows;

	m_amountOfColumnRegisters = newAmountOfColumnRegisters;
	m_amountOfRows = newAmountOfRows;
	
	m_amountOfColumns = m_amountOfColumnRegisters*8;
	m_amountOfOutputs = m_amountOfRows*m_amountOfColumns;

	if(LoadNotTooHigh() ){ //Check if new amount will not result in deadlock
		m_PWMValues = (unsigned char *) realloc(m_PWMValues, m_amountOfOutputs); //resize array for PWMValues
		for(int r=0; r<m_amountOfRows;r++){
			for(int c=0; c<m_amountOfColumns;c++ ){
				m_PWMValues[r*m_amountOfColumns+c]=0; //set all values to zero
			}
		}
		sei(); //Re-enable interrupt
	}
	else{
		// New value would result in deadlock, keep old values and print an error message
		m_amountOfColumnRegisters = oldAmountofColumnRegisters;
		m_amountOfRows = oldAmountOfRows;
		m_amountOfColumns = m_amountOfColumnRegisters*8;
		m_amountOfOutputs = m_amountOfRows*m_amountOfColumns;
		Serial.println("Amount of registers is not increased, because load would become too high");
		sei();
	}
}

bool CShiftMatrixPWM::LoadNotTooHigh(void){
	// This function calculates if the interrupt load would become higher than 0.9 and prints an error if it would.
	// This is with inverted outputs, which is worst case. Without inverting, it would be 42 per register.
	float interruptDuration = 157+43* (float) m_amountOfColumnRegisters;	
	float interruptFrequency = (float) m_ledFrequency* (float) m_maxBrightness* (float) m_amountOfRows;
	float load = interruptDuration*interruptFrequency/F_CPU;

	if(load > 0.9){
		Serial.print("New interrupt duration ="); Serial.print(interruptDuration); Serial.println("clock cycles");
		Serial.print("New interrupt frequency ="); Serial.print(interruptFrequency); Serial.println("Hz");
		Serial.print("New interrupt load would be ");
		Serial.print(load);
		Serial.println(" , which is too high.");
		return 0;
	}
	else{
		return 1;
	}

}

void CShiftMatrixPWM::Start(int ledFrequency, unsigned char maxBrightness){
	// Configure and enable timer1 or timer 2 for a compare and match A interrupt.    

	m_ledFrequency = ledFrequency;
	m_maxBrightness = maxBrightness;

	if(LoadNotTooHigh() ){
		if(m_timer==1){ 
			InitTimer1();
		}
		else if(m_timer==2){
			InitTimer2();
		}
	}
	else{
		Serial.println("Interrupts are disabled because load is too high.");
		cli(); //Disable interrupts
	}
}

void CShiftMatrixPWM::InitTimer1(void){
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
	OCR1A = round((float) F_CPU/((float) m_amountOfRows*(float) m_ledFrequency*((float) m_maxBrightness+1)))-1;
	/* Finally enable the timer interrupt 
	/* See datasheet  15.11.8) */
	bitSet(TIMSK1,OCIE1A);
}

void CShiftMatrixPWM::InitTimer2(void){
	/* Configure timer2 in CTC mode: clear the timer on compare match 
	* See the Atmega328 Datasheet 15.9.2 for an explanation on CTC mode.
	* See table 17-8 in the datasheet. */

	bitClear(TCCR2B,WGM22);
	bitSet(TCCR2A,WGM21);
	bitClear(TCCR2A,WGM20);

	/*  Select clock source: internal I/O clock, calculate most suitable prescaler
	*  This is only an 8 bit timer, so choose the prescaler so that OCR2A fits in 8 bits.
	*  See table 15-5 in the datasheet. */
	int compare_value =  round((float) F_CPU/((float) m_amountOfRows*(float) m_ledFrequency*((float) m_maxBrightness+1))-1);
	if(compare_value <= 255){
		m_prescaler = 1;
		bitClear(TCCR2B,CS22); bitClear(TCCR2B,CS21); bitClear(TCCR2B,CS20);
	}
	else if(compare_value/8 <=255){
		m_prescaler = 8;
		bitClear(TCCR2B,CS22); bitSet(TCCR2B,CS21); bitClear(TCCR2B,CS20);
	}
	else if(compare_value/32 <=255){
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
		OCR2A = round(   (  (float) F_CPU / (float) m_prescaler ) /  ((float) m_amountOfRows* (float) m_ledFrequency*( (float) m_maxBrightness+1) ) -1);
		/* Finally enable the timer interrupt 
		/* See datasheet  15.11.8) */
		bitSet(TIMSK2,OCIE2A);
}


void CShiftMatrixPWM::PrintInterruptLoad(void){
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
			Serial.println("Interrupt is disabled.");
			return;
		}
	}
	else if(m_timer==2){
		if(TIMSK2 & (1<<OCIE2A)){
			// interrupt is enabled, continue
		}
		else{
			// interrupt is disabled
			Serial.println("Interrupt is disabled.");
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
	Serial.print("Load of interrupt: ");   Serial.println(load,10); 
	Serial.print("Clock cycles per interrupt: ");   Serial.println(cycles_per_int); 
	Serial.print("Interrupt frequency: "); Serial.print(interrupt_frequency);   Serial.println(" Hz");
	Serial.print("PWM frequency: "); Serial.print(interrupt_frequency/(m_maxBrightness+1)); Serial.println(" Hz");
	Serial.print("Divided over "); Serial.print(m_amountOfRows, DEC); Serial.print(" rows, to have a total refresh rate of "); 
	Serial.print(interrupt_frequency/(m_amountOfRows*(m_maxBrightness+1))); Serial.println(" Hz");

	if(m_timer==1){   
		Serial.println("Timer1 in use for highest precision."); 
		Serial.println("Include servo.h to use timer2.");
		Serial.print("OCR1A: "); Serial.println(OCR1A, DEC);
		Serial.print("Prescaler: "); Serial.println(m_prescaler);

		//Re-enable Interrupt	
		bitSet(TIMSK1,OCIE1A); 
	}
	else if(m_timer==2){
		Serial.println("Timer2 in use, because Timer1 is used by servo library.");
		Serial.print("OCR2A: "); Serial.println(OCR2A, DEC);
		Serial.print("Presclaler: "); Serial.println(m_prescaler);  

		//Re-enable Interrupt	
		bitSet(TIMSK2,OCIE2A); 
	}
}

