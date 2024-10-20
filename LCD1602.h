#ifndef LCD1602_H_
#define LCD1602_H_

#define COMAND_PORT PORTB
#define COMAND_DDR	DDRB
#define DATA_PORT	PORTA
#define DATA_DDR	DDRA
#define DATA_PIN	PINA

#define E PB2			// ��� �������/������.	���� �� ������� �� ������ ����� ���������� 0, �� ������ ����� �������, ���� 1 � �� ��� ������.
#define RW PB1			// ��� ������/������.	���� ����� 0 � �� �� � ���������� ������� ����� ������, � ���� 1 � �� ����� ������ ������ �� ����������� �������.
#define RS  PB0			// ��� �����������		�� ���������� ������ (����� 1 �������� � 0) �� ������� ���������� ������� ��������, ��� ������ ������ �������� ������ ������ ������ �� ������ ������ D0 � D7, ���� �������� ������ �� ������ � ����������� ����� �� ��������� ����� RW.

#define CMD_DISPLAY_CLR 0b00000001
#define CMD_ROW_FIRST	0b10000000
#define CMD_ROW_SECOND	0b11000000


void LCD1602_Init_8pins(void);
void Send_Cmd(char comand);
void Send_Data(char data);
void Set_Pos(char cur_x, char cur_y);
void Print_Char(char c_ascii);
void Print_String(char* s_ascii);

	
#endif /* LCD1602_H_ */