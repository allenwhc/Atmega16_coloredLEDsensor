/*
 * Atmega16_coloredLEDsensor.c
 *
 * Created: 2/13/2015 9:26:00 AM
 *  Author: WHC
 */ 

#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define dataport PORTB
#define commport_pwm PORTD

//LCD display
#define rs	PD0
#define wr	PD1
#define en	PD2

//PWM
#define red		PD4
#define green	PD5
#define blue	PD7

int LCD_Init(void);
int LCD_SendData(unsigned char *);
int wrcomm(void);
int wrdata(void);

int InitPWM(void);
int ConfigPWM(void);

int InitADC(void);
uint16_t readADC(uint8_t channel);

/************************************************************************/
/*							 Main Function                              */
/************************************************************************/
int main(void)
{
	//set port ddr
	DDRB = 0xFF;
	DDRD = 0xFF;	//set portD 4,5,6 as output pins
	//DDRA = 0x00;
	
	//initialize LCD
	LCD_Init();
	//initialize PWM
	InitPWM();
	//initialize ADC
	InitADC();
	//set for PWM
	ConfigPWM();
	unsigned int adcResult[4];
	int result = 0;
	uint8_t adcResult_total, adcResult_red, adcResult_green, adcResult_blue;
	//adcResult[0] = readADC(0);
	//adcResult[1] = readADC(1);
	//adcResult[2] = readADC(2);
	//adcResult[3] = readADC(3);
	adcResult_total = readADC(0);
	_delay_ms(1000);
	adcResult_red = readADC(0x01);
	adcResult_green = readADC(0x02);
	adcResult_blue = readADC(0x03);
//	printf(adcResult_red);
	
	char total_buffer[10];
	char red_buffer[10];
	char green_buffer[10];
	char blue_buffer[10];
	
	itoa(adcResult_total, total_buffer, 10);
	itoa(adcResult_red, red_buffer, 10);
	itoa(adcResult_green, green_buffer, 10);
	itoa(adcResult_blue, blue_buffer, 10);
	
	LCD_SendData("RGB=");
	LCD_SendData(total_buffer);
	LCD_SendData("             ");
	LCD_SendData("R=");
	LCD_SendData(red_buffer);
	
	LCD_SendData(", G=");
	LCD_SendData(green_buffer);
	
	LCD_SendData(", B=");
	LCD_SendData(blue_buffer);
	dataport = 0xC0;
	wrcomm();
	LCD_SendData("Composition is:");
	
	return 1;
}
/************************************************************************/
/*							  LCD Display				                */
/************************************************************************/
int LCD_Init(void)
{
	_delay_ms(15);
	
	//Function set
	dataport = 0x38;	//0b00111000, DL = "HIGH", 8-bit mode; N = "high", 2-line display; F=0, 5x8 display
	wrcomm(); 
	_delay_us(50);
	
	//clear LCD screen
	dataport = 0x01;	//clear screen
	wrcomm();
	_delay_ms(2);
	
	//display on/off control
	dataport = 0x0e;	//0b00001110, D=1, display is turned on; C=1, cursor is turned on; B=0, blink is off
	wrcomm();
	_delay_us(50);
	//cursor at line 1
	dataport = 0x80;
	wrcomm();
	_delay_us(50);
	//entry mode set
	
	//cursor or display shift
	dataport = 0x1C;	//0b00011100, S/C=1, R/L=1, shift all the display to the right, cursor moves according to the display
	wrcomm();
	_delay_us(50);
	
	return 1;
	
}

int LCD_SendData(unsigned char *s)
{
	unsigned char *j = s;
	int i;
	for (i = 0; i < strlen(j); i++)
	{
		dataport = j[i];
		wrdata();
	}
	return 1;
}
int wrcomm(void)
{
	commport_pwm &= ~(1 << rs);		//selecting command register
	commport_pwm &= ~(1 << wr);		//selecting write mode
	commport_pwm |= 1 << en;		//EN = 1 
	_delay_ms(2);
	commport_pwm &= ~(1 << en);		//EN = 0
	_delay_ms(10);
	return 1;
}
int wrdata(void)
{
	commport_pwm |= 1 << rs;		//selecting data register
	commport_pwm &= ~(1 << wr);		//selecting write mode
	commport_pwm |= 1 << en;		//EN = 1
	_delay_ms(2);
	commport_pwm &= ~(1 << en);		//EN = 0
	_delay_ms(100);
	return 1;
}


/************************************************************************/
/*                    Pulse Width Modulation                            */
/************************************************************************/
int InitPWM()
{
	//initialize PWM for timer 1
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);	
	TCCR1B |= (1 << CS10) | (1 << WGM12);	//8-bit fast PWM, no pre-scaler
	TIMSK |= (1 << TOIE1);	//enable overflow
	
	//initialize PWM for timer 2
	TCCR2 |= (1 << COM21) | (1 << WGM20) | (1 << WGM21) | (1 << CS20);	//select fast PWM, no prescaler
	return 0;
}

int ConfigPWM()
{
	OCR2 = 0.7*255;			//blue
	bool flag_a = true;
	bool flag_b = true;

	uint8_t timer1A = 0xd0;	// green
	uint8_t timer1B = 0xc0;	// red
	OCR1A = timer1A;
	OCR1B = timer1B;
	
	timer1A += flag_a? 0:0;
	timer1B += flag_b? 0:0;
	
	if (timer1A == 0xff)
	{
		flag_a = false;
	}
	if (timer1B == 0xff)
	{
		flag_b =false;
	}
	
	return 0;
}

/************************************************************************/
/*					 Analog-Digital Conversion                          */
/************************************************************************/
int InitADC()
{
	ADMUX |= (1 << REFS0);	//select Vref=Avcc
	ADCSRA |= (1 << ADEN) | (1 <<ADPS2); //enable ADC, and set pre-scaler = 16
	return 0;
}

uint16_t readADC(uint8_t channel)
{
	channel &= 0b00000111;	//AND 7 keeps the value of 'channel' between 0 and 7
	ADMUX |= (ADMUX & 0b11110000) | channel;	//clear the bottom 3 bits of ADMUX before OR
	//start single conversion
	ADCSRA |= (1 << ADSC);
	
	//wait for the conversion to complete
	//thus, ADSC becomes '0' again
	while (ADCSRA & (1 << ADSC));
	return ADC;
}
