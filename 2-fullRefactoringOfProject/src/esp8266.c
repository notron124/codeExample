#ifdef STM32L1XX_MD
#include "stm32l1xx.h" // Here you should put your main microcontroller header, this header for VS code to understand u8, u16 .. etc.
#else
#include "stm32f10x.h" // Here you should put your main microcontroller header, this header for VS code to understand u8, u16 .. etc.
#endif
#include "packet.h"
#include "esp8266.h"
#include "flash.h"
#include "communication.h"

/*User includes*/
#include "glob.h"
#include "programs.h"
#include "def.h"
/***************/

/*User variables*/
extern union val_programm programm;
extern union prg_factor factor;
/****************/

void TransmitByte2(u8 byte);
void USART1_Configuration(u32 speed);

vu8 time_restart;
vu32 wifi_work_flags;
u8 wifi_senddata_zone;
u8 wifi_setup_zone;
u8 wifi_getip_zone;
u8 wifi_gslp_zone;
vu16 cntr_timeout;
u16 wifi_timeout;

/*  */
vu8 IDofIPD[IDOFIPD_BUFF_LENGTH];
vu8 cntr_RX_IPD, cntr_TX_IPD;

vu32 cntr_firmware_block;
u32 cntr_delay_ddos;
vu8 firmware_device_type;
char post_buffr[500];
vu8 token40[40];
union wifi_params wifi_param;

/* For selecting station via pult*/
u8 wifi_sta_select_mode;
vu8 WiFiSTAs[10][32];
vu32 countCWLAP, cwlap_mode;
vu32 count_name_sta_wifi, current_name_sta_wifi, count_power_sta_wifi;
u8 WiFiSetupInfraPassword[64];
u32 wifi_pswd_selected_symbol_table, cntr_wifi_pswd_symbol;
u8 offset_wifi_password;
u8 stage;

/*--AT commands--*/
/*!
	 \brief      Switch wifi mode with save in flash.
	 \param[in]  -
	 \retval		 OK
	 \note	    AT+RST
*/
const char reset[] = "AT+RST";

/*!
	 \brief      Switch wifi mode with save in flash. Will be saved in flash.
	 \param[in]  mode  1: Station, 2: SoftAp, 3: Station + SoftAP
	 \retval		 OK
	 \note		 AT+CWMODE_DEF=<mode>
	 \note		 Current params: 1
*/
const char setWifiMode1[] = "AT+CWMODE_DEF=1";

/*!
	 \brief      Switch wifi mode with save in flash. Wont be saved in flash.
	 \param[in]  mode  1: Station, 2: SoftAp, 3: Station + SoftAP
	 \retval		 OK
	 \note	    AT+CWMODE_DEF=<mode>
	 \note		 Current params: 2
*/
const char setWifiMode2[] = "AT+CWMODE=2";

/*!
	 \brief      Switch wifi mode with save in flash. Will be saved in flash.
	 \param[in]  mode  1: Station, 2: SoftAp, 3: Station + SoftAP
	 \retval		 OK
	 \note		 AT+CWMODE_DEF=<mode>
	 \note		 Current params: 3
*/
const char setWifiMode3[] = "AT+CWMODE_DEF=3";

/*!
	 \brief      Create Soft Access Point with save in flash. Will be saved in flash.
	 \param[in]  ID   String parametr, SSID of AP
	 \param[in]	 pwd  String parametr, 8-64 bytes ASCII
	 \param[in]	 chl  channel ID
	 \param[in]	 enc  Encryption method, WEP is not supported. 0: Open, 2: WPA_PSK, 3: WPA2_PSK, 4: WPA_WPA2_PSK
	 \param[in]	 maxConn(optional) maximum nuber of Stations to which ESP8266 SoftAP can be conencted
	 \param[in]  ssidHidden(optional) 0: SSID is broadcasted, 1: SSID is not broadcasted
	 \retval		 OK or ERROR
	 \note	    AT+SWSAP_DEF=<ID>,<password>,<channel>,<enc>,[<maxConn>],[<ssidHidden>]
	 \note		 Current params: TRP,12345678,1,3
*/
const char createSoftAP[] = "AT+CWSAP_DEF=\"TRP\",\"12345678\",1,3";

/*!
	 \brief      Check\set IP address in SoftAP mode. Will be saved in flash.
	 \param[in]  IP string parametr, the IP address of the ESP8266 SoftAP
	 \param[in]  gateway gateway
	 \param[in]  netmask netmask
	 \retval		 OK
	 \note		 AT+CIPAP_DEF=<IP>,<gateway>,<netmask>
	 \note 		 Current params: 192.168.0.1, 192.168.0.1, 255.255.255.0
*/
const char setIPAddress[] = "AT+CIPAP_DEF=\"192.168.0.1\",\"192.168.0.1\",\"255.255.255.0\"";

/*!
	 \brief      Enable or Disable multiple connections.
	 \param[in]  mode 0: single connection, 1: multiple connections
	 \retval		 OK
	 \note		 AT+CIPMUX=<mode>
	 \note 		 Current params: 1
*/
const char enableMulticonn[] = "AT+CIPMUX=1";

/*!
    \brief      Deletes/Creates TCP Server.
    \param[in]  mode 0: deletes server, 1: creates server
    \param[in]  port port number, 333 by default
    \retval		 OK
    \note		 AT+CIPSERVER=<mode>,<port>
    \note 		 Current params: 1, 53004
    \note       Port depends on device type
    \note       Check "V\:\_Temp\Отдел ИТ\VarnavIS\Telemetric (DevWeb)\ports.txt"
*/
const char createTCPServer[] = "AT+CIPSERVER=1,53004";

/*!
	 \brief      Enables/Disables DHCP. Configuration saved in flash.
	 \param[in]  mode 0: Sets ESP8266 SoftAP, 1: Sets ESP8266 Station, 2: Sets both SoftAP and Station
	 \param[in]  en 0: Disables DHCP, 1: Enables DHCP
	 \retval		 OK
	 \note		 AT+CWDHCP_DEF=<mode>,<en>
	 \note 		 Current params: 1, 1
*/
const char turnOnDHCP[] = "AT+CWDHCP_DEF=1,1";

/*!
	 \brief      Connects to an AP. Configuration saved in the flash.
	 \param[in]  ssid the SSID of the target AP
	 \param[in]  pwd password, MAX: 64-byte ASCII
	 \param[in]  bssid(optional) the target AO's MAC address, used when multiple APs have the same SSID
	 \param[in]  pci_en(optional) disable the connection to WEP or OPEN AP and can be used for PCI authentication
	 \retval		 OK or +CWJAP_DEF:<error code> FAIL
	 \note		 AT+CWJAP_DEF=<ssid>,<pwd>,[<bssid>],[<pci_en>]
	 \note 		 Current params: KiX, averon12
*/
const char connectToAP[] = "AT+CWJAP_DEF=\"KiX\",\"averon12\"";

