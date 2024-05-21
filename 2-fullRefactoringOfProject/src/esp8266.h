
#ifndef ESP8266_H_
#define ESP8266_H_


//#define COUNT_OF_ATTEMP_TO_CONNECT 10
#define USART_WiFi	USART1 /*Put USART that connected to ESP8266 here*/

#define WIFI_PARAMS 100
#define VAL_TOKEN_LENGTH 40

typedef enum
{
	WIFI_DISCONNECTED,
	WIFI_CONNECTED,
	WIFI_TCP_OPENED
} WiFiConnectStatus; // Показывает активно ли WiFi и TCP соединения

typedef enum
{
   WSTANBY = 0,
   WRUN,
   WERROR,
   WOK,
   WBUFOVERFLOW,
   WTIMEOUT
} ATcommandStatus;

typedef enum
{
	ATCMD = 0,		 // Работа с АТ-командами
	UART2TCP_SEND,	 // Отправка АТ-команды
	UART2TCP_STANBY // Ожидание данных через TCP-соединение
} WiFiConnectionState;

__IO u8 fResp;
vu8 responseCount;
__IO char Response[128];

char currentIP[20];

__IO WiFiConnectionState connectionState;

void ESP8266_SendData(void);
void ESP8266_SendCommand(USART_TypeDef *USARTx, const char *buf, u16 timeout_ms);
void ESP8266_GetIP(USART_TypeDef *USARTx, char *ip_addr, u16 timeout_ms);
void PeriodicRequestsToServer(void);
void HandleWifi(void);
void wifi_init(void);
u8 DecToChar(char *tc, u32 num);
void DelaySendProgToServer(void);
void PeriodicRequestsToServer(void);
void InitESPVariables(void);

union wifi_params
{
   struct
   {
      u8 wifi_flags;
      u8 ssid[33];
      u8 pswd[64];
      u8 dummy[2];
   } w;
   u8 ch[WIFI_PARAMS];
   u16 chw[WIFI_PARAMS / 2];
   u32 chdw[WIFI_PARAMS / 4];
};

#define INTERVAL_SEND_TELEMETRY  15
#define IDOFIPD_BUFF_LENGTH      20

extern vu32 wifi_work_flags;
#define F_EXT_WIFI_CONNECTED			0x0001
#define F_WIFI_MORE_MESSAGE			0x0002
#define F_BUSY_WIFI_UART				0x0004
#define F_WIFI_PRE_MORE_MESSAGE		0x0008

#define F_SSL_CONNECTED					0x0010
#define F_SUB_SERVER						0x0020
#define F_NEED_RECEIVE_TOKEN			0x0040
#define F_SEND_TO_SERVER_POWER_ON	0x0080

#define F_TX_ERROR_TO_SERVER			0x0100
#define F_REQUEST_FIRMWARE_LENGTH 	0x0200
#define F_REQUEST_FIRMWARE_DATA		0x0400
#define F_SEND_DATA_TO_SERVER			0x0800

#define F_FRAGMENT_FIRMWARE			0x1000
#define F_RUN_PROG_TO_SERVER        0x2000
#define F_SERVER_IPD						0x4000
#define F_RX_WIFI_ACTIVITY 			0x8000

#define F_SEND_DATA_TO_TABLET			0x10000
#define F_REQUEST_IP						0x20000
#define F_DELAY_SEND_TO_SERVER		0x40000

#define SET_EXT_WIFI_CONNECTED 	wifi_work_flags |= F_EXT_WIFI_CONNECTED
#define RESET_EXT_WIFI_CONNECTED wifi_work_flags &= ~F_EXT_WIFI_CONNECTED
#define EXT_WIFI_CONNECTED       (wifi_work_flags & F_EXT_WIFI_CONNECTED)

#define SET_WIFI_MORE_MESSAGE    wifi_work_flags |= F_WIFI_MORE_MESSAGE
#define RESET_WIFI_MORE_MESSAGE  wifi_work_flags &= ~F_WIFI_MORE_MESSAGE
#define WIFI_MORE_MESSAGE        (wifi_work_flags & F_WIFI_MORE_MESSAGE)

#define SET_WIFI_PRE_MORE_MESSAGE    wifi_work_flags |= F_WIFI_PRE_MORE_MESSAGE
#define RESET_WIFI_PRE_MORE_MESSAGE  wifi_work_flags &= ~F_WIFI_PRE_MORE_MESSAGE
#define WIFI_PRE_MORE_MESSAGE        (wifi_work_flags & F_WIFI_PRE_MORE_MESSAGE)

#define SET_BUSY_WIFI_UART       wifi_work_flags |= F_BUSY_WIFI_UART
#define RESET_BUSY_WIFI_UART     wifi_work_flags &= ~F_BUSY_WIFI_UART
#define BUSY_WIFI_UART           (wifi_work_flags & F_BUSY_WIFI_UART)

#define SET_REQUEST_IP           wifi_work_flags |= F_REQUEST_IP
#define RESET_REQUEST_IP         wifi_work_flags &= ~F_REQUEST_IP
#define REQUEST_IP               (wifi_work_flags & F_REQUEST_IP)

#define SET_SSL_CONNECTED        wifi_work_flags |= F_SSL_CONNECTED
#define RESET_SSL_CONNECTED      wifi_work_flags &= ~F_SSL_CONNECTED
#define SSL_CONNECTED            (wifi_work_flags & F_SSL_CONNECTED)

#define SET_SUB_SERVER           wifi_work_flags |= F_SUB_SERVER
#define RESET_SUB_SERVER         wifi_work_flags &= ~F_SUB_SERVER
#define SUB_SERVER               (wifi_work_flags & F_SUB_SERVER)

