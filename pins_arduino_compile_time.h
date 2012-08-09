#ifndef Pins_Arduino_Compile_Time_h
#define Pins_Arduino_Compile_time_h


/* This is an alternative to pins_arduino.h
In pins_arduino.h and .cpp the look up arrays are defined as:
const uint16_t PROGMEM port_to_output_PGM[]
This places the array in program memory. pgm_read_byte() functions are used to retreive the array from program memory at runtime.

Because the arrays are received at runtime, it takes some instructions to look up the outputs.
This is not a problem in most cases, but it is if you are trying to write super fast code.

To make use of efficient sbi (Set Bit Immidiate) and cbi (Clear Bit Immidiate), the pin to write to must be defined as const.
The compiler does not understand that digitalPinToPort(2) is constant and does not optimize it away.

In this file I redefine the arrays as:
volatile uint8_t * const port_to_output_PGM_ct[]

So it is a constant pointer to a volatile uint8_t: 
a pointer to an output register that stays in the same place (* const), but can change in value (volatile uint8_t)

These definitions are understood by the compiler and result in superfast code,
but still allow you to use the arduino pin numbers instead of registers like PORTB

(C) 2011-2012 Elco Jacobs. www.elcojacobs.com

*/

#include <pins_arduino.h>

#define NOT_A_PIN 0
#define NOT_A_PORT 0


// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATMEGA8 & 168 / ARDUINO
//
//                  +-\/-+
//            PC6  1|    |28  PC5 (AI 5)
//      (D 0) PD0  2|    |27  PC4 (AI 4)
//      (D 1) PD1  3|    |26  PC3 (AI 3)
//      (D 2) PD2  4|    |25  PC2 (AI 2)
// PWM+ (D 3) PD3  5|    |24  PC1 (AI 1)
//      (D 4) PD4  6|    |23  PC0 (AI 0)
//            VCC  7|    |22  GND
//            GND  8|    |21  AREF
//            PB6  9|    |20  AVCC
//            PB7 10|    |19  PB5 (D 13)
// PWM+ (D 5) PD5 11|    |18  PB4 (D 12)
// PWM+ (D 6) PD6 12|    |17  PB3 (D 11) PWM
//      (D 7) PD7 13|    |16  PB2 (D 10) PWM
//      (D 8) PB0 14|    |15  PB1 (D 9) PWM
//                  +----+
//
// (PWM+ indicates the additional PWM pins on the ATmega168.)

// ATMEL ATMEGA1280 / ARDUINO
//
// 0-7 PE0-PE7   works
// 8-13 PB0-PB5  works
// 14-21 PA0-PA7 works
// 22-29 PH0-PH7 works
// 30-35 PG5-PG0 works
// 36-43 PC7-PC0 works
// 44-51 PJ7-PJ0 works
// 52-59 PL7-PL0 works
// 60-67 PD7-PD0 works
// A0-A7 PF0-PF7
// A8-A15 PK0-PK7

#define PA 1
#define PB 2
#define PC 3
#define PD 4
#define PE 5
#define PF 6
#define PG 7
#define PH 8
#define PJ 10
#define PK 11
#define PL 12


#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

volatile uint8_t * const port_to_output_PGM_ct[] = {
	NOT_A_PORT,
	&PORTA,
	&PORTB,
	&PORTC,
	&PORTD,
	&PORTE,
	&PORTF,
	&PORTG,
	&PORTH,
	NOT_A_PORT,
	&PORTJ,
	&PORTK,
	&PORTL,
};