/*!
	 \brief      Enable/Disable autoconnetion to the AP
	 \param[in]  enable 0: does NOT auto-connect to AP on pwoer-up, 1: connects to AP automatically on power-up
	 \retval		 OK
	 \note		 AT+CWAUTOCONN=<enable>
	 \note 		 Current params: 0
*/
const char disableAutoconn[] = "AT+CWAUTOCONN=0";

/*!
	 \brief      Sets the size of SSL buffer.
	 \param[in]  size the size of the SSL buffer, range value: [2048, 4096]
	 \retval		 OK
	 \note		 AT+CIPSSLSIZE=<size>
	 \note 		 Current params: 4096
*/
const char setSSLSize[] = "AT+CIPSSLSIZE=4096";

/*!
	 \brief      Establishes TCP connection, UDP transmission or SSL connection.
	 \param[in]  linkID ID of network connection (0-4), used for multiple connections
	 \param[in]  type string parameter indicating the connection type: "TCP", "UDP" or "SSL"
	 \param[in]  remoteIP string parameter indicating the remote IP address
	 \param[in]  remotePort the remote port number
	 \param[in]  TCPKeepAlive(optional) detection time interval when TCP is kept alive, this function is disabled by default.
	 \retval		 OK or ERROR or ALREADY CONNECTED
	 \note		 AT+CIPSTART=<linkID>,<type>,<remoteIP>,<remotePort>,[<TCPKeepAlive>]
	 \note 		 Current params: 4, SSL, device.averon.ru, 443
*/
const char establishSSLConn[] = "AT+CIPSTART=4,\"SSL\",\"device.averon.ru\",443";

/*Requests*/
const char cmd_get_reg[] = "GET /dr/4/1/810001 HTTP/1.1\x0D\x0AHost: device.averon.ru\x0D\x0A\x0D\x0A";
const char cmd_post_power_on[] = "POST /dt/ HTTP/1.1\r\nHost: device.averon.ru\r\nContent-Type: application/json\r\nContent-Length: {\r\n\r\n";
const char cmd_post_power_on_data[] = "{\"e\":,\"m\":,\"o\":,\"f\":,\"d\":,\"u\":,\"h\":,\"w1\":,\"w2\":,\"w3\":,\"w4\":,\"w5\":,\"w6\":,\"w7\":,\"w8\":,\"w9\":,\"w10\":,\"w11\":,\"w12\":,\"w13\":,\"w14\":,\"w15\":,\"w16\":,\"w17\":}";
const char cmd_post_param[] = "POST /dt/ HTTP/1.1\r\nHost: device.averon.ru\r\nContent-Type: application/json\r\nContent-Length: {\r\n\r\n";
const char cmd_post_param_data[] = "{\"e\":,\"m\":,\"p\":{\"t\":,\"s\":,\"v\":,\"c\":,\"z\":,\"b\":,\"a\":,\"n\":\"\",\"l\":\"\"}}";
const char cmd_post_run_prog[] = ",\"y\":[";
const char cmd_get_dgus[] = "GET /dgus//0/200 HTTP/1.1\r\nHost: device.averon.ru\r\n\r\n";
const char cmd_get_dgup[] = "GET /dgup//0/0 HTTP/1.1\r\nHost: device.averon.ru\r\n\r\n";
const char sub_server[] = "-sub";

/*Configurable data*/
#define NAME_TAG_SIZE 3
#define SERIAL_NUMBER_SIZE 6
#define SSID_SIZE (NAME_TAG_SIZE + SERIAL_NUMBER_SIZE + 2)

u8 ssid[SSID_SIZE] = {'T', 'R', 'P', '#'};
u32 serial_number;

u32 send_params[24];

u8 GetID(void)
{
	return termopress_id; /*Put your id variable here (termopress_id, evp_id, etc.)*/
}

/* Fill parameters that will be send in fist parameters packet. FormParamsPacket1()
 * It is here because parameters depeds on which device this API is used. */
void FillParams1(void)
{
	send_params[0] = alert_flag0;
	send_params[1] = termopress_mode;
	send_params[2] = 1;
	send_params[3] = VERS_PULT_H;
	send_params[4] = VERS_PULT_L;
	send_params[5] = major_firmware;
	send_params[6] = minor_firmware;
	send_params[7] = tar_prm.cold_bias;
	send_params[8] = tar_prm.couple_bias;
	send_params[9] = tar_prm.couple_kfc;
	send_params[10] = tar_prm.press_bias;
	send_params[11] = tar_prm.press_kfc;
	send_params[12] = 0;
	send_params[13] = 0;
	send_params[14] = 0;
	send_params[15] = 0;
	send_params[16] = 0;
	send_params[17] = 0;
	send_params[18] = 0;
	send_params[19] = 0;
	send_params[20] = 0;
	send_params[21] = 0;
	send_params[22] = 0;
	send_params[23] = 0;
}

/* Does not work yet, go in FormParamsPacket2() */
void FillParams2(void)
{
	send_params[0] = alert_flag0;
	send_params[1] = termopress_mode;
	send_params[2] = mes_temperature;
	send_params[3] = 0;
	send_params[4] = mes_pressure / 10;
	send_params[5] = 0;
	send_params[6] = count_zone;
	send_params[7] = 0;
	send_params[8] = prc_time;
	send_params[9] = 0;
	send_params[10] = 0;
	send_params[11] = 0;
	send_params[12] = 0;
	send_params[13] = 0;
	send_params[14] = 0;
	send_params[15] = 0;
	send_params[16] = 0;
	send_params[17] = 0;
	send_params[18] = 0;
	send_params[19] = 0;
	send_params[20] = 0;
	send_params[21] = 0;
	send_params[22] = 0;
	send_params[23] = 0;
}

/* Functions */

/* Copy strings */
void Strcpy(u8 *dest, char *str, u8 max_len)
{
	u8 i = 0;
	do
	{
		if (i == max_len)
		{
			dest[i] = 0;
			break;
		}

		dest[i] = str[i];
	} while (str[i++]);
}

/* Check if strings equal */
ErrorStatus Strcmp(u8 *str1, char *str2)
{
	u8 ii = 0;

	while (1)
	{
		if (!(str1[ii] - str2[ii]))
		{
			if (str1[ii] == 0)
				return SUCCESS;
			else
				ii++;
		}
		else
			return ERROR;
	}
}

void USART_SendStr(const char *String)
{
   while (*String)
      TransmitByte2(*String++);
}

/*
 * Исполнение любого AT-запроса
 * Возможные результаты работы в ATcommandStatus
 */
void ESP8266_SendCommand(USART_TypeDef *USARTx, const char *buf, u16 timeout_ms)
{
   RESET_WIFI_MORE_MESSAGE;

   connectionState = ATCMD;

   packet2tx_count = 0;
   end_trans_buff2 = 0;

   USART_SendStr(buf);
   TransmitByte2(0x0D);
   TransmitByte2(0x0A);

   while (!(USARTx->SR & USART_SR_TC))
      ;

   USART_SendData(USARTx, trans_buff2[packet2tx_count++]);
   USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);

   if (!timeout_ms)
   {
      fResp = WSTANBY;
      return;
   }

   fResp = WRUN;
   responseCount = 0;
   Response[0] = 0;
   cntr_timeout = 0;
   wifi_timeout = timeout_ms;
}

