/*
CShiftPWM.h - Library for Arduino to PWM many outputs using shift registers
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

#ifndef CShiftPWM_h
#define CShiftPWM_h

class CShiftPWM{
public:
	CShiftPWM(int timerInUse, bool noSPI, int latchPin, int dataPin, int clockPin);
	~CShiftPWM();

public:
	void Start(int ledFrequency, unsigned char max_Brightness);
	void SetAmountOfRegisters(unsigned char newAmount);
	void SetPinGrouping(int grouping);
	void PrintInterruptLoad(void);
	void OneByOneSlow(void);
	void OneByOneFast(void);
	void SetOne(int pin, unsigned char value);
	void SetAll(unsigned char value);

	void SetGroupOf2(int group, unsigned char v0, unsigned char v1, int offset = 0);
	void SetGroupOf3(int group, unsigned char v0, unsigned char v1, unsigned char v2, int offset = 0);
	void SetGroupOf4(int group, unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3, int offset = 0);
	void SetGroupOf5(int group, unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3, unsigned char v4, int offset = 0);

	void SetRGB(int led, unsigned char r,unsigned char g,unsigned char b, int offset = 0);
	void SetAllRGB(unsigned char r,unsigned char g,unsigned char b);
	void SetHSV(int led, unsigned int hue, unsigned int sat, unsigned int val, int offset = 0);
	void SetAllHSV(unsigned int hue, unsigned int sat, unsigned int val);
	

private:
	void OneByOne_core(int delaytime);
	bool IsValidPin(int pin);
	void InitTimer1(void);
	void InitTimer2(void);
	bool LoadNotTooHigh(void);
	
	const int m_timer;
	const bool m_noSPI;
	const int m_latchPin;
	const int m_dataPin;
	const int m_clockPin;	
	
	int m_prescaler;
	

public:
	int m_ledFrequency;  
	unsigned char m_maxBrightness;
	unsigned char m_amountOfRegisters;
	int m_amountOfOutputs;
	int m_pinGrouping;
	unsigned char * m_PWMValues;
	unsigned char m_counter;

};

#endif