const uint8_t digital_pin_to_port_PGM_ct[] = {
	// PORTLIST
	// -------------------------------------------
	PE      , // PE 0 ** 0 ** USART0_RX
	PE      , // PE 1 ** 1 ** USART0_TX
	PE      , // PE 4 ** 2 ** PWM2
	PE      , // PE 5 ** 3 ** PWM3
	PG      , // PG 5 ** 4 ** PWM4
	PE      , // PE 3 ** 5 ** PWM5
	PH      , // PH 3 ** 6 ** PWM6
	PH      , // PH 4 ** 7 ** PWM7
	PH      , // PH 5 ** 8 ** PWM8
	PH      , // PH 6 ** 9 ** PWM9
	PB      , // PB 4 ** 10 ** PWM10
	PB      , // PB 5 ** 11 ** PWM11
	PB      , // PB 6 ** 12 ** PWM12
	PB      , // PB 7 ** 13 ** PWM13
	PJ      , // PJ 1 ** 14 ** USART3_TX
	PJ      , // PJ 0 ** 15 ** USART3_RX
	PH      , // PH 1 ** 16 ** USART2_TX
	PH      , // PH 0 ** 17 ** USART2_RX
	PD      , // PD 3 ** 18 ** USART1_TX
	PD      , // PD 2 ** 19 ** USART1_RX
	PD      , // PD 1 ** 20 ** I2C_SDA
	PD      , // PD 0 ** 21 ** I2C_SCL
	PA      , // PA 0 ** 22 ** D22
	PA      , // PA 1 ** 23 ** D23
	PA      , // PA 2 ** 24 ** D24
	PA      , // PA 3 ** 25 ** D25
	PA      , // PA 4 ** 26 ** D26
	PA      , // PA 5 ** 27 ** D27
	PA      , // PA 6 ** 28 ** D28
	PA      , // PA 7 ** 29 ** D29
	PC      , // PC 7 ** 30 ** D30
	PC      , // PC 6 ** 31 ** D31
	PC      , // PC 5 ** 32 ** D32
	PC      , // PC 4 ** 33 ** D33
	PC      , // PC 3 ** 34 ** D34
	PC      , // PC 2 ** 35 ** D35
	PC      , // PC 1 ** 36 ** D36
	PC      , // PC 0 ** 37 ** D37
	PD      , // PD 7 ** 38 ** D38
	PG      , // PG 2 ** 39 ** D39
	PG      , // PG 1 ** 40 ** D40
	PG      , // PG 0 ** 41 ** D41
	PL      , // PL 7 ** 42 ** D42
	PL      , // PL 6 ** 43 ** D43
	PL      , // PL 5 ** 44 ** D44
	PL      , // PL 4 ** 45 ** D45
	PL      , // PL 3 ** 46 ** D46
	PL      , // PL 2 ** 47 ** D47
	PL      , // PL 1 ** 48 ** D48
	PL      , // PL 0 ** 49 ** D49
	PB      , // PB 3 ** 50 ** SPI_MISO
	PB      , // PB 2 ** 51 ** SPI_MOSI
	PB      , // PB 1 ** 52 ** SPI_SCK
	PB      , // PB 0 ** 53 ** SPI_SS
	PF      , // PF 0 ** 54 ** A0
	PF      , // PF 1 ** 55 ** A1
	PF      , // PF 2 ** 56 ** A2
	PF      , // PF 3 ** 57 ** A3
	PF      , // PF 4 ** 58 ** A4
	PF      , // PF 5 ** 59 ** A5
	PF      , // PF 6 ** 60 ** A6
	PF      , // PF 7 ** 61 ** A7
	PK      , // PK 0 ** 62 ** A8
	PK      , // PK 1 ** 63 ** A9
	PK      , // PK 2 ** 64 ** A10
	PK      , // PK 3 ** 65 ** A11
	PK      , // PK 4 ** 66 ** A12
	PK      , // PK 5 ** 67 ** A13
	PK      , // PK 6 ** 68 ** A14
	PK      , // PK 7 ** 69 ** A15
};

const uint8_t digital_pin_to_bit_PGM_ct[] = {
	// PIN IN PORT
	// -------------------------------------------
	0         , // PE 0 ** 0 ** USART0_RX
	1         , // PE 1 ** 1 ** USART0_TX
	4         , // PE 4 ** 2 ** PWM2
	5         , // PE 5 ** 3 ** PWM3
	5         , // PG 5 ** 4 ** PWM4
	3         , // PE 3 ** 5 ** PWM5
	3         , // PH 3 ** 6 ** PWM6
	4         , // PH 4 ** 7 ** PWM7
	5         , // PH 5 ** 8 ** PWM8
	6         , // PH 6 ** 9 ** PWM9
	4         , // PB 4 ** 10 ** PWM10
	5         , // PB 5 ** 11 ** PWM11
	6         , // PB 6 ** 12 ** PWM12
	7         , // PB 7 ** 13 ** PWM13
	1         , // PJ 1 ** 14 ** USART3_TX
	0         , // PJ 0 ** 15 ** USART3_RX
	1         , // PH 1 ** 16 ** USART2_TX
	0         , // PH 0 ** 17 ** USART2_RX
	3         , // PD 3 ** 18 ** USART1_TX
	2         , // PD 2 ** 19 ** USART1_RX
	1         , // PD 1 ** 20 ** I2C_SDA
	0         , // PD 0 ** 21 ** I2C_SCL
	0         , // PA 0 ** 22 ** D22
	1         , // PA 1 ** 23 ** D23
	2         , // PA 2 ** 24 ** D24
	3         , // PA 3 ** 25 ** D25
	4         , // PA 4 ** 26 ** D26
	5         , // PA 5 ** 27 ** D27
	6         , // PA 6 ** 28 ** D28
	7         , // PA 7 ** 29 ** D29
	7         , // PC 7 ** 30 ** D30
	6         , // PC 6 ** 31 ** D31
	5         , // PC 5 ** 32 ** D32
	4         , // PC 4 ** 33 ** D33
	3         , // PC 3 ** 34 ** D34
	2         , // PC 2 ** 35 ** D35
	1         , // PC 1 ** 36 ** D36
	0         , // PC 0 ** 37 ** D37
	7         , // PD 7 ** 38 ** D38
	2         , // PG 2 ** 39 ** D39
	1         , // PG 1 ** 40 ** D40
	0         , // PG 0 ** 41 ** D41
	7         , // PL 7 ** 42 ** D42
	6         , // PL 6 ** 43 ** D43
	5         , // PL 5 ** 44 ** D44
	4         , // PL 4 ** 45 ** D45
	3         , // PL 3 ** 46 ** D46
	2         , // PL 2 ** 47 ** D47
	1         , // PL 1 ** 48 ** D48
	0         , // PL 0 ** 49 ** D49
	3         , // PB 3 ** 50 ** SPI_MISO
	2         , // PB 2 ** 51 ** SPI_MOSI
	1         , // PB 1 ** 52 ** SPI_SCK
	0         , // PB 0 ** 53 ** SPI_SS
	0         , // PF 0 ** 54 ** A0
	1         , // PF 1 ** 55 ** A1
	2         , // PF 2 ** 56 ** A2
	3         , // PF 3 ** 57 ** A3
	4         , // PF 4 ** 58 ** A4
	5         , // PF 5 ** 59 ** A5
	6         , // PF 6 ** 60 ** A6
	7         , // PF 7 ** 61 ** A7
	0         , // PK 0 ** 62 ** A8
	1         , // PK 1 ** 63 ** A9
	2         , // PK 2 ** 64 ** A10
	3         , // PK 3 ** 65 ** A11
	4         , // PK 4 ** 66 ** A12
	5         , // PK 5 ** 67 ** A13
	6         , // PK 6 ** 68 ** A14
	7         , // PK 7 ** 69 ** A15
};