/* Отправка данных в WiFi модуль */
void ESP8266_SendData(void)
{
   switch (wifi_senddata_zone)
   {
   case 10:
   case 0:
      if (connectionState == UART2TCP_STANBY)
      {
         u8 etb2, etb2_1, etb2_2, etb2_3;
         char cmd[] = "AT+CIPSEND=0,000";
         etb2 = end_trans_buff2;
         etb2_1 = 0;
         etb2_2 = 0;
         etb2_3 = 0;

         while (etb2 > 99)
         {
            etb2 -= 100;
            etb2_1++;
         }

         while (etb2 > 9)
         {
            etb2 -= 10;
            etb2_2++;
         }

         while (etb2)
         {
            etb2--;
            etb2_3++;
         }

         if (++cntr_TX_IPD >= IDOFIPD_BUFF_LENGTH)
            cntr_TX_IPD = 0;

         cmd[11] = IDofIPD[cntr_TX_IPD] + '0';

         cmd[13] = etb2_1 + '0';
         cmd[14] = etb2_2 + '0';
         cmd[15] = etb2_3 + '0';
         RESET_WIFI_MORE_MESSAGE;
         ESP8266_SendCommand(USART_WiFi, &cmd[0], 1000);
         wifi_senddata_zone++;
      }
      break;

   case 1:
   case 11:
      if (fResp != WRUN)
      {
         if (fResp == WOK)
         {
            wifi_senddata_zone++;
         }
         else
         {
            wifi_senddata_zone = 99;
            connectionState = UART2TCP_STANBY;
            RESET_SEND_DATA_TO_TABLET;
         }
      }
      break;

   case 2:
   case 12:
      if (WIFI_MORE_MESSAGE)
         wifi_senddata_zone++;
      break;

   case 3:
   {
      u8 count_data;
      u8 check_summ; // Обнулить контрольную сумму
      packet2tx_count = 0;
      end_trans_buff2 = 0;

      TransmitByte2(0xFF);               // Поместить "Сигнатура устройства" в буфер порта RS232
      TransmitByte2(packet.device_num);  // Поместить "Номер устройства (сетевой адрес)" в буфер порта RS232
      TransmitByte2(packet.command_num); // Поместить "Номер команды" в буфер порта RS232
      TransmitByte2(packet.data_size);   // Поместить "Размер блока данных" в буфер порта RS232

      check_summ = packet.device_num;
      check_summ += packet.command_num;
      check_summ += packet.data_size;

      if (packet.data_size > 0)
      {
         for (count_data = 0; count_data < packet.data_size; count_data++)
         {
            check_summ += packet.packet_data.data[count_data];  // Подсчитать контрольную сумму данных
            TransmitByte2(packet.packet_data.data[count_data]); // Блок данных

            if (packet.packet_data.data[count_data] == 0xFF)
               TransmitByte2(0xFF); // Дублировать код 0xFF
         }
      }
      TransmitByte2(check_summ); // Поместить "Контрольная сумма" пакета в буфер порта RS232

      connectionState = UART2TCP_SEND;
      wifi_senddata_zone++;

      while (!(USART_WiFi->SR & USART_SR_TC))
         ;

      USART_SendData(USART_WiFi, trans_buff2[packet2tx_count++]);
      USART_ITConfig(USART_WiFi, USART_IT_TXE, ENABLE);
   }
   break;

   case 13:
   {
      u8 count_data;
      u8 check_summ; // Обнулить контрольную сумму

      packet2tx_count = 0;
      end_trans_buff2 = 0;

      TransmitByte2(0xFF);                  // Поместить "Сигнатура устройства" в буфер порта RS232
      TransmitByte2(packet2tx.device_num);  // Поместить "Номер устройства (сетевой адрес)" в буфер порта RS232
      TransmitByte2(packet2tx.command_num); // Поместить "Номер команды" в буфер порта RS232
      TransmitByte2(packet2tx.data_size);   // Поместить "Размер блока данных" в буфер порта RS232

      check_summ = packet2tx.device_num;
      check_summ += packet2tx.command_num;
      check_summ += packet2tx.data_size;

      if (packet2tx.data_size > 0)
      {
         for (count_data = 0; count_data < packet2tx.data_size; count_data++)
         {
            check_summ += packet2tx.packet_data.data[count_data];  // Подсчитать контрольную сумму данных
            TransmitByte2(packet2tx.packet_data.data[count_data]); // Блок данных

            if (packet2tx.packet_data.data[count_data] == 0xFF)
               TransmitByte2(0xFF); // Дублировать код 0xFF
         }
      }

      TransmitByte2(check_summ); // Поместить "Контрольная сумма" пакета в буфер порта RS232

      connectionState = UART2TCP_SEND;
      wifi_senddata_zone++;

      while (!(USART_WiFi->SR & USART_SR_TC))
         ;

      USART_SendData(USART_WiFi, trans_buff2[packet2tx_count++]);
      USART_ITConfig(USART_WiFi, USART_IT_TXE, ENABLE);
   }
   break;

   default:
      break;
   }
}

/*
 * ATcommandStatus ESP8266_GetIP(USART_TypeDef* USARTx, char* ip_addr, u16 timeout_ms)
 *
 * +CIFSR:STAMAC,"
 * +CIFSR:APMAC,"
 * +CIFSR:APIP,"192.168.0.1"
 * +CIFSR:APMAC,"1a:fe:34:99:a5:e2"
 * +CIFSR:STAIP,"0.0.0.0"
 * +CIFSR:STAMAC,"18:fe:34:99:a5:e2"
 *
 * Функция позволяет получить IP-адресс подключения
 */
void ESP8266_GetIP(USART_TypeDef *USARTx, char *ip_addr, u16 timeout_ms)
{
   if (wifi_getip_zone == 0)
   {
      ESP8266_SendCommand(USART_WiFi, "AT+CIFSR", timeout_ms);
      wifi_getip_zone = 1;
   }
   else if (wifi_getip_zone == 1)
   {
      if (fResp != WRUN)
      {
         if (fResp == WOK)
            wifi_getip_zone = 2;
         else
            wifi_getip_zone = 11;
      }
   }
   else if (wifi_getip_zone == 2)
   {
      u8 ii = 0;
      u8 jj = 0;

      while (Response[ii])
      {
         if (jj == 21)
         {
            ip_addr[19] = 0;
            wifi_getip_zone = 11;
            break;
         }

         if (Response[ii] == '"' && !jj)
         {
            jj++;
         }
         else if (Response[ii] == '"' && jj)
         {
            ip_addr[jj - 1] = 0;
            wifi_getip_zone = 3;
            connectionState = UART2TCP_STANBY;
            break;
         }
         else if (jj)
         {
            ip_addr[jj - 1] = Response[ii];
            jj++;
         }

         ii++;
      }

      if (wifi_getip_zone != 3)
         wifi_getip_zone = 11;
   }
}

