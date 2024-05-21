#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

enum commaFlags
{
   e_ID = 0,
   e_LENGTH,
   e_DATA,
   e_HTTP200OK,
   e_CONTENT_LENGTH,
   e_TOKEN_AND_DATE,
   e_TELEMETRY,
   e_FRAGMENT_FIRMWARE,
   e_FIRMWARE
};

enum dataLength
{
   e_FRAGMENT_FIRMWARE_LENGTH = 1,
   e_TELEMETRY_LEGNTH = 2,
   e_TOKEN_AND_DATE_LENGTH = 40,
   e_FIRMWAE_LENGTH = 200
};

#define RTC_SECOND 	0
#define RTC_MINUTE 	1
#define RTC_HOUR 		2
#define RTC_DAY 		3
#define RTC_DEYS 		4
#define RTC_MONTH 	5
#define RTC_YEAR 		6
#define RTC_CONTROLS 7

#define FAILS_UNTIL_RESTART 10

typedef struct
{
   uint32_t firmware_downloaded : 1;
   uint32_t dummy : 31;
} hc_bits;


typedef union
{
   hc_bits bits;
   uint32_t flagsHolder;
} hc_flags;

typedef struct
{
   hc_flags flags;
} hCommunication;


hCommunication communication;

// /* Optional, might already be in your project */
// vu32 dataParse_flags;
// #define F_FIRMWARE_DATA_PACKET_ADD 				0x00000001

// #define SET_FIRMWARE_DATA_PACKET_ADD 					dataParse_flags |= F_FIRMWARE_DATA_PACKET_ADD
// #define RESET_FIRMWARE_DATA_PACKET_ADD 				dataParse_flags &= ~F_FIRMWARE_DATA_PACKET_ADD
// #define FIRMWARE_DATA_PACKET_ADD 						(dataParse_flags & F_FIRMWARE_DATA_PACKET_ADD)

void ProcessESPData(USART_TypeDef* USARTx);
void SendDataToESP(USART_TypeDef *USARTx);
void ProcessCBData(USART_TypeDef* USARTx);
void SendDataToCB(USART_TypeDef* USARTx);

vu8 uart_rec_byte;

#endif