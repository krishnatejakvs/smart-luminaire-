
#ifndef UTILITY_H
#define UTILITY_H

void init(void);
int parity(unsigned char *bp);
void swap(char*,char*);
void bcdToascii(unsigned char value);
void floattoascii(float x, char y);

void bin2bcd( unsigned  int val );



#endif 