void GetIP51(void)
{
   ESP8266_SendCommand(USART_WiFi, "AT+CIFSR", 1000);
   wifi_setup_zone++;
}

void GetIP53(void)
{
   u8 ii = 0;
   u8 jj = 0;

   while (Response[ii])
   {
      if (jj == 21)
      {
         currentIP[19] = 0;
         break;
      }

      if (Response[ii] == '"' && !jj)
      {
         jj++;
      }
      else if (Response[ii] == '"' && jj)
      {
         currentIP[jj - 1] = 0;
         wifi_setup_zone = 18;
         connectionState = UART2TCP_STANBY;
         break;
      }
      else if (jj)
      {
         currentIP[jj - 1] = Response[ii];
         jj++;
      }

      ii++;
   }

   for (; jj < 16; jj++)
      currentIP[jj - 1] = ' ';

   if (jj <= 16)
      currentIP[jj - 1] = 0;

   if (wifi_setup_zone != 18)
   {
      wifi_setup_zone = 99;
   }
}

char temp_char[11];

u8 DecToChar(char *tc, u32 num)
{
   u32 t_num = num;
   char tmp[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
   u8 c, i;

   while (t_num >= 1000000000)
   {
      t_num -= 1000000000;
      tmp[0]++;
   }

   while (t_num >= 100000000)
   {
      t_num -= 100000000;
      tmp[1]++;
   }

   while (t_num >= 10000000)
   {
      t_num -= 10000000;
      tmp[2]++;
   }

   while (t_num >= 1000000)
   {
      t_num -= 1000000;
      tmp[3]++;
   }

   while (t_num >= 100000)
   {
      t_num -= 100000;
      tmp[4]++;
   }

   while (t_num >= 10000)
   {
      t_num -= 10000;
      tmp[5]++;
   }

   while (t_num >= 1000)
   {
      t_num -= 1000;
      tmp[6]++;
   }

   while (t_num >= 100)
   {
      t_num -= 100;
      tmp[7]++;
   }

   while (t_num >= 10)
   {
      t_num -= 10;
      tmp[8]++;
   }

   while (t_num >= 1)
   {
      t_num -= 1;
      tmp[9]++;
   }

   for (i = 0; i < 10; i++)
   {
      if (tmp[0] != 0)
         break;
      for (c = 0; c < 9; c++)
         tmp[c] = tmp[c + 1];
   }

   if (i == 10)
      i--;

   for (c = 0; c < 10 - i; c++)
      tc[c] = tmp[c] + '0';

   tc[c] = 0;

   return 10 - i;
}

void ESPConfSpeed(void)
{
   ESP8266_SendCommand(USART_WiFi, "AT+UART_DEF=115200,8,1,0,0", 1000);
   wifi_setup_zone++;
}

void USARTConf(void)
{
   if (fResp != WRUN)
   {
      USART1_Configuration(115200);
      wifi_setup_zone++;
   }
}

void ESPEchoOff(void)
{
   ESP8266_SendCommand(USART_WiFi, "ATE0", 2000); // Команда ATE0 отключает эхо на AT-командыPutChr_LCD(_comm, 0xCF);
   wifi_setup_zone++;
}

void OkCheck(void)
{
   if (fResp != WRUN)
   {
      if (fResp == WOK)
         wifi_setup_zone++;
      else
      {

         wifi_setup_zone = 99;
      }
   }
}

void OkCheckWithIPReq(void)
{
   if (fResp != WRUN)
   {
      if (fResp == WOK)
      {
         wifi_setup_zone++;
         SET_REQUEST_IP;
         connectionState = UART2TCP_STANBY;
      }
      else
         wifi_setup_zone = 99;
   }
}

void OkCheckWithSSLConnReset(void)
{
   if (fResp != WRUN)
   {
      if (fResp == WOK)
      {
         wifi_setup_zone++;
         RESET_SSL_CONNECTED;
         SET_REQUEST_IP;
      }
      else
         wifi_setup_zone = 99;
   }
}

void OkCheckWithCommand(void)
{
   if (fResp != WRUN)
   {
      if (fResp == WOK)
         wifi_setup_zone++;
      else
      {
         if (SUB_SERVER)
         {
            wifi_setup_zone = 99;
         }
         else
         {
            SET_SUB_SERVER;
            SET_NEED_RECEIVE_TOKEN;
            wifi_setup_zone = 54;
            ESP8266_SendCommand(USART_WiFi, &reset[0], 4000);
         }
      }
   }
}

void OkCheckWSubTokenFlags(void)
{
   if (fResp != WRUN)
   {
      if (fResp == WOK)
         wifi_setup_zone++;
      else
      {
         if (SUB_SERVER)
         {
            wifi_setup_zone = 99;
         }
         else
         {
            SET_SUB_SERVER;
            SET_NEED_RECEIVE_TOKEN;
            wifi_setup_zone = 18;
         }
      }
   }
}

void OkCheckWPowerOnFlag(void)
{
   if (fResp != WRUN)
   {
      if (fResp == WOK)
         wifi_setup_zone++;
      else
      {
         if (SUB_SERVER)
         {
            wifi_setup_zone = 99;
            RESET_SUB_SERVER;
            SET_SEND_TO_SERVER_POWER_ON;
         }
         else
         {
            SET_SUB_SERVER;
            SET_SEND_TO_SERVER_POWER_ON;
            wifi_setup_zone = 18;
         }
      }
   }
}

void Reserved(void)
{
   wifi_setup_zone++;
}

void ESPSetSoftAPMode(void)
{
   ESP8266_SendCommand(USART_WiFi, setWifiMode2, 2000);
   wifi_setup_zone++;
}

void ESPSetStationAPMode(void)
{
   ESP8266_SendCommand(USART_WiFi, setWifiMode3, 2000);
   wifi_setup_zone++;
}

void ESPConfAPParam(void)
{
   u8 sn[6], i, c;
   char cmd[128];
   u32 usn = serial_number;

   for (c = 0; c < 6; c++)
      sn[c] = 0;

   while (usn > 99999)
   {
      usn -= 100000;
      sn[0]++;
   }

   while (usn > 9999)
   {
      usn -= 10000;
      sn[1]++;
   }

   while (usn > 999)
   {
      usn -= 1000;
      sn[2]++;
   }

   while (usn > 99)
   {
      usn -= 100;
      sn[3]++;
   }

   while (usn > 9)
   {
      usn -= 10;
      sn[4]++;
   }

   while (usn)
   {
      usn--;
      sn[5]++;
   }

   for (c = 0; c < SERIAL_NUMBER_SIZE; c++)
      ssid[c + NAME_TAG_SIZE + 1] = sn[c] + '0';

   ssid[SSID_SIZE - 1] = 0;

   for (i = 0; i < 14; i++)
      cmd[i] = createSoftAP[i];

   for (c = 0; c < SSID_SIZE - 1; c++)
      cmd[i + c] = ssid[c];

   i += c;
   cmd[i++] = '"';
   cmd[i++] = ',';
   cmd[i++] = '"';

   if (wifi_param.w.wifi_flags & 0x02)
   {
      for (c = 0; c < 8; c++)
         cmd[i + c] = c + '1';
   }
   else
   {
      for (c = 0; c < 64; c++)
      {
         if (wifi_param.w.pswd[c] == 0)
            break;

         cmd[i + c] = wifi_param.w.pswd[c];
      }
   }

   i += c;
   cmd[i++] = '"';
   cmd[i++] = ',';
   cmd[i++] = '1';
   cmd[i++] = ',';
   cmd[i++] = '3';
   cmd[i] = 0;

   ESP8266_SendCommand(USART_WiFi, cmd, 2000);
   wifi_setup_zone++;
}

void ESPSetIP(void)
{
   ESP8266_SendCommand(USART_WiFi, setIPAddress, 2000);
   wifi_setup_zone++;
}

void ESPMulticonnMode(void)
{
   ESP8266_SendCommand(USART_WiFi, enableMulticonn, 2000);
   wifi_setup_zone++;
}

void ESPRunTCPServ(void)
{
   ESP8266_SendCommand(USART_WiFi, createTCPServer, 2000);
   wifi_setup_zone++;
}

void GoToRequestIP(void)
{
   if ((connectionState == UART2TCP_STANBY))
   {
      if (REQUEST_IP)
      {
         RESET_REQUEST_IP;
         wifi_setup_zone++;
      }
   }
}

void ESPStationMode(void)
{
   ESP8266_SendCommand(USART_WiFi, setWifiMode1, 2000);
   wifi_setup_zone++;
}

void ESPDHCPStationMode(void)
{
   ESP8266_SendCommand(USART_WiFi, turnOnDHCP, 2000);
   wifi_setup_zone++;
}

void ESPSetWifiParam(void)
{
   char cmd[128];
   u8 i, c;

   for (i = 0; i < 14; i++)
      cmd[i] = connectToAP[i];

   for (c = 0; c < 33; c++)
   {
      if (wifi_param.w.ssid[c] == 0)
         break;

      cmd[i + c] = wifi_param.w.ssid[c];
   }

   i += c;
   cmd[i++] = '"';
   cmd[i++] = ',';
   cmd[i++] = '"';

   for (c = 0; c < 64; c++)
   {
      if (wifi_param.w.pswd[c] == 0)
         break;

      cmd[i + c] = wifi_param.w.pswd[c];
   }

   i += c;
   cmd[i++] = '"';
   cmd[i] = 0;
   ESP8266_SendCommand(USART_WiFi, cmd, 20000);
   wifi_setup_zone++;
}

void ESPEnableMultiConn(void)
{
   ESP8266_SendCommand(USART_WiFi, enableMulticonn, 2000);
   wifi_setup_zone++;
}

void RunCheck(void)
{
   if (fResp != WRUN)
      wifi_setup_zone++;
}

void ESPDisableAutoConn(void)
{
   ESP8266_SendCommand(USART_WiFi, disableAutoconn, 2000);
   wifi_setup_zone++;
}

void ESPSetSSLSize(void)
{
   ESP8266_SendCommand(USART_WiFi, setSSLSize, 2000);
   wifi_setup_zone++;
}

void ApConnectedActions(void)
{
   if ((connectionState == UART2TCP_STANBY) && (!SEND_DATA_TO_TABLET) && (!DELAY_SEND_TO_SERVER) && (!PACKET2_COMPLETE)) // && (!BUSY_WIFI_UART))
   {
      if (REQUEST_IP)
      {
         RESET_REQUEST_IP;
         wifi_setup_zone = 51;
      }
      else if (NEED_RECEIVE_TOKEN)
      {
         wifi_setup_zone++;
         SET_SEND_DATA_TO_SERVER;
      }
      else if (SEND_TO_SERVER_POWER_ON)
      {
         wifi_setup_zone++;
         SET_SEND_DATA_TO_SERVER;
      }
      else if (TX_ERROR_TO_SERVER)
      {
         wifi_setup_zone++;
         SET_SEND_DATA_TO_SERVER;
      }
      else if (REQUEST_FIRMWARE_LENGTH)
      {
         wifi_setup_zone++;
         SET_SEND_DATA_TO_SERVER;
      }
      else if (REQUEST_FIRMWARE_DATA)
      {
         if ((cntr_firmware_block % 128) == 0)
         {
            if (++cntr_delay_ddos > 400000)
            {
               cntr_delay_ddos = 0;
               wifi_setup_zone++;
               SET_SEND_DATA_TO_SERVER;
            }
         }
         else
         {
            wifi_setup_zone++;
            SET_SEND_DATA_TO_SERVER;
         }
      }
   }
}

void EstablishSSLConn(void)
{
   if (connectionState == UART2TCP_STANBY)
   {
      if (NEED_RECEIVE_TOKEN)
      {
         wifi_setup_zone += 2;
      }
      else if (SEND_TO_SERVER_POWER_ON)
         wifi_setup_zone = 31;
      else if (TX_ERROR_TO_SERVER)
      {
         RESET_TX_ERROR_TO_SERVER;
         wifi_setup_zone = 36;
      }
      else if (REQUEST_FIRMWARE_LENGTH)
      {
         RESET_REQUEST_FIRMWARE_LENGTH;
         wifi_setup_zone = 41;
      }
      else if (REQUEST_FIRMWARE_DATA)
      {
         wifi_setup_zone = 46;
      }

      if (!SSL_CONNECTED)
      {
         SET_SEND_DATA_TO_SERVER;
         char cipstart[sizeof(establishSSLConn) + sizeof(sub_server)];
         u8 c = 0, s = 0, k = 0;

         while (establishSSLConn[s] != '.')
         {
            cipstart[c++] = establishSSLConn[s++];
         }

         if (SUB_SERVER)
         {
            while (sub_server[k])
            {
               cipstart[c++] = sub_server[k++];
            }
         }

         while (establishSSLConn[s])
         {
            cipstart[c++] = establishSSLConn[s++];
         }

         cipstart[c] = 0;

         ESP8266_SendCommand(USART_WiFi, &cipstart[0], 20000);
         wifi_setup_zone--;
      }
   }
}

void ESPSendIPData(void)
{
   char cmd[] = "AT+CIPSEND=4,00";
   u8 a, c;

   SET_SSL_CONNECTED;
   SET_SEND_DATA_TO_SERVER;
   RESET_NEED_RECEIVE_TOKEN;

   a = sizeof(cmd_get_reg) - 1;

   if (SUB_SERVER)
   {
      a += 1 * (sizeof(sub_server) - 1);
   }

   c = a / 10;
   cmd[13] = c + '0';
   c = a % 10;
   cmd[14] = c + '0';
   RESET_WIFI_MORE_MESSAGE;
   ESP8266_SendCommand(USART_WiFi, &cmd[0], 4000);
   wifi_setup_zone++;
}

void WaitForMoreSign(void)
{
   if (WIFI_MORE_MESSAGE)
      wifi_setup_zone++;
}

void FromSerialNumber(void)
{
   u8 c = 0, i = 0, a = 0, sub = sizeof(sub_server) - 1;
   u8 lngth = sizeof(cmd_get_reg) + 1 * sub + 1;
   char cmd[lngth];
   u32 usn = serial_number;

   while (a < 41)
      cmd[c++] = cmd_get_reg[a++];

   if (SUB_SERVER)
   {
      while (sub_server[i])
      {
         cmd[c++] = sub_server[i++];
      }
   }

   while (cmd_get_reg[a])
      cmd[c++] = cmd_get_reg[a++];

   cmd[c] = 0;
   a = 10;
   cmd[a++] = GetID() + '0';
   c = 0;

   while (usn > 99999)
   {
      usn -= 100000;
      c++;
   }

   cmd[++a] = c + '0';
   c = 0;

   while (usn > 9999)
   {
      usn -= 10000;
      c++;
   }

   cmd[++a] = c + '0';
   c = 0;

   while (usn > 999)
   {
      usn -= 1000;
      c++;
   }

   cmd[++a] = c + '0';
   c = 0;

   while (usn > 99)
   {
      usn -= 100;
      c++;
   }

   cmd[++a] = c + '0';
   c = 0;

   while (usn > 9)
   {
      usn -= 10;
      c++;
   }

   cmd[++a] = c + '0';
   c = 0;

   while (usn)
   {
      usn--;
      c++;
   }

   cmd[++a] = c + '0';

   ESP8266_SendCommand(USART_WiFi, &cmd[0], 4000);
   wifi_setup_zone = 18;
}

void FormParamsPacket1(void)
{
   char cmd[] = "AT+CIPSEND=4,184";
   char data[500];
   u32 c, s, t, z;
   u32 data_length, post_length;

   RESET_SEND_TO_SERVER_POWER_ON;
   SET_SSL_CONNECTED;
   SET_SEND_DATA_TO_SERVER;

   FillParams1();

   for (c = 0, s = 0, z = 0; z < sizeof(send_params) / sizeof(u32); z++)
   {
      while (cmd_post_power_on_data[s] != ':')
         data[c++] = cmd_post_power_on_data[s++];

      data[c++] = cmd_post_power_on_data[s++];

      for (t = 0; t < DecToChar(&temp_char[0], send_params[z]); t++)
         data[c++] = temp_char[t];
   }

	while (cmd_post_power_on_data[s] != 0)
		data[c++] = cmd_post_power_on_data[s++];

	data[c] = 0;
	data_length = c;
	c = 0;
	s = 0;

	while (s < 9)
	{
		post_buffr[c++] = cmd_post_power_on[s++];
	}

	for (t = 0; t < 40; c++, t++)
	{
		post_buffr[c] = token40[t];
	}

	if (SUB_SERVER)
	{
		while (cmd_post_power_on[s] != '.')
			post_buffr[c++] = cmd_post_power_on[s++];

		post_buffr[c++] = cmd_post_power_on[s++];

		while (cmd_post_power_on[s] != '.')
			post_buffr[c++] = cmd_post_power_on[s++];

		t = 0;

		while (sub_server[t])
		{
			post_buffr[c++] = sub_server[t++];
		}
	}

	while (cmd_post_power_on[s] != '{')
		post_buffr[c++] = cmd_post_power_on[s++];

	for (t = 0; t < DecToChar(&temp_char[0], data_length); t++)
		post_buffr[c++] = temp_char[t];

	s++;

	while (cmd_post_power_on[s] != 0)
		post_buffr[c++] = cmd_post_power_on[s++];

	for (t = 0; t < data_length; t++)
		post_buffr[c++] = data[t];

	post_buffr[c] = 0;
	post_length = c;

	for (t = 0; t < DecToChar(&temp_char[0], post_length); t++)
		cmd[t + 13] = temp_char[t];

	cmd[t + 13] = 0;

	RESET_WIFI_MORE_MESSAGE;
	ESP8266_SendCommand(USART_WiFi, &cmd[0], 4000);

	wifi_setup_zone++;
}

void SendParamsPacket(u16 timeout_ms)
{
	ESP8266_SendCommand(USART_WiFi, &post_buffr[0], timeout_ms);
	connectionState = UART2TCP_SEND;
	wifi_setup_zone = 18;
}

void SendParamsPacket34(void)
{
	SendParamsPacket(4000);
}

void SendParamsPacket39(void)
{
	SendParamsPacket(2000);
}

void SendGetRequest(u16 timeout_ms)
{
	ESP8266_SendCommand(USART_WiFi, &post_buffr[0], timeout_ms);
	connectionState = UART2TCP_SEND;
	wifi_setup_zone = 18;
}

void SendGetRequest44(void)
{
	SET_FRAGMENT_FIRMWARE;
	SendGetRequest(4000);
}

void SendGetRequest49(void)
{
	SendGetRequest(4000);
}

void FormParamsPacket2(void)
{
	char cmd[] = "AT+CIPSEND=4,184";
	char data[500];
	u32 c, s, t, z;
	u32 data_length, post_length;
	u32 send_params[9];
	SET_SSL_CONNECTED;
	SET_SEND_DATA_TO_SERVER;

   send_params[0] = alert_flag0;
   send_params[1] = termopress_mode;
   send_params[2] = mes_temperature;
   send_params[3] = 0;
   send_params[4] = mes_pressure / 10;
   send_params[5] = 0;
   send_params[6] = count_zone;
   send_params[7] = 0;
   send_params[8] = prc_time;

   s = 0;
	c = 0;

	for (z = 0; z < sizeof(send_params) / sizeof(u32); z++)
	{
		if (z == 2)
		{
			while (cmd_post_param_data[s] != ':')
				data[c++] = cmd_post_param_data[s++];

			data[c++] = cmd_post_param_data[s++];
		}

		while (cmd_post_param_data[s] != ':')
			data[c++] = cmd_post_param_data[s++];

		data[c++] = cmd_post_param_data[s++];

		for (t = 0; t < DecToChar(&temp_char[0], send_params[z]); t++)
			data[c++] = temp_char[t];
	}

	while (cmd_post_param_data[s] != ':') // n
		data[c++] = cmd_post_param_data[s++];

	data[c++] = cmd_post_param_data[s++];
	data[c++] = cmd_post_param_data[s++];

   for (t = 0; t < DecToChar(&temp_char[0], (prg_num + 1)); t++)
      data[c++] = temp_char[t];

   while (cmd_post_param_data[s] != ':') // l
      data[c++] = cmd_post_param_data[s++];

	data[c++] = cmd_post_param_data[s++];
	data[c++] = cmd_post_param_data[s++];

	while (cmd_post_param_data[s] != '}')
		data[c++] = cmd_post_param_data[s++];

   if (RUN_PROG_TO_SERVER) // передать параметры запущенной программы
   {
      u8 ii = 0;
      RESET_RUN_PROG_TO_SERVER;

      while (cmd_post_run_prog[ii] != 0)
         data[c++] = cmd_post_run_prog[ii++];

      for (t = 0; t < DecToChar(&temp_char[0], prg_prm[prg_num].pressure); t++)
         data[c++] = temp_char[t];
      data[c++] = ',';

      for (t = 0; t < DecToChar(&temp_char[0], prg_prm[prg_num].temperature); t++)
         data[c++] = temp_char[t];
      data[c++] = ',';         
      
      for (t = 0; t < DecToChar(&temp_char[0], prg_prm[prg_num].time_hold); t++)
         data[c++] = temp_char[t];
      data[c++] = ',';
      
      for (t = 0; t < DecToChar(&temp_char[0], prg_prm[prg_num].time_press); t++)
         data[c++] = temp_char[t];
      data[c++] = ',';

      for (t = 0; t < DecToChar(&temp_char[0], prg_prm[prg_num].wflags); t++)
         data[c++] = temp_char[t];
      data[c++] = ',';

      data[c - 1] = ']';
   }

   data[c++] = cmd_post_param_data[s++];
   data[c++] = cmd_post_param_data[s++]; //}}
   data[c] = 0;
   data_length = c;

   c = 0;
   s = 0;

   while (s < 9)
      post_buffr[c++] = cmd_post_power_on[s++];

   for (t = 0; t < 40; c++, t++)
      post_buffr[c] = token40[t];

   if (SUB_SERVER)
   {
      while (s < 32)
         post_buffr[c++] = cmd_post_power_on[s++];

      t = 0;

      while (sub_server[t])
         post_buffr[c++] = sub_server[t++];
   }

   while (cmd_post_param[s] != '{')
      post_buffr[c++] = cmd_post_param[s++];

   for (t = 0; t < DecToChar(&temp_char[0], data_length); t++)
      post_buffr[c++] = temp_char[t];

   s++;

   while (cmd_post_param[s] != 0)
      post_buffr[c++] = cmd_post_param[s++];

   for (t = 0; t < data_length; t++)
      post_buffr[c++] = data[t];

   post_buffr[c] = 0;
   post_length = c;

   for (t = 0; t < DecToChar(&temp_char[0], post_length); t++)
      cmd[t + 13] = temp_char[t];

   cmd[t + 13] = 0;

   RESET_WIFI_MORE_MESSAGE;
   ESP8266_SendCommand(USART_WiFi, &cmd[0], 20000);

   wifi_setup_zone++;
}

/* This GET request is for dividing the Firmware into 200 bytes sections */
void FormGetRequest41(void)
{
   char cmd[] = "AT+CIPSEND=4,184";
   u32 c = 0, s = 0, k;

   SET_SSL_CONNECTED;
   SET_SEND_DATA_TO_SERVER;

   while (s < 10)
      post_buffr[c++] = cmd_get_dgus[s++];

   for (k = 0; k < 40; k++)
      post_buffr[c++] = token40[k];

   post_buffr[c++] = cmd_get_dgus[s++];
   post_buffr[c++] = firmware_device_type + '0';
   s++;

   while (s < 39)
      post_buffr[c++] = cmd_get_dgus[s++];

   if (SUB_SERVER)
   {
      k = 0;

      while (sub_server[k])
         post_buffr[c++] = sub_server[k++];
   }

   while (cmd_get_dgus[s] != 0)
      post_buffr[c++] = cmd_get_dgus[s++];

   post_buffr[c] = 0;

   for (s = 0; s < DecToChar(&temp_char[0], c); s++)
      cmd[s + 13] = temp_char[s];

   cmd[s + 13] = 0;
   RESET_WIFI_MORE_MESSAGE;
   ESP8266_SendCommand(USART_WiFi, &cmd[0], 2000);

   wifi_setup_zone++;
}

/* This GET request for choosing witch 200 bytes file we wanna get*/
void FormGetRequest46(void)
{
   char cmd[] = "AT+CIPSEND=4,184";
   u32 c = 0, s = 0, k = 0;

   SET_SSL_CONNECTED;
   SET_SEND_DATA_TO_SERVER;
   RESET_REQUEST_FIRMWARE_DATA;

   while (s < 10)
      post_buffr[c++] = cmd_get_dgup[s++];

   for (k = 0; k < 40; k++)
      post_buffr[c++] = token40[k];

   post_buffr[c++] = cmd_get_dgup[s++];
   post_buffr[c++] = firmware_device_type + '0';
   s++;
   post_buffr[c++] = cmd_get_dgup[s++];

   for (k = 0; k < DecToChar(&temp_char[0], cntr_firmware_block); k++)
      post_buffr[c++] = temp_char[k];

   s++;

   while (s < 37)
      post_buffr[c++] = cmd_get_dgup[s++];

   if (SUB_SERVER)
   {
      k = 0;
      while (sub_server[k])
         post_buffr[c++] = sub_server[k++];
   }

   while (cmd_get_dgup[s] != 0)
      post_buffr[c++] = cmd_get_dgup[s++];

   post_buffr[c] = 0;

   for (s = 0; s < DecToChar(&temp_char[0], c); s++)
      cmd[s + 13] = temp_char[s];

   cmd[s + 13] = 0;
   RESET_WIFI_MORE_MESSAGE;
   ESP8266_SendCommand(USART_WiFi, &cmd[0], 2000);

   wifi_setup_zone++;
}

void Delay(void)
{
   if (++cntr_delay_ddos > 200000)
   {
      cntr_delay_ddos = 0;
      wifi_setup_zone = 0;
   }
}

void DelaySendProgToServer(void)
{
   if (DELAY_SEND_TO_SERVER)
   {
      if (++cntr_delay_send_to_server > 2)
      {
         cntr_delay_send_to_server = 0;
         RESET_DELAY_SEND_TO_SERVER;
      }
   }
}

void PeriodicRequestsToServer(void)
{
   if (wifi_param.w.wifi_flags & 0x04)
   {
      if (TX_TELEMETRY_TO_SERVER)
      {
         if (((time_restart % INTERVAL_SEND_TELEMETRY) == 0))
            SET_TX_ERROR_TO_SERVER;
      }
      else if ((wifi_param.w.wifi_flags & 0x08) && (!(wifi_param.w.wifi_flags & 0x01)))
      {
         if (time_restart == 30)
            SET_REQUEST_FIRMWARE_LENGTH;
      }

      if ((wifi_setup_zone == 99) && (time_restart % 30 == 0))
      {
         connectionState = UART2TCP_STANBY;
         wifi_setup_zone = 0;
      }
   }
}

void (*const HandleAccessPointMode[])(void) = {
    ESPConfSpeed,        // 0
    USARTConf,           // 1
    ESPEchoOff,          // 2
    OkCheck,             // 3
    ESPSetSoftAPMode,    // 4
    OkCheck,             // 5
    Reserved,            // 6
    Reserved,            // 7
    ESPSetStationAPMode, // 8
    OkCheck,             // 9
    ESPConfAPParam,      // 10
    OkCheck,             // 11
    ESPSetIP,            // 12
    OkCheck,             // 13
    ESPEnableMultiConn,  // 14
    RunCheck,            // 15
    ESPRunTCPServ,       // 16
    OkCheckWithIPReq,    // 17
    GoToRequestIP,       // 18
    GetIP51,             // 19
    OkCheck,             // 20
    GetIP53              // 21
};

void (*const HandleConnToRouterMode[])(void) = {
    ESPConfSpeed,            // 0
    USARTConf,               // 1
    ESPEchoOff,              // 2
    OkCheck,                 // 3
    ESPStationMode,          // 4
    OkCheck,                 // 5
    ESPDHCPStationMode,      // 6
    OkCheck,                 // 7
    ESPSetWifiParam,         // 8
    OkCheck,                 // 9
    ESPEnableMultiConn,      // 10
    RunCheck,                // 11
    ESPRunTCPServ,           // 12
    OkCheck,                 // 13
    ESPDisableAutoConn,      // 14
    OkCheck,                 // 15
    ESPSetSSLSize,           // 16
    OkCheckWithSSLConnReset, // 17
    ApConnectedActions,      // 18
    EstablishSSLConn,        // 19
    OkCheckWithCommand,      // 20
    ESPSendIPData,           // 21
    OkCheckWSubTokenFlags,   // 22
    WaitForMoreSign,         // 23
    FromSerialNumber,        // 24
    Reserved,                // 25
    Reserved,                // 26
    Reserved,                // 27
    Reserved,                // 28
    Reserved,                // 29
    OkCheckWPowerOnFlag,     // 30
    FormParamsPacket1,       // 31
    OkCheck,                 // 32
    WaitForMoreSign,         // 33
    SendParamsPacket34,      // 34
    OkCheck,                 // 35
    FormParamsPacket2,       // 36
    OkCheck,                 // 37
    WaitForMoreSign,         // 38
    SendParamsPacket39,      // 39
    OkCheck,                 // 40
    FormGetRequest41,        // 41
    OkCheck,                 // 42
    WaitForMoreSign,         // 43
    SendGetRequest44,        // 44
    OkCheck,                 // 45
    FormGetRequest46,        // 46
    OkCheck,                 // 47
    WaitForMoreSign,         // 48
    SendGetRequest49,        // 49
    Reserved,                // 50
    GetIP51,                 // 51
    OkCheck,                 // 52
    GetIP53,                 // 53
    RunCheck,                // 54
    Delay                    // 55
};

void wifi_init(void)
{
   if (wifi_param.w.wifi_flags & 0x01) // по умолчанию как точка доступа (к ней подключается планшет)
   {
      if (wifi_setup_zone != 99)
         HandleAccessPointMode[wifi_setup_zone]();
      else
         RESET_SEND_DATA_TO_SERVER;
   }
   else // подключение к роутеру
   {
      if (wifi_setup_zone != 99)
         HandleConnToRouterMode[wifi_setup_zone]();
      else
         RESET_SEND_DATA_TO_SERVER;
   }

   if (fResp == WRUN)
   {
      if (cntr_timeout > wifi_timeout)
      {
         fResp = WTIMEOUT;
         connectionState = UART2TCP_STANBY;
      }
   }
}

/* Contents of this function strongly depends on the device.
 * Include all nescesarry files, with flags, modes, device - unique variables.*/
void HandleWifi(void)
{
   if (time_restart > 2 || serial_number != 0)
   {
      if (wifi_param.w.wifi_flags & 0x04) // 17.03
      {
         if (!SEND_DATA_TO_SERVER)
            ESP8266_SendData();

         wifi_init();
      }
      else if (wifi_setup_zone != 99)
         wifi_setup_zone = 99;
   }

   ProcessPacketData();

   if (PACKET2_COMPLETE && (!SEND_DATA_TO_SERVER) && (!SEND_DATA_TO_TABLET) && (connectionState == UART2TCP_STANBY))
   {
      RESET_PACKET2_COMPLETE;
      ReplacePacket2();
   }

   if (REPEAT_LAST_PACKET && (!SEND_DATA_TO_SERVER) && (!SEND_DATA_TO_TABLET) && (connectionState == UART2TCP_STANBY))
   {
      RESET_REPEAT_LAST_PACKET;
      TransmitPacket2();
   }

   if (STA_REQUEST && (!SEND_DATA_TO_SERVER) && (!SEND_DATA_TO_TABLET) && (connectionState == UART2TCP_STANBY))
   {
      RESET_STA_REQUEST;
      SET_STA_REQUEST_IN_PORGRESS;
      stage = 0;
      wifi_sta_select_mode = 0;
      count_name_sta_wifi = 0;
      current_name_sta_wifi = 0;
      countCWLAP = 0;
      WiFiSTAs[0][0] = 0;
      ESP8266_SendCommand(USART_WiFi, "AT+CWLAP", 10000);
   }

   /* In every device there is a variable that displays current mode, use it here.
    * WRITE_FIRMWARE_PECH_MODE should be declared with other modes declaration, include header if nescessary.
    */
   // if (mode[current_mode] == WRITE_FIRMWARE_PECH_MODE) // uncomment this inserting your "modes" variable and mode that dedicated to WRITE_FIRMWARE_PECH
   {
      if (write_firmware_pech_zone > 1)
      {
         RX_recount = 0;
         SET_START_BYTE;
      }

      if (write_firmware_pech_zone == 5)
      {
         if (uart_rec_byte == ')')
            write_firmware_pech_zone = 7;

         if (BYTES_TRANSFERRED)
         {
            RESET_BYTES_TRANSFERRED;
            uart_rec_byte = 0;

            if (cntr_firmware_length < firmware_length)
            {
               for (end_trans_buff = 0; end_trans_buff < 64; end_trans_buff++, cntr_firmware_length++)
               {
                  trans_buff[end_trans_buff] = (*(u8 *)(START_MIRROR_FIRMWARE + cntr_firmware_length));
                  if (trans_buff[end_trans_buff] == 0x0D)
                  {
                     cntr_firmware_length += 2;
                     break;
                  }
               }

               while (!(USART_Boards->SR & USART_FLAG_TC))
                  ;

               packet_count = 0;
               USART_SendData(USART_Boards, trans_buff[packet_count++]);
               USART_ITConfig(USART_Boards, USART_IT_TXE, ENABLE);
            }
            else
            {
               write_firmware_pech_zone++;
            }
         }
      }
   }
}

void InitESPVariables(void)
{
   currentIP[0] = 'E';
   currentIP[1] = 'r';
   currentIP[2] = 'r';
   connectionState = UART2TCP_STANBY;
   wifi_senddata_zone = 99;
   wifi_setup_zone = 0;
   firmware_device_type = 0;
   time_restart = 0;
}