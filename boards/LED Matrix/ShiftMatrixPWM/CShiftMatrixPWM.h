/*
ShiftMatrixPWM.h - Library for Arduino to PWM many outputs using shift registers - Version 1
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


---> See ShiftMatrixPWM.h for more info
*/

#ifndef CShiftMatrixPWM_h
#define CShiftMatrixPWM_h

class CShiftMatrixPWM{
public:
	CShiftMatrixPWM(const int timerInUse); 
	~CShiftMatrixPWM();

public:
	void Start(int ledFrequency, unsigned char max_Brightness);
	void SetMatrixSize(int newAmountofRows, int newAmountOfColumnRegisters);
	void PrintInterruptLoad(void);
	void OneByOneSlow(void);
	void OneByOneFast(void);
	void SetOne(int row, int col, unsigned char value);
	void SetAll(unsigned char value);

	void SetGroupOf2(int row, int group, unsigned char v0, unsigned char v1);
	void SetGroupOf3(int row, int group, unsigned char v0, unsigned char v1, unsigned char v2);
	void SetGroupOf4(int row, int group, unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3);
	void SetGroupOf5(int row, int group, unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3, unsigned char v4);

private:
	void OneByOne_core(int delaytime);
	bool IsValidPin(int row, int col);
	void InitTimer1(void);
	void InitTimer2(void);
	int m_prescaler;
	bool LoadNotTooHigh(void);
	const int m_timer;


public:	
	int m_ledFrequency;  
	unsigned char m_maxBrightness;
	unsigned char m_amountOfColumnRegisters;
	int m_amountOfColumns;
	unsigned char m_amountOfRows;
	int m_amountOfOutputs;
	unsigned char * m_PWMValues;
	unsigned char m_counter;
	unsigned char m_currentRow;
};

#endif