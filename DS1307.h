#ifndef DS1307_H_
#define DS1307_H_

//==============================================================================
//								Коды статуса I2C
//==============================================================================

//	Miscellaneous States
#define ERR_NO_STATE_INFO						0xF8
#define ERR_ILLEGAL_START_STOP					0x00

// Status Codes for Master Transmitter Mode
#define MT_START								0x08
#define MT_REPEATED_START						0x10
#define MT_SLA_TRANSMITED_ACK_RECIEVED			0x18
#define MT_SLA_TRANSMITED_NOT_ACK_RECIEVED		0x20
#define MT_DATA_TRANSMITED_ACK_RECIEVED			0x28
#define MT_DATA_TRANSMITED_NOT_ACK_RECIEVED		0x30
#define MT_ARBITRATION_LOST						0x38

// Status Codes for Master Receiver Mode
#define MR_START								0x08
#define MR_REPEATED_START						0x10
#define MR_ARBITRATION_LOST						0x38
#define	MR_SLA_R_TRANSMITED_ACK_RECIEVED		0x40
#define	MR_SLA_R_TRANSMITED_NOT_ACK_RECIEVED	0x48
#define	MR_DATA_RECIEVED_ACK_RETURNED			0x50
#define	MR_DATA_RECIEVED_NOT_ACK_RETURNED		0x58



//==============================================================================
//								Адреса DS1307
//==============================================================================

#define DS1307_WRITE_ADRESS		0b11010000
#define DS1307_READ_ADRESS		0b11010001



/*
 *	https://robotchip.ru/obzor-chasov-realnogo-vremeni-ds1307/
 */

//==============================================================================
//							Функции протокола I2C
//==============================================================================
// Обработчик ошибок
void I2CErrorHandler(void);

// Функции мастера трансмиттера
void I2CMTInit(void);
void I2CMTStart(void);
void I2CMTSendSLA(char sla);
void I2CMTSendData(char data);
void I2CMTStop(void);

// Функции мастера рессивера
void I2CMRInit(void);
void I2CMRStart(void);
void I2CMRSendSLA(char sla);
char I2CMRRead(void);
char I2CMRReadLast(void);
void I2CMRStop(void);


//==============================================================================
//		Функции конвертации из десятичного числа в двоично-десятичное
//==============================================================================

char BinToDec(char c);				// Перевод двоично-десятичного числа в десятичное
char DecToBin(char c);				// Перевод десятичного числа в двоично-десятичное


//==============================================================================
//							Функции общения с DS1307
//==============================================================================
// Адрес часов слейв на запись W	0b11010000
// Адрес часов слейв на чтение R	0b11010001
		
void DS1307_WriteTime(char time[]);
void DS1307_ReadTime(char time[]);


#endif /* DS1307_H_ */

