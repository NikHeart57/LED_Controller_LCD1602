#include <avr/io.h>
#include "DS1307.h"



//==============================================================================
//							Функции протокола I2C
//==============================================================================
// Обработчик ошибок
void I2CErrorHandler(void)
{
	
}


//  Функции мастера трансмиттера  //
void I2CMTInit(void)
{
	TWBR = 100;												// Задает скорость соединения. Чем больше тем медленнее; 255 -> 28кбит/с; 66 -> 100кбит/с; 11 -> 388кбит/с
	TWSR |= (0 << TWPS1)|(0 << TWPS0);						// Задает скорость соединения
}


void I2CMTStart(void)
{
	TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN);			// Send START condition
	while (!(TWCR & (1 << TWINT)));							// Wait for TWINT Flag set. This indicates that the START condition has been transmitted
	if ((TWSR & 0xF8) != MT_START)							// Check value of TWI Status Register. Mask prescaler bits. If status different from MT_START go to ERROR
	{
		I2CErrorHandler();
	}
}

void I2CMTSendSLA(char sla)
{
	TWDR = sla;												// Load data into TWDR Register.
	TWCR = (1 << TWINT)|(1 << TWEN);						// Clear TWINT bit in TWCR to start transmission of data
	while (!(TWCR & (1 << TWINT)));							// Wait for TWINT Flag set. This indicates that the SLA has been transmitted
	if ((TWSR & 0xF8) != MT_DATA_TRANSMITED_ACK_RECIEVED)	// Check value of TWI Status Register. Mask	prescaler bits. If status different from MT_DATA_TRANSMITED_ACK_RECIEVED go to ERROR
	{
		I2CErrorHandler();
	}
}

void I2CMTSendData(char data)
{
	TWDR = data;											// Load data into TWDR Register.
	TWCR = (1 << TWINT)|(1 << TWEN);						// Clear TWINT bit in TWCR to start transmission of data
	while (!(TWCR & (1 << TWINT)));							// Wait for TWINT Flag set. This indicates that the DATA has been transmitted
	if ((TWSR & 0xF8) != MT_DATA_TRANSMITED_ACK_RECIEVED)	// Check value of TWI Status Register. Mask	prescaler bits. If status different from MT_DATA_TRANSMITED_ACK_RECIEVED go to ERROR
	{
		I2CErrorHandler();
	}
}

void I2CMTStop(void)
{
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTO);			// Transmit STOP condition
}




//  Функции мастера рессивера  //
void I2CMRInit(void)
{
	TWBR = 66;												// Задает скорость соединения. Чем больше тем медленнее; 255 -> 28кбит/с; 66 -> 100кбит/с; 11 -> 388кбит/с
	TWSR |= (0 << TWPS1)|(0 << TWPS0);						// Задает скорость соединения.
}

void I2CMRStart(void)
{
	TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN);			// Send START condition
	while (!(TWCR & (1 << TWINT)));							// Wait for TWINT Flag set. This indicates that the START condition has been transmitted
	if ((TWSR & 0xF8) != MR_START)							// Check value of TWI Status Register. Mask prescaler bits. If status different from MR_START go to ERROR
	{
		I2CErrorHandler();
	}
}

void I2CMRSendSLA(char sla)
{
	TWDR = sla;												// Load data into TWDR Register.
	TWCR = (1 << TWINT)|(1 << TWEN);						// Clear TWINT bit in TWCR to start transmission of data
	while (!(TWCR & (1 << TWINT)));							// Wait for TWINT Flag set. This indicates that the SLA has been transmitted
	if ((TWSR & 0xF8) != MR_SLA_R_TRANSMITED_ACK_RECIEVED)	// Check value of TWI Status Register. Mask	prescaler bits. If status different from MR_SLA_R_TRANSMITED_ACK_RECIEVED go to ERROR
	{
		I2CErrorHandler();
	}
}

