/*****************************************************
This program was produced by the
CodeWizardAVR V2.05.0 Professional
Automatic Program Generator
© Copyright 1998-2010 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 
Version : 
Date    : 3/2/2013
Author  : NeVaDa
Company : 
Comments: 


Chip type               : ATmega88PA
Program type            : Application
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 256
*****************************************************/

#include <mega88a.h>
#include <delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <i2c.h>
#include <string.h>
#include "RTC.h"
#include "serial.h"
#include "eeprom.h"
#include "utility.h"
#include <1wire.h>
#include <ds18b20.h>
#define MAX_DS1820 1
#define LUX_SENSOR_ADDRESS        0b10010100



// Pin change 16-23 interrupt service routine
interrupt [PC_INT2] void pin_change_isr2(void)
{
//LUX sensor pin change interrupt service routine.

}

// External Interrupt 0 service routine
interrupt [EXT_INT0] void ext_int0_isr(void)
{

//this is an extra interrupt service routine, can be used for any purpose. Falling edge

}



// Standard Input/Output functions


#define ADC_VREF_TYPE 0x40
#define SWT 1



// Read the AD conversion result


unsigned int read_adc(unsigned char adc_input)
{  
ADMUX|=adc_input;
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=0x40;
// Wait for the AD conversion to complete
while ((ADCSRA & 0x10)==0);
ADCSRA|=0x10;
return ADCW;
} 



 void pwm()
 {
TCCR0A |= (1<<COM0A1)| (1<<WGM01) | (1<<WGM00) ; 
TCCR0B |= (1<<CS02) | (1<<CS00)  ; // divide by 1024
  
OCR0A = 50; // duty cycle 
TCNT0=0x00;
//OCR0A=0xAF;
OCR0B=0x00;
}


void pwm_off()
{   TCCR0A=0x00;
TCCR0B=0x00;
}


 volatile int seconds,flag;
                                                          


//// Timer1 output compare A interrupt service routine
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
      seconds++;  
     //putchar(seconds);

        if (seconds >= 10 || flag==0)
    {  
         PORTD.5=0;
        // pwm_off();
         seconds = 10; 
        
      //  puts("no mobile");
    }
                               // Place your code here

}
    // Pin change 0-7 interrupt service routine
interrupt [PC_INT0] void pin_change_isr0(void)
{
   //common pin chnage interrupt service routine for PIR and RTC   
 if (PINB.1==0)
  {
  seconds=0; 
  PORTD.5=1;
  //pwm(); 

  }
//  else
//  {
//    PORTD.5=0;
//    pwm_off();
//   } 

}

unsigned int LUX_read(unsigned char address)
{

    unsigned int data1,mantissa,exponent;     
    unsigned int data2;
    unsigned long int lux ;
     
     
    #asm("cli")
    i2c_start();
    i2c_write(LUX_SENSOR_ADDRESS);
    i2c_write(address);
    i2c_start();
    i2c_write(LUX_SENSOR_ADDRESS | 1);
    data1=i2c_read(0);  
    //data2=i2c_read(0);
    i2c_stop();
    #asm("sei")  
      
    #asm("cli")
    i2c_start();
    i2c_write(LUX_SENSOR_ADDRESS);
    i2c_write(address+1);
    i2c_start();
    i2c_write(LUX_SENSOR_ADDRESS | 1);
    data2=i2c_read(0);
    i2c_stop();
    #asm("sei")
         
    
    // puts("lux reading");
 
    
   
    mantissa= (data2 & 0x0f)+ ((data1 & 0x0f)<<4) ;
    exponent = ((data1 & 0xff)>>4);
    lux= (pow(2,exponent)) * mantissa * 0.045;
    bin2bcd(lux);   
//    putchar(exponent);
//    putchar(mantissa);          
    data1 << 8;
    data1 = data1 | data2;
    return data1;

}
 



// Declare your global variables here

