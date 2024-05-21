#ifndef DEBUG_H_
#define DEBUG_H_

typedef enum
{
   hUSART_IDLE = 0,
   hUSART_BUSY_TX,
   hUSART_BUSY_RX,
} hUART_States;

typedef enum
{
   hUSART_OK = 0,
   hUSART_BUSY
} hUART_Status;

typedef struct
{
   USART_TypeDef *instance;
   __IO hUART_States state;
   __IO hUART_Status status;
   __IO uint32_t TxDataCntr;
   char *pData;
} hUSART;

hUART_Status SendData_IT(hUSART *hUSART);
hUART_Status SendData_USART(hUSART *hUSART, char *pData, __IO uint32_t dataSize);

hUSART dUsart;

uint32_t rawData;

#endif
