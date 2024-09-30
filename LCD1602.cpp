#define F_CPU 14745600UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "LCD1602.h"

char cur_pos_x = 0;
char cur_pos_y = 0;

void LCD1602_Init_8pins(void)
{
	COMAND_DDR |= (1 << RS)|(1 << RW)|(1 << E);
	COMAND_PORT = 0x00;
	
	DATA_DDR = 0xff;
	DATA_PORT = 0x00;
	
	_delay_ms(15);							// Ждем 15 мс  потому что так в даташите сказано
	Send_Cmd(0b00111000);					// Function set // Sets to 8-bit operation and selects 2-line display and 5 ? 8	dot character font.
	Send_Cmd(0b00001100);					// Display on/off control
	Send_Cmd(0b00000110);					// Entry mode set
	Send_Cmd(CMD_DISPLAY_CLR);				// Clear
}


void Send_Cmd(char comand)
{
	DATA_PORT = comand;
	
	COMAND_PORT &= ~(1 << RS);				// 0 в пин RS (команда/данные)	- передача команды
	COMAND_PORT &= ~(1 << RW);				// 0 в пин RW (запись/чтение)	- запись данных
	COMAND_PORT |= (1 << E);				// 1 в пин тактирования
	_delay_us(100);
	
	COMAND_PORT &= ~(1 << E);				// 0 в пин тактирования			- тактирование для отправки данных
	_delay_us(100);
}


void Send_Data(char data)
{
	DATA_PORT = data;
	
	COMAND_PORT |= (1 << RS);				// 0 в пин RS (команда/данные)	- передача данных
	COMAND_PORT &= ~(1 << RW);				// 0 в пин RW (запись/чтение)	- запись данных
	COMAND_PORT |= (1 << E);				// 1 в пин тактирования
	_delay_us(100);
	
	COMAND_PORT &= ~(1 << E);
	_delay_us(100);
}


void Set_Pos(char x, char y)
{

	cur_pos_x = x;
	cur_pos_y = y;
	Send_Cmd((0x40 * y + x) | 0b10000000);

}


void Print_Char(char c_ascii)
{
	Send_Data((char)c_ascii);
}


void Print_String(char* s_ascii)
{	
	int array_counter = 0;
	
	while (s_ascii[array_counter] != '\0')
	{	
		Set_Pos(cur_pos_x, cur_pos_y);
		Print_Char((char)s_ascii[array_counter++]);
		cur_pos_x++;
		
		if (cur_pos_x >= 16)
		{
			cur_pos_x = 0;
			cur_pos_y++;
			
			if (cur_pos_y >= 2)
			{
				cur_pos_y = 0;
			}
		}		
	}
}