void main(void)
{
// Declare your local variables here

// Crystal Oscillator division factor: 1
#asm("cli")
#pragma optsize-
CLKPR=0x80;
CLKPR=0x00;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Input/Output Ports initialization
// Port B initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTB=0x00;
DDRB=0x00;

// Port C initialization
// Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=Out 
// State6=T State5=T State4=T State3=T State2=T State1=P State0=0 
PORTC=0x02;
DDRC=0x01;

// Port D initialization
// Func7=In Func6=Out Func5=Out Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=0 State5=0 State4=T State3=T State2=T State1=T State0=T 
PORTD=0x00;
DDRD=0x40;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: Timer 0 Stopped
// Mode: Ph. correct PWM top=OCR0A
// OC0A output: Non-Inverted PWM
// OC0B output: Disconnected


// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer1 Stopped
// Mode: Normal top=0xFFFF
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: on
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off

TCCR1A=0x00;
TCCR1B=0x0D;
TCNT1H=0x00;
TCNT1L=0x02;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0b00011110;
OCR1AL=0b10000100;
//OCR1A = 7812;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: 125.000 kHz
// Mode: Normal top=0xFF
// OC2A output: Disconnected
// OC2B output: Disconnected
ASSR=0x00;
TCCR2A=0x00;
TCCR2B=0x00;
TCNT2=0x00;
OCR2A=0x00;
OCR2B=0x00;

// External Interrupt(s) initialization
// INT0: On
// INT0 Mode: Falling Edge
// INT1: Off
// Interrupt on any change on pins PCINT0-7: On
// Interrupt on any change on pins PCINT8-14: Off
// Interrupt on any change on pins PCINT16-23: On
EICRA=0x02;
EIMSK=0x01;
EIFR=0x01;
PCICR=0x05;
PCMSK0=0x06;
PCMSK2=0x80;
PCIFR=0x05;

// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=0x00;

// Timer/Counter 1 Interrupt(s) initialization
TIMSK1=0x02;

// Timer/Counter 2 Interrupt(s) initialization
TIMSK2=0x00;



// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART0 Mode: Asynchronous
// USART Baud Rate: 9600
UCSR0A=0x00;
UCSR0B=0xD8;
UCSR0C=0x06;
UBRR0H=0x00;
UBRR0L=0x33;
//
//// Analog Comparator initialization
//// Analog Comparator: Off
//// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
ADCSRB=0x00;
DIDR1=0x00;

// ADC initialization
// ADC Clock frequency: 500.000 kHz
// ADC Voltage Reference: AVCC pin
// ADC Auto Trigger Source: ADC Stopped
// Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On
// ADC4: On, ADC5: On
//DIDR0=0x00;
//ADCSRA=0xCF;
//ADMUX=0x40;
DIDR0=0x04;
ADMUX=ADC_VREF_TYPE & 0xff;
ADCSRA=0x87;


// SPI initialization
// SPI disabled

SPCR=0x00;

// TWI initialization
// TWI disabled
TWCR=0x00;

// I2C Bus initialization
i2c_init();

// 1 Wire Bus initialization
// 1 Wire Data port: PORTB
// 1 Wire Data bit: 0
// Note: 1 Wire port settings must be specified in the
// Project|Configure|C Compiler|Libraries|1 Wire IDE menu.
w1_init();
pwm();
SREG=0x80;
seconds=0;
// Global enable interrupts
#asm("sei")

while (1)
      {
       int count=0,i=0;
      unsigned char a;
      char tmp,x,tmp1,tmp2,tmp3,tmp4,tmp5;
     unsigned int LUX,adc,templ,temph;
     float current,adc1,adc2,temp;
     char buffer[8],buffer1[5];      // you need some space 
     char *str1 = buffer;
     char *str2 = buffer1;     

           
       tmp = getchar();
       if(tmp == 0x24)  // $
       {
            count=1;
            tmp1=getchar();
            tmp2=getchar();
            tmp3=getchar();
            tmp4=getchar();
            tmp5=getchar();
            
       }
       if(count!=0)
       {
       if(tmp1 == 0x30)
       {    flag=0;
            PORTD.5=0;
       }
       else 
        {   flag=1;
            PORTD.5=1;
            seconds=0;  
         }
       tmp3=tmp3-0x30;
        
       tmp3=(tmp3*0x19);
       
       OCR0A=tmp3;
       
       TCNT0=0xFF;
      
   
     putchar('$');
     //ds18B20 functions    
      w1_init();
      w1_write(0xCC);
      w1_write(0x44);
      //delay_ms(1000);
      w1_init();
      w1_write(0xCC);    
      w1_write(0xBE);
      templ=w1_read();
      temph=w1_read();
      temp=((temph & 0x04)>>2)*pow(2,6)+ ((temph & 0x02)>>1)*pow(2,5) +(temph & 0x01)*pow(2,4)+((templ & 0x80)>>7)*pow(2,3)+((templ & 0x40)>>6)*pow(2,2)+((templ & 0x20)>>5)*pow(2,1)+((templ & 0x10)>>4)*pow(2,0)+((templ & 0x08)>>3)*pow(2,-1)+((templ & 0x04)>>2)*pow(2,-2)+((templ & 0x02)>>1)*pow(2,-3)+(templ & 0x01)*pow(2,-4);
      ftoa(temp, 2, str2);
      //puts("temperature"); 
    putchar(buffer1[0]);
    putchar(buffer1[1]);
    putchar(buffer1[2]);
    putchar(buffer1[3]);
    putchar(buffer1[4]);
   // putchar(buffer1[5]);
   
    w1_init();
    putchar(',');
    //LUX read
    LUX = LUX_read(0x03);
    putchar(',');
    if(PORTD.5==1)
    {
        putchar(0x31);
    }
    else
    { 
        putchar(0x30);
     }
    putchar(',');    
    putchar('&');   
    count=0;
       
        }
    }
}
     