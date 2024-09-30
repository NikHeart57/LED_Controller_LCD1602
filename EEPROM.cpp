#include <avr/io.h>
#include "EEPROM.h"

void EEPROM_WriteByte(char adr, char value)
{
	while (EECR & (1<<EEWE));
	EEAR = (uint16_t)adr;
	EEDR = value;
	EECR |= (1<<EEMWE);
	EECR |= (1<<EEWE);
}

char EEPROM_ReadByte(char adr)
{
	while (EECR & (1<<EEWE));
	EEAR = (uint16_t)adr;
	EECR |= (1<<EERE);
	return EEDR;
}


