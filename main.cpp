#define F_CPU 14745600UL

#include <stdlib.h>						// включает функции - itoa() atoi()
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "LCD1602.h"
#include "DS1307.h"
#include "func.h"
#include "EEPROM.h"

char itoa_temp[9];

bool time_update = false;
bool screen_update = true;
bool sync_update = true;

int counter = 0;
int temp_tested_hour = 0;
int temp_tested_minute = 0;
int temp_compared_hour = 0;
int temp_compared_minute = 0;
int temp_general;

enum button
{
	button_none = 0,
	button_increment = 1,
	button_decrement = 2,
	button_time = 3,
	button_schedule = 4
};

int button = button_none;

enum status
{
	status_normal = 0,
	status_setup_second = 1,
	status_setup_minute = 2,
	status_setup_hour = 3,
	status_scedule_sunrise_hour = 4,
	status_scedule_sunrise_minute = 5,
	status_scedule_day_hour = 6,
	status_scedule_day_minute = 7,
	status_scedule_sunset_hour = 8,
	status_scedule_sunset_minute = 9,
	status_scedule_night_hour = 10,
	status_scedule_night_minute = 11
};
	
int status = status_normal;

enum time
{
	time_second = 0,
	time_minute = 1,
	time_hour = 2, 
	time_day = 3,
	time_date = 4,
	time_month = 5,
	time_year = 6
};

char time[7] = {55, 59, 0, 0, 0, 0, 0};
char time_setup[7];


enum schedule
{
	schedule_hour = 0,
	schedule_minute = 1,
};

char schedule_sunrise[2]	= {8, 0};
char schedule_day[2]		= {10, 0};
char schedule_sunset[2]		= {18, 0};
char schedule_night[2]		= {20, 0};

	
int	schedule_realtime_i	= time[time_hour] * 60					+ time[time_minute];
int schedule_sunrise_i	= schedule_sunrise[schedule_hour] * 60	+ schedule_sunrise[schedule_minute];
int schedule_day_i		= schedule_day[schedule_hour] * 60		+ schedule_day[schedule_minute];
int schedule_sunset_i	= schedule_sunset[schedule_hour] * 60	+ schedule_sunset[schedule_minute];
int schedule_night_i	= schedule_night[schedule_hour] * 60	+ schedule_night[schedule_minute];


void Setup_INT(void);
void Setup_TIM0(void);
void Setup_TIM1(void);

int main(void)
{
	_delay_ms(50);
	Setup_INT();
	Setup_TIM0();
	Setup_TIM1();
	LCD1602_Init_8pins();
	
	DS1307_ReadTime(time);
	
	while (true)
	{
	}
}


