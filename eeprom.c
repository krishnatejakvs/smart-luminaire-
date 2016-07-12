 #include "eeprom.h"
// I2C Bus functions
#asm
   .equ __i2c_port=0x0B ;PORTD
   .equ __sda_bit=3
   .equ __scl_bit=4
#endasm
#include <i2c.h>
#include <delay.h>



unsigned char eeprom_read(unsigned char address) {

    unsigned char data;
    #asm("cli")
    i2c_start();
    i2c_write(EEPROM_ADDRESS);
    i2c_write(address);
    i2c_start();
    i2c_write(EEPROM_ADDRESS | 1);
    data=i2c_read(0);
    i2c_stop();
    #asm("sei")  
    return data;
}

void eeprom_write(unsigned char address, unsigned char data) {
    #asm("cli")
    i2c_start();
    i2c_write(EEPROM_ADDRESS);
    i2c_write(address);
    i2c_write(data);
    i2c_stop();
    /* 10ms delay to complete the write operation    ( reduce to 5 msec and test)*/ 
    delay_ms(10);
    #asm("sei")
}  