#define SET_NEED_RECEIVE_TOKEN   	wifi_work_flags |= F_NEED_RECEIVE_TOKEN
#define RESET_NEED_RECEIVE_TOKEN 	wifi_work_flags &= ~F_NEED_RECEIVE_TOKEN
#define NEED_RECEIVE_TOKEN       	(wifi_work_flags & F_NEED_RECEIVE_TOKEN)

#define SET_SEND_TO_SERVER_POWER_ON    wifi_work_flags |= F_SEND_TO_SERVER_POWER_ON
#define RESET_SEND_TO_SERVER_POWER_ON  wifi_work_flags &= ~F_SEND_TO_SERVER_POWER_ON
#define SEND_TO_SERVER_POWER_ON        (wifi_work_flags & F_SEND_TO_SERVER_POWER_ON)

#define SET_TX_ERROR_TO_SERVER         wifi_work_flags |= F_TX_ERROR_TO_SERVER
#define RESET_TX_ERROR_TO_SERVER       wifi_work_flags &= ~F_TX_ERROR_TO_SERVER
#define TX_ERROR_TO_SERVER             (wifi_work_flags & F_TX_ERROR_TO_SERVER)

#define SET_REQUEST_FIRMWARE_LENGTH    wifi_work_flags |= F_REQUEST_FIRMWARE_LENGTH
#define RESET_REQUEST_FIRMWARE_LENGTH  wifi_work_flags &= ~F_REQUEST_FIRMWARE_LENGTH
#define REQUEST_FIRMWARE_LENGTH        (wifi_work_flags & F_REQUEST_FIRMWARE_LENGTH)

#define SET_REQUEST_FIRMWARE_DATA      wifi_work_flags |= F_REQUEST_FIRMWARE_DATA
#define RESET_REQUEST_FIRMWARE_DATA    wifi_work_flags &= ~F_REQUEST_FIRMWARE_DATA
#define REQUEST_FIRMWARE_DATA          (wifi_work_flags & F_REQUEST_FIRMWARE_DATA)

#define SET_SEND_DATA_TO_SERVER        wifi_work_flags |= F_SEND_DATA_TO_SERVER
#define RESET_SEND_DATA_TO_SERVER      wifi_work_flags &= ~F_SEND_DATA_TO_SERVER
#define SEND_DATA_TO_SERVER            (wifi_work_flags & F_SEND_DATA_TO_SERVER)

#define SET_FRAGMENT_FIRMWARE          wifi_work_flags |= F_FRAGMENT_FIRMWARE
#define RESET_FRAGMENT_FIRMWARE        wifi_work_flags &= ~F_FRAGMENT_FIRMWARE
#define FRAGMENT_FIRMWARE              (wifi_work_flags & F_FRAGMENT_FIRMWARE)

#define SET_RUN_PROG_TO_SERVER         wifi_work_flags |= F_RUN_PROG_TO_SERVER
#define RESET_RUN_PROG_TO_SERVER       wifi_work_flags &= ~F_RUN_PROG_TO_SERVER
#define RUN_PROG_TO_SERVER             (wifi_work_flags & F_RUN_PROG_TO_SERVER)

#define SET_SERVER_IPD			wifi_work_flags |= F_SERVER_IPD
#define RESET_SERVER_IPD		wifi_work_flags &= ~F_SERVER_IPD
#define SERVER_IPD				(wifi_work_flags & F_SERVER_IPD)

#define SET_RX_WIFI_ACTIVITY 			wifi_work_flags |= F_RX_WIFI_ACTIVITY
#define RESET_RX_WIFI_ACTIVITY 		wifi_work_flags &= ~F_RX_WIFI_ACTIVITY
#define RX_WIFI_ACTIVITY 				(wifi_work_flags & F_RX_WIFI_ACTIVITY)

#define SET_SEND_DATA_TO_TABLET  		wifi_work_flags |= F_SEND_DATA_TO_TABLET
#define RESET_SEND_DATA_TO_TABLET		wifi_work_flags &= ~F_SEND_DATA_TO_TABLET
#define SEND_DATA_TO_TABLET			(wifi_work_flags & F_SEND_DATA_TO_TABLET)

#define SET_DELAY_SEND_TO_SERVER  		wifi_work_flags |= F_DELAY_SEND_TO_SERVER
#define RESET_DELAY_SEND_TO_SERVER		wifi_work_flags &= ~F_DELAY_SEND_TO_SERVER
#define DELAY_SEND_TO_SERVER			(wifi_work_flags & F_DELAY_SEND_TO_SERVER)

extern u8 wifi_senddata_zone;
extern u8 wifi_setup_zone;
extern u8 wifi_getip_zone;
extern u8 wifi_gslp_zone;
extern vu16 cntr_timeout;
extern u16 wifi_timeout;
extern vu8 IDofIPD[];
extern vu8 cntr_RX_IPD, cntr_TX_IPD;
extern vu32 cntr_firmware_block;
extern u32 cntr_delay_ddos;
extern vu8 firmware_device_type;
extern vu8 token40[];
extern union wifi_params wifi_param;

/* For selecting station and entering password via pult*/
extern u8 wifi_sta_select_mode;
extern vu8 WiFiSTAs[10][32];
extern vu32 countCWLAP, cwlap_mode;
extern vu32 count_name_sta_wifi, current_name_sta_wifi, count_power_sta_wifi;
extern u8 WiFiSetupInfraPassword[64];
extern u32 wifi_pswd_selected_symbol_table, cntr_wifi_pswd_symbol;
extern u8 offset_wifi_password;
extern u8 stage;

extern u32 serial_number;

/* Delay before sending current porgram to server */
u8 cntr_delay_send_to_server;

#endif /* ESP8266_H_ */