ISR(INT0_vect)
{
	cli();

	// Опрос порта и определение того какая кнопка нажата
	int error_counter = 0;
	int success_counter = 0;
	char temp = 0;
	
	do	// Опрос порта D до тех пор пока не будет определена кнопка (типа против дребезга и некорректных значений в порте)
	{
		_delay_us(20);
		
		temp = PIND;
		temp = temp >> 4;
			
		if (temp == 0b00000111 || temp == 0b00001011 || temp == 0b00001101 || temp == 0b00001110)					// Если после 10 опросов нет значения кнопки то выход из прерывания
		{
			success_counter++;
		}
		else
		{
			error_counter++;
			success_counter--;
		}
		
		if (error_counter > 100)
		{
			button = button_none;
			sei();
			return;
		}
		
	} while (!(success_counter > 1000 || error_counter > 1000));
	
	
	switch (temp)
	{
		// 1 Нажата кнопка инкремента
		case 0b00000111:
			button = button_increment;
			
			switch (status)
			{
				case status_normal:
					// Код
					// Ничего
					break;
				
				case status_setup_hour:
					// Код
					time_setup[time_hour]++;
					
					if (time_setup[time_hour] >= 24)
					{
						time_setup[time_hour] = 0;
					}
					
					Set_Pos(6,1);
					Print_String((char*)"  ");
					Set_Pos(6,1);
					Print_String((char*)itoa(time_setup[time_hour], itoa_temp, 10));
					break;
				
				case status_setup_minute:
					// Код
					time_setup[time_minute]++;
					
					if (time_setup[time_minute] >= 60)
					{
						time_setup[time_minute] = 0;
					}
					
					Set_Pos(7,1);
					Print_String((char*)"  ");
					Set_Pos(7,1);
					Print_String((char*)itoa(time_setup[time_minute], itoa_temp, 10));
					break;
				
				case status_setup_second:
					// Код
					time_setup[time_second]++;
					
					if (time_setup[time_second] >= 60)
					{
						time_setup[time_second] = 0;
					}
					
					Set_Pos(8,1);
					Print_String((char*)"  ");
					Set_Pos(8,1);
					Print_String((char*)itoa(time_setup[time_second], itoa_temp, 10));
					break;
				
				case status_scedule_sunrise_hour:
					// Код
					temp_tested_hour		= schedule_sunrise[schedule_hour];
					temp_tested_minute		= schedule_sunrise[schedule_minute];
					temp_compared_hour		= schedule_day[schedule_hour];
					temp_compared_minute	= schedule_day[schedule_minute];
					
					temp_tested_hour++;
					
					if ((temp_tested_hour * 60 + temp_tested_minute) < (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_sunrise[schedule_hour] = temp_general / 60;
						schedule_sunrise[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) - 1;
						schedule_sunrise[schedule_hour] = temp_general / 60;
						schedule_sunrise[schedule_minute] = temp_general % 60;
					}
										
					Set_Pos(0,0);
					Print_String((char*)"rasv            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
					
				case status_scedule_sunrise_minute:
					// Код
					temp_tested_hour		= schedule_sunrise[schedule_hour];
					temp_tested_minute		= schedule_sunrise[schedule_minute];
					temp_compared_hour		= schedule_day[schedule_hour];
					temp_compared_minute	= schedule_day[schedule_minute];
					
					temp_tested_minute++;
					
					if (temp_tested_minute >= 60)
					{
						temp_tested_minute = 0;
						temp_tested_hour++;
					}
					
					if ((temp_tested_hour * 60 + temp_tested_minute) < (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_sunrise[schedule_hour] = temp_general / 60;
						schedule_sunrise[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) - 1;
						schedule_sunrise[schedule_hour] = temp_general / 60;
						schedule_sunrise[schedule_minute] = temp_general % 60;
						
					}
					
					Set_Pos(0,0);
					Print_String((char*)"rasv            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_day_hour:
					// Код
					temp_tested_hour		= schedule_day[schedule_hour];
					temp_tested_minute		= schedule_day[schedule_minute];
					temp_compared_hour		= schedule_sunset[schedule_hour];
					temp_compared_minute	= schedule_sunset[schedule_minute];
					
					temp_tested_hour++;
										
					if ((temp_tested_hour * 60 + temp_tested_minute) < (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_day[schedule_hour] = temp_general / 60;
						schedule_day[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) - 1;
						schedule_day[schedule_hour] = temp_general / 60;
						schedule_day[schedule_minute] = temp_general % 60;
					}
								
					Set_Pos(0,0);
					Print_String((char*)"den'            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_day_minute:
					// Код
					temp_tested_hour		= schedule_day[schedule_hour];
					temp_tested_minute		= schedule_day[schedule_minute];
					temp_compared_hour		= schedule_sunset[schedule_hour];
					temp_compared_minute	= schedule_sunset[schedule_minute];
					
					temp_tested_minute++;
					
					if (temp_tested_minute >= 60)
					{
						temp_tested_minute = 0;
						temp_tested_hour++;
					}
					
					if ((temp_tested_hour * 60 + temp_tested_minute) < (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_day[schedule_hour] = temp_general / 60;
						schedule_day[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) - 1;
						schedule_day[schedule_hour] = temp_general / 60;
						schedule_day[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"den'            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_sunset_hour:
					// Код
					temp_tested_hour		= schedule_sunset[schedule_hour];
					temp_tested_minute		= schedule_sunset[schedule_minute];
					temp_compared_hour		= schedule_night[schedule_hour];
					temp_compared_minute	= schedule_night[schedule_minute];
					
					temp_tested_hour++;
					
					if ((temp_tested_hour * 60 + temp_tested_minute) < (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_sunset[schedule_hour] = temp_general / 60;
						schedule_sunset[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) - 1;
						schedule_sunset[schedule_hour] = temp_general / 60;
						schedule_sunset[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"zakt            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_sunset_minute:
					// Код
					temp_tested_hour		= schedule_sunset[schedule_hour];
					temp_tested_minute		= schedule_sunset[schedule_minute];
					temp_compared_hour		= schedule_night[schedule_hour];
					temp_compared_minute	= schedule_night[schedule_minute];
					
					temp_tested_minute++;
					
					if (temp_tested_minute >= 60)
					{
						temp_tested_minute = 0;
						temp_tested_hour++;
					}
					
					if ((temp_tested_hour * 60 + temp_tested_minute) < (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_sunset[schedule_hour] = temp_general / 60;
						schedule_sunset[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) - 1;
						schedule_sunset[schedule_hour] = temp_general / 60;
						schedule_sunset[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"zakt            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_night_hour:
					// Код
					temp_tested_hour		= schedule_night[schedule_hour];
					temp_tested_minute		= schedule_night[schedule_minute];
					temp_compared_hour		= 24;
					temp_compared_minute	= 0;
					
					temp_tested_hour++;
									
					if ((temp_tested_hour * 60 + temp_tested_minute) < (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_night[schedule_hour] = temp_general / 60;
						schedule_night[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) - 1;
						schedule_night[schedule_hour] = temp_general / 60;
						schedule_night[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"noch            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;	
				
				case status_scedule_night_minute:
					// Код
					temp_tested_hour		= schedule_night[schedule_hour];
					temp_tested_minute		= schedule_night[schedule_minute];
					temp_compared_hour		= 24;
					temp_compared_minute	= 0;
					
					temp_tested_minute++;
					
					if (temp_tested_minute >= 60)
					{
						temp_tested_minute = 0;
						temp_tested_hour++;
					}
					
					if ((temp_tested_hour * 60 + temp_tested_minute) < (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_night[schedule_hour] = temp_general / 60;
						schedule_night[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) - 1;
						schedule_night[schedule_hour] = temp_general / 60;
						schedule_night[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"noch            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
									
				default:
					// Код
					break;
			}			
			break;
		
		// 2 Нажата кнопка декремента
		case 0b00001011:
			button = button_decrement;
			
			switch (status)
			{
				case status_normal:
					// Код
					// Ничего
					break;
				
				case status_setup_hour:
					// Код
					time_setup[time_hour]--;
					
					if (time_setup[time_hour] >= 24)
					{
						time_setup[time_hour] = 23;
					}
					
					Set_Pos(6,1);
					Print_String((char*)"  ");
					Set_Pos(6,1);
					Print_String((char*)itoa(time_setup[time_hour], itoa_temp, 10));
					break;
				
				case status_setup_minute:
					// Код
					time_setup[time_minute]--;
					
					if (time_setup[time_minute] >= 60)
					{
						time_setup[time_minute] = 59;
					}
					
					Set_Pos(7,1);
					Print_String((char*)"  ");
					Set_Pos(7,1);
					Print_String((char*)itoa(time_setup[time_minute], itoa_temp, 10));
					break;
				
				case status_setup_second:
					// Код
					time_setup[time_second]--;
					
					if (time_setup[time_second] >= 60)
					{
						time_setup[time_second] = 59;
					}
					
					Set_Pos(8,1);
					Print_String((char*)"  ");
					Set_Pos(8,1);
					Print_String((char*)itoa(time_setup[time_second], itoa_temp, 10));
					break;
				
				case status_scedule_sunrise_hour:
					// Код
					temp_tested_hour		= schedule_sunrise[schedule_hour];
					temp_tested_minute		= schedule_sunrise[schedule_minute];
					temp_compared_hour		= 0;
					temp_compared_minute	= 0;
					
					temp_tested_hour--;
					
					if ((temp_tested_hour * 60 + temp_tested_minute) > (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_sunrise[schedule_hour] = temp_general / 60;
						schedule_sunrise[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) + 1;
						schedule_sunrise[schedule_hour] = temp_general / 60;
						schedule_sunrise[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"rasv            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_sunrise_minute:
					// Код
				
					temp_tested_hour		= schedule_sunrise[schedule_hour];
					temp_tested_minute		= schedule_sunrise[schedule_minute];
					temp_compared_hour		= 0;
					temp_compared_minute	= 0;
					
					temp_tested_minute--;
					
					if (temp_tested_minute >= 60)
					{
						temp_tested_minute = 59;
						temp_tested_hour--;
					}
					
					if ((temp_tested_hour * 60 + temp_tested_minute) > (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_sunrise[schedule_hour] = temp_general / 60;
						schedule_sunrise[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) + 1;
						schedule_sunrise[schedule_hour] = temp_general / 60;
						schedule_sunrise[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"rasv            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_day_hour:
					// Код
					temp_tested_hour		= schedule_day[schedule_hour];
					temp_tested_minute		= schedule_day[schedule_minute];
					temp_compared_hour		= schedule_sunrise[schedule_hour];
					temp_compared_minute	= schedule_sunrise[schedule_minute];
					
					temp_tested_hour--;
					
					if ((temp_tested_hour * 60 + temp_tested_minute) > (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_day[schedule_hour] = temp_general / 60;
						schedule_day[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) + 1;
						schedule_day[schedule_hour] = temp_general / 60;
						schedule_day[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"den'            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_day_minute:
					// Код
					temp_tested_hour		= schedule_day[schedule_hour];
					temp_tested_minute		= schedule_day[schedule_minute];
					temp_compared_hour		= schedule_sunrise[schedule_hour];
					temp_compared_minute	= schedule_sunrise[schedule_minute];
					
					temp_tested_minute--;
					
					if (temp_tested_minute >= 60)
					{
						temp_tested_minute = 59;
						temp_tested_hour--;
					}
					
					if ((temp_tested_hour * 60 + temp_tested_minute) > (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_day[schedule_hour] = temp_general / 60;
						schedule_day[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) + 1;
						schedule_day[schedule_hour] = temp_general / 60;
						schedule_day[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"den'            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_sunset_hour:
					// Код
					temp_tested_hour		= schedule_sunset[schedule_hour];
					temp_tested_minute		= schedule_sunset[schedule_minute];
					temp_compared_hour		= schedule_day[schedule_hour];
					temp_compared_minute	= schedule_day[schedule_minute];
					
					temp_tested_hour--;
					
					if ((temp_tested_hour * 60 + temp_tested_minute) > (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_sunset[schedule_hour] = temp_general / 60;
						schedule_sunset[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) + 1;
						schedule_sunset[schedule_hour] = temp_general / 60;
						schedule_sunset[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"zakt            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					
					break;
				
				case status_scedule_sunset_minute:
					// Код
					temp_tested_hour		= schedule_sunset[schedule_hour];
					temp_tested_minute		= schedule_sunset[schedule_minute];
					temp_compared_hour		= schedule_day[schedule_hour];
					temp_compared_minute	= schedule_day[schedule_minute];
					
					temp_tested_minute--;
					
					if (temp_tested_minute >= 60)
					{
						temp_tested_minute = 59;
						temp_tested_hour--;
					}
					
					if ((temp_tested_hour * 60 + temp_tested_minute) > (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_sunset[schedule_hour] = temp_general / 60;
						schedule_sunset[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) + 1;
						schedule_sunset[schedule_hour] = temp_general / 60;
						schedule_sunset[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"zakt            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_night_hour:
					// Код
					temp_tested_hour		= schedule_night[schedule_hour];
					temp_tested_minute		= schedule_night[schedule_minute];
					temp_compared_hour		= schedule_sunset[schedule_hour];
					temp_compared_minute	= schedule_sunset[schedule_minute];
					
					temp_tested_hour--;
					
					if ((temp_tested_hour * 60 + temp_tested_minute) > (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_night[schedule_hour] = temp_general / 60;
						schedule_night[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) + 1;
						schedule_night[schedule_hour] = temp_general / 60;
						schedule_night[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"noch            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_night_minute:
					// Код
					temp_tested_hour		= schedule_night[schedule_hour];
					temp_tested_minute		= schedule_night[schedule_minute];
					temp_compared_hour		= schedule_sunset[schedule_hour];
					temp_compared_minute	= schedule_sunset[schedule_minute];
					
					temp_tested_minute--;
					
					if (temp_tested_minute >= 60)
					{
						temp_tested_minute = 59;
						temp_tested_hour--;
					}
					
					if ((temp_tested_hour * 60 + temp_tested_minute) > (temp_compared_hour * 60 + temp_compared_minute))
					{
						temp_general = temp_tested_hour * 60 + temp_tested_minute;
						schedule_night[schedule_hour] = temp_general / 60;
						schedule_night[schedule_minute] = temp_general % 60;
					}
					else
					{
						temp_general = (temp_compared_hour * 60 + temp_compared_minute) + 1;
						schedule_night[schedule_hour] = temp_general / 60;
						schedule_night[schedule_minute] = temp_general % 60;
					}
					
					Set_Pos(0,0);
					Print_String((char*)"noch            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				default:
					// Код
					break;
			}
			break;
		
		
		// 3 Нажата кнопка настройки времени
		case 0b00001101:
			button = button_time;
			
			switch (status)
			{
				case status_normal:
					status = status_setup_hour;
					
					for (int i = 0; i < 7; i++)
					{
						time_setup[i] = time[i];
					}
					
					Set_Pos(0,0);
					Print_String((char*)"nastroika casov ");
					Set_Pos(0,1);
					Print_String((char*)"chasi           ");
					
					Set_Pos(6,1);
					Print_String((char*)itoa(time_setup[time_hour], itoa_temp, 10));
					
					break;
				
				case status_setup_hour:
					status = status_setup_minute;
					
					Set_Pos(0,0);
					Print_String((char*)"nastroika casov ");
					Set_Pos(0,1);
					Print_String((char*)"minuti          ");
					
					Set_Pos(7,1);
					Print_String((char*)itoa(time_setup[time_minute], itoa_temp, 10));
					
					break;
				
				case status_setup_minute:
					status = status_setup_second;
					
					Set_Pos(0,0);
					Print_String((char*)"nastroika casov ");
					Set_Pos(0,1);
					Print_String((char*)"secyndi         ");
					
					Set_Pos(8,1);
					Print_String((char*)itoa(time_setup[time_second], itoa_temp, 10));
					
					break;
				
				case status_setup_second:
					status = status_normal;
					
					for (int i = 0; i < 7; i++)
					{
						time[i] = time_setup[i];
					}
					
					DS1307_WriteTime(time);
					
					Set_Pos(0,0);
					Print_String((char*)"Vrema:          ");
					Set_Pos(7,0);
					Print_Time(time);
					
					Set_Pos(0,1);
					Print_String((char*)"                ");
					
					break;
				
				case status_scedule_sunrise_hour:
					// Код
					// Ничего
					break;
				
				case status_scedule_sunrise_minute:
					// Код
					// Ничего
					break;
				
				case status_scedule_day_hour:
					// Код
					// Ничего
					break;
				
				case status_scedule_day_minute:
					// Код
					// Ничего
					break;
				
				case status_scedule_sunset_hour:
					// Код
					// Ничего
					break;
				
				case status_scedule_sunset_minute:
					// Код
					// Ничего
					break;
				
				case status_scedule_night_hour:
					// Код
					// Ничего
					break;
				
				case status_scedule_night_minute:
					// Код
					// Ничего
					break;
				
				default:
					status = status_normal;
					break;
			}	
			break;
		
		
		// 4 Нажата кнопка настройки расписания
		case 0b00001110:
			button = button_schedule;
			
			switch (status)
			{
				case status_normal:
					// Код
					status = status_scedule_sunrise_hour;
					
					Set_Pos(0,0);
					Print_String((char*)"rasv            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_setup_hour:
					// Код
					// Ничего
					break;
				
				case status_setup_minute:
					// Код
					// Ничего
					break;
				
				case status_setup_second:
					// Код
					// Ничего
					break;
				
				case status_scedule_sunrise_hour:
					// Код
					status = status_scedule_sunrise_minute;
					
					Set_Pos(0,0);
					Print_String((char*)"rasv            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_sunrise_minute:
					// Код
					status = status_scedule_day_hour;
					
					Set_Pos(0,0);
					Print_String((char*)"den'            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_day_hour:
					// Код
					status = status_scedule_day_minute;
					
					Set_Pos(0,0);
					Print_String((char*)"den'            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_day_minute:
					// Код
					status = status_scedule_sunset_hour;
					
					Set_Pos(0,0);
					Print_String((char*)"zakt            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_sunset_hour:
					// Код
					status = status_scedule_sunset_minute;
					
					Set_Pos(0,0);
					Print_String((char*)"zakt            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_sunset_minute:
					// Код
					status = status_scedule_night_hour;
					
					Set_Pos(0,0);
					Print_String((char*)"noch            ");
					Set_Pos(0,1);
					Print_String((char*)"chas            ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_night_hour:
					// Код
					status = status_scedule_night_minute;
					
					Set_Pos(0,0);
					Print_String((char*)"noch            ");
					Set_Pos(0,1);
					Print_String((char*)"min             ");
					
					Set_Pos(5,0);
					Print_Schedule(schedule_sunrise);
					Set_Pos(11,0);
					Print_Schedule(schedule_day);
					Set_Pos(5,1);
					Print_Schedule(schedule_sunset);
					Set_Pos(11,1);
					Print_Schedule(schedule_night);
					break;
				
				case status_scedule_night_minute:
					// Код
					status = status_normal;
					
					Set_Pos(0,0);
					Print_String((char*)"Vrema:          ");
					Set_Pos(7,0);
					Print_Time(time);
					
					Set_Pos(0,1);
					Print_String((char*)"                ");
					break;
				
				default:
					status = status_normal;
					break;
			}
			break;
		
		// 0 Ниче не нажато
		default:
			button = button_none;
			
			break;
	}

	sei();
}


ISR(TIMER1_COMPA_vect)
{
	cli();

	time[time_second]++;

	if (time[time_second] >= 60)
	{
		time[time_second] = 0;
		time[time_minute]++;
		
		if (time[time_minute] >= 60)
		{
			time[time_minute] = 0;
			time[time_hour]++;
			
			if (time[time_hour] >= 24)
			{
				time[time_second] = 0;
				time[time_minute] = 0;
				time[time_hour] = 0;
			}
		}
	}
	
	// Отображение часов на экране
	if (status == status_normal)
	{
		Set_Pos(0,0);
		Print_String((char*)"Vrema:          ");
		Set_Pos(7,0);
		Print_Time(time);
		
		schedule_realtime_i	= time[time_hour] * 60					+ time[time_minute];
		schedule_sunrise_i	= schedule_sunrise[schedule_hour] * 60	+ schedule_sunrise[schedule_minute];
		schedule_day_i		= schedule_day[schedule_hour] * 60		+ schedule_day[schedule_minute];
		schedule_sunset_i	= schedule_sunset[schedule_hour] * 60	+ schedule_sunset[schedule_minute];
		schedule_night_i	= schedule_night[schedule_hour] * 60	+ schedule_night[schedule_minute];
			
					
		if (schedule_realtime_i < schedule_sunrise_i)
		{
			OCR0 = 0;
			
			Set_Pos(0,1);
			Print_String((char*)"noch         ");
			Set_Pos(5,1);
			Print_String((char*)"0.00%");
		}
		else if (schedule_realtime_i < schedule_day_i)
		{
			OCR0 = (((float)((schedule_realtime_i + time[time_second] / 60.0) - schedule_sunrise_i))/((float)(schedule_day_i - schedule_sunrise_i))) * 255.0;
			
			Set_Pos(0,1);
			Print_String((char*)"rasv         ");
			Set_Pos(5,1);
			
			if (OCR0 / 2.55 < 10.0)
			{
				Print_String((char*)"0");
			}
			
			Print_String((char*)ftoa((OCR0 / 2.55), itoa_temp, 2));
			Print_String((char*)"%");
		}
		else if (schedule_realtime_i < schedule_sunset_i)
		{
			OCR0 = 255; 
			
			Set_Pos(0,1);
			Print_String((char*)"den'         ");
			Set_Pos(5,1);
			Print_String((char*)ftoa((OCR0 / 2.55), itoa_temp, 2));
			Print_String((char*)"%");
		}
		else if (schedule_realtime_i < schedule_night_i)
		{

			OCR0 = (1.0 - (((float)((schedule_realtime_i + time[time_second] / 60.0) - schedule_sunset_i))/((float)(schedule_night_i - schedule_sunset_i)))) * 255.0;

			
			Set_Pos(0,1);
			Print_String((char*)"zakt         ");
			Set_Pos(5,1);
			
			if (OCR0 / 2.55 < 10.0)
			{
				Print_String((char*)"0");
			}
			
			Print_String((char*)ftoa((OCR0 / 2.55), itoa_temp, 2));
			Print_String((char*)"%");
		}
		else
		{
			OCR0 = 0;
			
			Set_Pos(0,1);
			Print_String((char*)"noch         ");
			Set_Pos(5,1);
			Print_String((char*)"0.00%");
		}
	}
	
	// Обновление времени в начале часа
	if (time[time_minute] == 0 && sync_update == true)
	{
		sync_update = false;
		DS1307_ReadTime(time);
	}
	
	if (time[time_minute] == 59 && sync_update == false)
	{
		sync_update = true;
	}
	
	sei();
}


void Setup_INT(void)
{
	DDRB |= 0;
	PORTB |= 0b11110000;
	
	
	GICR |= (0 << INT1)|(1 << INT0)|(0 << INT2);						// General Interrupt Control Register - Установка битов INT1, INT0 или INT2 разрешает прерывания при возникновении события на соответствующем выводе микроконтроллера AVR, а сброс — запрещает.
	MCUCR |= (1 << ISC11)|(0 << ISC10)|(1 << ISC01)|(0 << ISC00);		// 10	- Перывание по спадающему фронту INT0, INT1
	MCUCSR |= (0 << ISC2);												// 0	- Перывание по спадающему фронту INT2
	
	sei();
}

void Setup_TIM0(void)
{
	//////////// Таймер 0 (8 бит) PWM ////////////
	OCR0  |= 0;			// Скважность - Значение сравнения f = fcpu/(N*256) = 7372800/256 = 28800 || = 14745600/256 = 57600 | 248 -> 97.1% | 249 -> 97.5% | 250 -> 97.9% | 251 -> 98.3% | 252 -> 98.7%  | *по расчету надо 97,5 - 98,6%
	TCCR0 |= (0 << FOC0)|(1  << WGM01)|(1 << WGM00)|(1 << COM01)|(0 << COM00)|(0 << CS02)|(0 << CS01)|(1 << CS00);	// WGM - fast PWM, COM - clear on compare, CS - прескелер, FOC - ?
	
	DDRB |= 0b00001000;		// Вывод ШИМ - PB3(OC0)
	
	sei();
}

void Setup_TIM1(void)
{
	//////////// Таймер 1 (16 бит) Часы ////////////
	OCR1A   = 14398;	// Запись значения прерывания CTC производится до инициализации таймера
	TCCR1A |= (0 << COM1A1)|(0 << COM1A0)|(0 << COM1B1)|(0 << COM1B0)|(0 << FOC1A)|(0 << FOC1B)|(0 << WGM11)|(0 << WGM10);	// COM - порты, FOC - ?, WGM - режим CTC, CS - прескелер
	TCCR1B |= (0 <<  ICNC1)|(0 <<  ICES1)|(0 <<  WGM13)|(1 <<  WGM12)|(1 <<  CS12)|(0 <<  CS11)|(1 <<  CS10);				// IC - настройки пина ICP1 (PD6)
	TIMSK  |= (0 << TICIE1)|(1 << OCIE1A)|(0 << OCIE1B)|(0 << TOIE1);														// OCIE1A - Прерывание по совпадению разрешено
	
	sei();
}