#elif defined(__AVR_ATmega32U4__)

#if defined(CORE_TEENSY)

volatile uint8_t * const port_to_output_PGM_ct[] = {
	NOT_A_PORT, NOT_A_PORT, &PORTB, &PORTC, &PORTD, &PORTE, &PORTF
};
const uint8_t digital_pin_to_port_PGM_ct[] = {
	PB, PB, PB, PB, PB, PD, PD, PD, PD, PC, PC,
	PD, PD, PB, PB, PB, PF, PF, PF, PF, PF, PF,
	PD, PD, PE
};
const uint8_t digital_pin_to_bit_PGM_ct[] = {
	0,  1,  2,  3,  7,  0,  1,  2,  3,  6,  7,
	6,  7,  4,  5,  6,  7,  6,  5,  4,  1,  0,
	4,  5,  6
};
#else
//Assumes Arduino Leonardo
volatile uint8_t * const port_to_output_PGM_ct[] = {
	NOT_A_PORT, NOT_A_PORT, &PORTB, &PORTC, &PORTD, &PORTE, &PORTF
};
const uint8_t digital_pin_to_port_PGM_ct[] = {
	PD, PD, PD, PD, PD, PC, PD, PE, PB, PB, PB,
	PB, PD, PC, PB, PB, PB, PB, PF, PF, PF, PF,
	PF, PF, PD, PD, PB, PB, PB, PD
};
const uint8_t digital_pin_to_bit_PGM_ct[] = {
	2, 3, 1, 0, 4, 6, 7, 6, 4, 5, 6,
	7, 6, 7, 3, 1, 2, 0, 7, 6, 5, 4,
	1, 0, 4, 7, 4, 5, 6, 6
};
#endif
#elif defined(__AVR_AT90USB1286__)

volatile uint8_t * const port_to_output_PGM_ct[] = {
        NOT_A_PORT, &PORTA, &PORTB, &PORTC, &PORTD, &PORTE, &PORTF
};
const uint8_t digital_pin_to_port_PGM_ct[] = {
	PD, PD, PD, PD, PD, PD, PD, PD, PE, PE,
	PC, PC, PC, PC, PC, PC, PC, PC, PE, PE,
	PB, PB, PB, PB, PB, PB, PB, PB, PA, PA,
	PA, PA, PA, PA, PA, PA, PE, PE, PF, PF,
	PF, PF, PF, PF, PF, PF
};
const uint8_t digital_pin_to_bit_PGM_ct[] = {
	0,  1,  2,  3,  4,  5,  6,  7,  0,  1,
	0,  1,  2,  3,  4,  5,  6,  7,  6,  7,
	0,  1,  2,  3,  4,  5,  6,  7,  0,  1,
	2,  3,  4,  5,  6,  7,  4,  5,  0,  1,
	2,  3,  4,  5,  6,  7
};

#else

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)

volatile uint8_t * const port_to_output_PGM_ct[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	&PORTB,
	&PORTC,
	&PORTD,
};


const uint8_t digital_pin_to_port_PGM_ct[] = {
	PD, // 0 
	PD,
	PD,
	PD,
	PD,
	PD,
	PD,
	PD,
	PB, // 8 
	PB,
	PB,
	PB,
	PB,
	PB,
	PC, // 14 
	PC,
	PC,
	PC,
	PC,
	PC,
};

const uint8_t digital_pin_to_bit_PGM_ct[] = {
	0, // 0, port D 
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	0, // 8, port B 
	1,
	2,
	3,
	4,
	5,
	0, // 14, port C /
	1,
	2,
	3,
	4,
	5,
};

#endif


#endif