char I2CMRRead(void)
{
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWEA);			// Clear TWINT bit in TWCR to start receiving of data, set bit in TWEA to return ASK bit
	while (!(TWCR & (1 << TWINT)));							// Wait for TWINT Flag set. This indicates that the data has been recieved
	if ((TWSR & 0xF8) != MR_DATA_RECIEVED_ACK_RETURNED)		// Check value of TWI Status Register. Mask prescaler bits. If status different from MR_DATA_RECIEVED_ACK_RETURNED go to ERROR
	{
		I2CErrorHandler();
		return 0;
	}
	return TWDR;											// Возврат данных
}

char I2CMRReadLast(void)
{
	TWCR = (1 << TWINT)|(1 << TWEN);						// Clear TWINT bit in TWCR to start receiving of data, DONT set bit in TWEA to return NO ASK bit
	while (!(TWCR & (1 << TWINT)));							// Wait for TWINT Flag set. This indicates that the data has been recieved
	if ((TWSR & 0xF8) != MR_DATA_RECIEVED_NOT_ACK_RETURNED)	// Check value of TWI Status Register. Mask prescaler bits. If status different from MR_DATA_RECIEVED_NOT_ACK_RETURNED go to ERROR
	{
		I2CErrorHandler();
		return 0;
	}
	return TWDR;											// Возврат данных
}

void I2CMRStop(void)
{
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTO);			// Transmit STOP condition
}



//==============================================================================
//		Функции конвертации из десятичного числа в двоично-десятичное
//==============================================================================

char BinToDec(char binary)									// Перевод двоично-десятичного числа в десятичное
{
	char decimal = ((binary >> 4) * 10 + (0b00001111 & binary));
	return decimal;
}

char DecToBin(char decimal)									// Перевод десятичного числа в двоично-десятичное
{
	char binary = ((decimal / 10) << 4)|(decimal % 10);
	return binary;
}


//==============================================================================
//							Функции общения с DS1307
//==============================================================================
// Адрес часов слейв на запись W	0b11010000
// Адрес часов слейв на чтение R	0b11010001
	
void DS1307_WriteTime(char time[])
{
	// ===== Включение I2C =====
	I2CMTInit();							// Инициализация мастера								// Ответ TWSR: 11111 = 0xf8 - No relevant state information	available; TWINT = “0”
	I2CMTStart();							// Задание стартового условия мастером					// Ответ TWSR: 00001 = 0x08 - A START condition has been transmitted
	
	// ===== Начало передачи и передача начального адреса с котрого начнется запись =====
	I2CMTSendSLA(DS1307_WRITE_ADRESS);		// Передача адреса слейва								// Ответ TWSR: 00011 = 0x18 - SLA+W has been transmitted;		ACK has been received
	I2CMTSendData(0x00);					// Передача адреса ячейки с которого начнется запись	// Ответ TWSR: 00101 = 0x28 - Data byte has been transmitted;	ACK has been received
		
	// ===== Передача данных =====			// Ответы TWSR: 00101 = 0x28 - Data byte has been transmitted;	ACK has been received
	I2CMTSendData(DecToBin(time[0]));		// Адрес - 0x00	7-CH	6-10Sec		5-10Sec		4-10Sec		3-Sec	2-Sec	1-Sec	0-Sec	(CH - работа осцилятора часов (вкл/выкл микросхенмы DS1307) CH-0 - вкл осцилятор, CH-1 - вЫкл осцилятор)
	I2CMTSendData(DecToBin(time[1]));		// Адрес - 0x01	7-0		6-10Min		5-10Min		4-10Min		3-Min	2-Min	1-Min	0-Min
	I2CMTSendData(DecToBin(time[2]));		// Адрес - 0x02	7-0		6-12/24		5-10H/AMPM	4-10Hour	3-Hour	2-Hour	1-Hour	0-Hour
	I2CMTSendData(DecToBin(time[3]));		// Адрес - 0x03	7-0		6-0			5-0			4-0			3-0		2-Day	1-Day	0-Day	(День недели)
	I2CMTSendData(DecToBin(time[4]));		// Адрес - 0x04	7-0		6-0			5-10Date	4-10Date	3-Date	2-Date	1-Date	0-Date	(День месяца)
	I2CMTSendData(DecToBin(time[5]));		// Адрес - 0x05	7-0		6-0			5-0			4-10Month	3-Month	2-Month	1-Month	0-Month
	I2CMTSendData(DecToBin(time[6]));		// Адрес - 0x06	7-10Y	6-10Y		5-10Y		4-10Y		3-Y		2-Y		1-Y		0-Y
	I2CMTSendData(0b00010000);				// Адрес - 0x07	7-OUT	6-0			5-0			4-SQWE		3-0		2-0		1-RS1	0-RS0	
	// OUT - логика на выходе (этот бит управляет логическим уровнем на выводе SQW/OUT, когда выход сигнала с прямоугольными импульсами отключен. Если SQWE = 0, то логический уровень на выводе SQW/OUT равен 1, если OUT = 1, и 0, если OUT = 0.); 
	// SQWE - генератор на выходе (когда этот бит установлен в 1, включается генерация прямоугольных импульсов); 
	// RS1..0 - прескелереры частоты генератора SQWE (00 - 1Гц, 01 - 4096Гц, 10 - 8192Гц, 11 - 32768Гц)
	
	// ===== Выключение I2C =====
	I2CMTStop();							// Стоп от мастера										// Ответ TWSR: 11111 = 0xf8 - No relevant state information	available; TWINT = “0”
}

