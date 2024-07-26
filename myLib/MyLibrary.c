/*
 * MyLibrary.c
 *
 *  Created on: Jul 4, 2024
 *      Author: user
 */
#include "main.h"
#include <stdio.h>
#include "Myhead.h"

extern UART_HandleTypeDef huart2;

myBuf buf;

myCMDSET myCmd[] =      //NULL Pointer = 0x0000 0000 (adress value)
{
   {"LED", 2},
   {"MOTOR", 2},
   {NULL, 0}
};

int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart2, &ch, 1, 10);
	return ch;
}

int __io_getchar(void)
{
	char ch;
	while(HAL_UART_Receive(&huart2, &ch, 1, 10) != HAL_OK);
	HAL_UART_Transmit(&huart2, &ch, 1, 10); // echo
	if(ch == '\r') HAL_UART_Transmit(&huart2, "\n", 1, 10);
	return ch;
}

void CLS()
{
	printf("\033[2J\033[0;0H\n");
}

void Cursor(int OnOff)
{
	if(OnOff) printf("\033[?25h\n");
	else printf("\033[?25l\n");
}

void Wait()
{
	while(HAL_GPIO_ReadPin(B1_GPIO_Port,B1_Pin) != 0);
}

void ProgramStart(char *name)
{
	//printf("\033[2J\033[1;1H\n"); // clear screen
	CLS();
	printf("Program(%s) ready... Press Blue button to start\r\n", name);
	Wait(); //while(HAL_GPIO_ReadPin(B1_GPIO_Port,B1_Pin) != 0); // B1 for start
}

void Memdump(int n)
{
	int max_row = 20;
	int max_col = 16 / n;
	char str[10];
	sprintf(str,"%%0%dx ", n*2);

	for(int i = 0; i < max_row; i++)
	  {
		  for(int j = 0; j < max_col; j++)
	  	  {
			  unsigned int v =
					  (n == 1) ? buf.v0[(i * 16) + j] :
					  (n == 2) ? buf.v1[(i * 8) + j] :
					  (n == 4) ? buf.v2[(i * 4) + j] : -1;
			  printf(str, v);
	  		  if(j == ((8 / n) - 1)) printf("| ");
	  	  }
	  	  printf("\r\n");
	  }
}

int head = 0, tail = 0;
int GetBuffer(char *b) // b : char array pointer
{
	int len = 0;
	char* s = &buf;
	tail = MAX_BUF - huart2.hdmarx->Instance->NDTR;
	if(tail > head)
	{
		memcpy(b, s + head, tail - head);
		len = tail - head;
	}
	else if(tail < head)
	{
		memcpy(b, s + head, MAX_BUF - head);
		memcpy(b + MAX_BUF - head, s, tail);
		len = MAX_BUF + tail - head;
	}
	else
	{
		len = 0;
	}
	*(b + len) = 0;
	head = tail;
	return len;
}

