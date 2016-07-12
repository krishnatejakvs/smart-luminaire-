



#define EEPROM_ADDRESS     0xA0 


unsigned char eeprom_read(unsigned char address);
void eeprom_write(unsigned char address, unsigned char data);