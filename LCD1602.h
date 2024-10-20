#ifndef LCD1602_H_
#define LCD1602_H_

#define COMAND_PORT PORTB
#define COMAND_DDR	DDRB
#define DATA_PORT	PORTA
#define DATA_DDR	DDRA
#define DATA_PIN	PINA

#define E PB2			// ѕин команда/данные.	≈сли мы подадим на данную ножку логический 0, то значит будет команда, если 1 Ч то это данные.
#define RW PB1			// ѕин запись/чтение.	≈сли будет 0 Ч то мы в контроллер диспле€ будем писать, а если 1 Ч то будем читать данные из контроллера диспле€.
#define RS  PB0			// ѕин тактировни€		ѕо спадающему фронту (когда 1 мен€етс€ в 0) на которой контроллер диспле€ понимает, что именно сейчас наступил момент чтени€ данных на ножках данных D0 Ч D7, либо передачи данных из модул€ в зависимости также от состо€ни€ ножки RW.

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