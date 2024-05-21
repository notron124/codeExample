#include "stm32f10x.h"
#include "debug.h"

/* This is mostly for debug puposes.
   To make it work:
   1) Initialise dUsart->instance with USART, that you are using
   2) Put SendData_IT into USART interrupt handler and pass pointer to dUsart
   3) Make char buffer in which you will store all data, that you want to send wia USART
   4) Make variable, that will track number of character to print
   5) Use sprintf to store data into your buffer (check manual before using it)
   6) Use SendData_USART to start sending data. Pass pointer to dUsart, pointer to buffer and data size (number of characters to print) to it.
   
   Example:
   char dataBuff[128];

   uint32_t size;
   size = sprintf(dataBuff, "%d	%d\n", (int)someData1, (int)someData2);

   SendData_USART(&dUsart, dataBuff, size);
*/

hUART_Status SendData_USART(hUSART *hUSART, char *pData, __IO uint32_t dataSize)
{
	if (hUSART->state == hUSART_IDLE)
	{
		hUSART->state = hUSART_BUSY_TX;
		hUSART->pData = pData;
		hUSART->TxDataCntr = dataSize;
		USART_ITConfig(hUSART->instance, USART_IT_TXE, ENABLE);

		return hUSART_OK;
	}
	else
		return hUSART_BUSY;
}

hUART_Status SendData_IT(hUSART *hUSART)
{
   if (hUSART->state == hUSART_BUSY_TX)
   {
      hUSART->instance->DR = (uint8_t)(*hUSART->pData++ & (uint8_t)0x00FF);

      if (--hUSART->TxDataCntr == 0U)
      {
         USART_ITConfig(hUSART->instance, USART_IT_TXE, DISABLE);

         hUSART->state = hUSART_IDLE;
      }

      return hUSART_OK;
   }
   else
   {
      return hUSART_BUSY;
   }
}
