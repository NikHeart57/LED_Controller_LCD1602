#ifndef EEPROM_H_
#define EEPROM_H_

void EEPROM_WriteByte(char adr, char value);			// Запись байта
char EEPROM_ReadByte(char adr);							// Чтение байта

#endif /* EEPROM_H_ */