void DS1307_ReadTime(char time[])
{
	// ===== Включение I2C =====
	I2CMTInit();							// Инициализация мастера								// Ответ TWSR: 11111 = 0xf8 - No relevant state information	available; TWINT = “0”
	I2CMTStart();							// Задание стартового условия мастером					// Ответ TWSR: 00001 = 0x08 - A START condition has been transmitted
	
	// ===== Начало передачи и передача начального адреса с котрого начнется чтение =====
	I2CMTSendSLA(DS1307_WRITE_ADRESS);		// Передача адреса слэйва								// Ответ TWSR: 00011 = 0x18 - SLA+W has been transmitted;		ACK has been received
	I2CMTSendData(0x00);					// Передача адреса ячейки с которого начнется чтение	// Ответ TWSR: 00101 = 0x28 - Data byte has been transmitted;	ACK has been received
		
	// ===== Чтение данных =====	
	I2CMRStart();							// Повторный старт										// Ответ TWSR: 00010 = 0x10 - A repeated START condition has been transmitted
	I2CMTSendSLA(DS1307_READ_ADRESS);		// Отправка адреса слэйва								// Ответ TWSR: 01000 = 0x40 - SLA+R has been transmitted;	ACK has been received
	time[0] = BinToDec(I2CMRRead());		// Чтение данных с ACK									// Ответ TWSR: 01010 = 0x50 - Data byte has been received;	ACK has been returned
	time[1] = BinToDec(I2CMRRead());		// Чтение данных с ACK									// Ответ TWSR: 01010 = 0x50 - Data byte has been received;	ACK has been returned
	time[2] = BinToDec(I2CMRRead());		// Чтение данных с ACK									// Ответ TWSR: 01010 = 0x50 - Data byte has been received;	ACK has been returned
	time[3] = BinToDec(I2CMRRead());		// Чтение данных с ACK									// Ответ TWSR: 01010 = 0x50 - Data byte has been received;	ACK has been returned
	time[4] = BinToDec(I2CMRRead());		// Чтение данных с ACK									// Ответ TWSR: 01010 = 0x50 - Data byte has been received;	ACK has been returned
	time[5] = BinToDec(I2CMRRead());		// Чтение данных с ACK									// Ответ TWSR: 01010 = 0x50 - Data byte has been received;	ACK has been returned
	time[6] = BinToDec(I2CMRReadLast());	// Чтение данных С NO ACK								// Ответ TWSR: 01011 = 0x58 - Data byte has been received;	NOT ACK has been returned
	
	// ===== Выключение I2C =====			
	I2CMRStop();							// Стоп от мастера										// Ответ TWSR: 11111 = 0xf8 - No relevant state information	available; TWINT = “0”		
}

