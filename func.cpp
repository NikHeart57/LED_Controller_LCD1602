#define F_CPU 14745600UL

#include <stdlib.h>						// включает функции - itoa() atoi()
#include <avr/interrupt.h>
#include <math.h>
#include "func.h"
#include "LCD1602.h"


void Print_Time(char time[])
{	
	char itoa_temp[9];
	
	for (int i = 0; i < 3; i++)
	{
		if (time[2 - i] < 10)
		{
			Print_String((char*)"0");
		}

		Print_String((char*)itoa(time[2 - i], itoa_temp, 10));
		
		if (i == 0 || i == 1)
		{
			Print_String((char*)":");
		}
	}
}

void Print_Schedule(char time[])
{
	char itoa_temp[9];
	
	for (int i = 0; i < 2; i++)
	{
		if (time[i] < 10)
		{
			Print_String((char*)"0");
		}

		Print_String((char*)itoa(time[i], itoa_temp, 10));
		
		if (i == 0)
		{
			Print_String((char*)":");
		}
	}
}




// Reverses a string 'str' of length 'len'
void reverse(char* str, int len)
{
	int i = 0, j = len - 1, temp;
	while (i < j) {
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++;
		j--;
	}
}

// Converts a given integer x to string str[].
// d is the number of digits required in the output.
// If d is more than the number of digits in x,
// then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
	int i = 0;
	while (x) {
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}
	
	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
	str[i++] = '0';
	
	reverse(str, i);
	str[i] = '\0';
	return i;
}

// Converts a floating-point/double number to a string.
char* ftoa(float n, char* res, int afterpoint)
{
	// Extract integer part
	int ipart = (int)n;
	
	// Extract floating part
	float fpart = n - (float)ipart;
	
	// convert integer part to string
	int i = intToStr(ipart, res, 0);
	
	// check for display option after point
	if (afterpoint != 0) {
		res[i] = '.'; // add dot
		
		// Get the value of fraction part upto given no.
		// of points after dot. The third parameter
		// is needed to handle cases like 233.007
		fpart = fpart * pow(10, afterpoint);
		
		intToStr((int)fpart, res + i + 1, afterpoint);
	}
	
	return res;
}
