
#ifdef STM32L1XX_MD
#include "stm32l1xx.h" // Here you should put your main microcontroller header, this header for VS code to understand u8, u16 .. etc.
#else
#include "stm32f10x.h" // Here you should put your main microcontroller header, this header for VS code to understand u8, u16 .. etc.
#endif
#include "packet.h"
#include "esp8266.h"
#include "flash.h"
#include "communication.h"

const u8 _ok[] = {13, 10, 'O', 'K', 13, 10};                   // OK 0x0D 0x0A
const u8 _error[] = {13, 10, 'E', 'R', 'R', 'O', 'R', 13, 10}; // 0x0D 0x0A ERROR 0x0D 0x0A
const u8 _send_ok[] = {13, 10, 'S', 'E', 'N', 'D', ' ', 'O', 'K', 13, 10};
const u8 _send_fail[] = {13, 10, 'S', 'E', 'N', 'D', ' ', 'F', 'A', 'I', 'L', 13, 10};
vu8 cntr_ok = 0, cntr_error = 0, cntr_send_ok = 0, cntr_send_fail = 0;

const u8 plusIPD[] = "\xD\xA+IPD,";                                                                            // "\xD\xA+IPD,"
const u8 plusCWLAP[] = {'+', 'C', 'W', 'L', 'A', 'P', ':'};                                                    //"+CWLAP:"
const u8 ClosedSSL[] = {'4', ',', 'C', 'L', 'O', 'S', 'E', 'D'};                                               // "4,CLOSED"
const u8 HTTP200OK[] = {' ', '2', '0', '0', ' ', 'O', 'K'};                                                    // " 200 OK"
const u8 xDxAxDxA[] = {13, 10, 13, 10};                                                                        // 0x0D 0x0A 0x0D 0x0A
const u8 ContentLength40[] = {'C', 'O', 'N', 'T', 'E', 'N', 'T', '-', 'L', 'E', 'N', 'G', 'T', 'H', ':', ' '}; // "Content-Length: 40"
const u8 Date_[] = {13, 10, 'D', 'A', 'T', 'E', ':', ' '};                                                     // "\xD\xADate: "
const u8 listShortMonth[][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
vu32 countIPD = 0;
vu32 lenOfIPD = 0, receivedLenOfIPD = 0;
vu8 dataOfIPD[0xFF] = {0};
vu8 countClosedSSL = 0;
vu8 cntr_content_length = 0;
vu32 lenOfContent = 0;
vu8 charIDofIPD[3] = {0, 0, 0};
u8 charLenOfIPD[5] = {0, 0, 0, 0, 0};
u8 contentLength[5] = {0, 0, 0, 0, 0};
u8 charLenWiFiPower[4] = {0, 0, 0, 0};
vu8 commaFlag = 0;
u8 commaPos = 0;
vu8 cntr_incoming_byte = 0;
vu8 parce_date_zone = 0, cntr_parce_date = 0;

vu8 tempIDofIPD = 0;

vu8 time[7] = {0}; // date and time

vu32 cntr_fails;

//                                           HGFEDCBA BAFGDECH
const u16 sym[] = {
    0x1100, // 0xc000,	// 0 11000000 00010001
    0x7D00, // 0xf900,	// 1 11111001 01111101
    0x2300, // 0xa400,	// 2 10100100 00100011
    0x2500, // 0xb000,	// 3 10110000 00100101
    0x4D00, // 0x9900,	// 4 10011001 01001101
    0x8500, // 0x9200,	// 5 10010010 10000101
    0x8100, // 0x8200,	// 6 10000010 10000001
    0x3D00, // 0xf800,	// 7 11111000 00111101
    0x0100, // 0x8000,	// 8 10000000 00000001
    0x0500, // 0x9000,	// 9 10010000 00000101
    0xff00, //               11111111 11111111
};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*
 * inline void ControlOfResponse(u8 rec_byte)
 * Функция распознает ответы OK и ERROR при AT-запросах
 * "\xD\xAOK\xD\xA"
 * "\xD\xAERROR\xD\xA"
 */
vu8 okCount = 0;
vu8 errorCount = 0;

vu32 CharToLong(u8 *str);

/* Detects response form ESP, does not work yet. Makes very long delay.*/
ATcommandStatus DetectResponse(u8 rec2_byte, u8 *counter, const u8* expectedResponce)
{
   if (rec2_byte == expectedResponce[*counter])
   {
      if (++(*counter) >= sizeof(expectedResponce))
      {
         *counter = 0;
         return 1;
      }
      return 0;
   }
   else
   {
      *counter = 0;
      if (rec2_byte == expectedResponce[*counter])
         *counter++;
      return 0;
   }
}

inline void ControlOfResponse(u8 rec_byte)
{

   if (responseCount < sizeof(Response) - 1)
   {
      Response[responseCount++] = rec_byte;
      Response[responseCount] = 0;
   }

   if (rec_byte == _ok[cntr_ok])
   {
      if (++cntr_ok >= sizeof(_ok))
      {
         cntr_ok = 0;
         fResp = WOK;
         connectionState = UART2TCP_STANBY;
         if (STA_REQUEST_IN_PORGRESS)
         {
            RESET_STA_REQUEST_IN_PORGRESS;
            STA_to_send = current_name_sta_wifi;
         }
      }
   }
   else
   {
      cntr_ok = 0;

      if (rec_byte == _ok[cntr_ok])
         cntr_ok++;
   }

   if (rec_byte == _error[cntr_error])
   {
      if (++cntr_error >= sizeof(_error))
      {
         cntr_error = 0;
         fResp = WERROR;
         connectionState = UART2TCP_STANBY;
      }
   }
   else
   {
      cntr_error = 0;

      if (rec_byte == _error[cntr_error])
         cntr_error++;
   }

   if (countCWLAP >= sizeof(plusCWLAP))
   {
      if (rec_byte == 13)
      {
         countCWLAP = 0;
         cwlap_mode = 0;
         charLenWiFiPower[0] = 0;
      }
      else
      {
         switch (cwlap_mode)
         {
         case 0:
            if (rec_byte == '"')
            {
               cwlap_mode++;
               count_name_sta_wifi = 0;
            }
            break;
         case 1:
            if (rec_byte == '"')
            {
               cwlap_mode++;
               WiFiSTAs[current_name_sta_wifi][count_name_sta_wifi] = 0;
            }
            else
            {
               WiFiSTAs[current_name_sta_wifi][count_name_sta_wifi] = rec_byte;
               if (count_name_sta_wifi < 32)
                  count_name_sta_wifi++;
            }
            break;
         case 2:
            if (rec_byte == '-')
            {
               cwlap_mode++;
               count_power_sta_wifi = 0;
            }
            break;
         case 3:
            if (rec_byte == ',')
            {
               charLenWiFiPower[count_power_sta_wifi] = 0;
               if (CharToLong(&charLenWiFiPower[0]) < 85)
               {
                  if (current_name_sta_wifi < 9)
                     current_name_sta_wifi++;
               }
               else
               {
                  WiFiSTAs[current_name_sta_wifi][0] = 0;
               }
               cwlap_mode++;
            }
            else
            {
               charLenWiFiPower[count_power_sta_wifi++] = rec_byte;
            }
            break;
         default:
            break;
         }
      }
   }
   else
   {
      if (rec_byte == plusCWLAP[countCWLAP])
      {
         countCWLAP++;
      }
      else if (rec_byte == plusCWLAP[0])
      {
         countCWLAP = 1;
         cwlap_mode = 0;
      }
      else
      {
         countCWLAP = 0;
         cwlap_mode = 0;
      }
   }
}

inline u8 CharToDec(u8 *str)
{
   u8 cntOfSym = 0;
   while (str[cntOfSym] != 0)
   {
      cntOfSym++;
   }
   switch (cntOfSym)
   {
   case 1:
      return str[0] - 0x30;
   case 2:
      return (str[0] - 0x30) * 10 + (str[1] - 0x30);
   case 3:
      return (str[0] - 0x30) * 100 + (str[1] - 0x30) * 10 + (str[2] - 0x30);
   default:
      return 0;
   }
}

// inline void PacketDetect(u8 rec2_byte)
//{
//    if (PACKET2_FF)
//    {
//       if (rec2_byte == 0xFF)
//       {
//          if (packet2rx_count)
//          {
//             ptr2_byte--;
//             packet2rx_count--;
//          }
//          RESET_PACKET2_FF;
//       }
//       else
//       {
//          packet2_check = rec2_byte;
//          ptr2_byte = &packet2rx.device_num;
//          packet2rx_count = 1;
//          RESET_PACKET2_FF;
//       }
//    }
//    else
//    {
//       packet2_check += rec2_byte;
//
//       if (rec2_byte == 0xFF)
//          SET_PACKET2_FF;
//       else
//          RESET_PACKET2_FF;
//    }
//
//    *ptr2_byte++ = rec2_byte;
//
//    if (++packet2rx_count >= MAX_DATA_SIZE)
//    {
//       packet2rx_count = 0;
//       ptr2_byte = &packet2rx.marker;
//    }
//
//    //chk = packet2rx_count;
//
//    if (packet2rx_count > 4)
//    {
//       if (packet2rx_count > packet2rx.data_size + 4)
//       {
//          if (rec2_byte == (u8)(packet2_check - rec2_byte))
//          {
//             if (wifi_param.w.wifi_flags & 0x04)
//             {
//                SET_PACKET2_COMPLETE;
//                SET_EXT_WIFI_CONNECTED;
//             }
//          }
//          packet2rx_count = 0;
//          ptr2_byte = &packet2rx.marker;
//          RESET_PACKET2_FF;
//       }
//    }
// }

vu32 CharToLong(u8 *str)
{
   u8 cntOfSym = 0;
   u8 c;
   u32 res, s;

   while (str[cntOfSym] != 0)
      cntOfSym++;

   s = 1;
   res = 0;

   for (c = cntOfSym; c > 0; c--)
   {
      res += (str[c - 1] - 0x30) * s;
      s *= 10;
   }

   return res;
}

vu32 MonthToLong(u8 *str)
{
   u32 aa, bb;

   for (aa = 0; aa < 12; aa++)
   {
      for (bb = 0; bb < 3; bb++)
      {
         if (!((str[bb] == listShortMonth[aa][bb]) || (str[bb] == listShortMonth[aa][bb] + 32)))
            break;
      }
      if (bb >= 3)
         break;
   }
   if (aa >= 12)
      aa = 0;
   else
      aa++;
   return aa;
}

inline void ParseID(u8 d)
{
   if (d == ',')
   {
      charIDofIPD[countIPD - 7] = 0;
      commaFlag++;
      commaPos = countIPD;
      countIPD++;

      tempIDofIPD = charIDofIPD[0] - '0'; // CharToLong(&charIDofIPD[0]); // Parse ID

      if (tempIDofIPD < 4)
      {
         RESET_SERVER_IPD;
         if (++cntr_RX_IPD >= IDOFIPD_BUFF_LENGTH)
            cntr_RX_IPD = 0;

         IDofIPD[cntr_RX_IPD] = tempIDofIPD; // Parse ID
      }
      else
         SET_SERVER_IPD;
   }
   else if (countIPD > 8) // If countIPD becomes more than 8, than message is incorrect, reset everything.
   {
      countIPD = 0;
      commaFlag = e_ID;
      commaPos = 0;
   }
   else // First enters here, if everything is right
   {
      charIDofIPD[countIPD - 7] = d; // Accumulate ID bytes
      countIPD++;
   }
}

inline void ParseLength(u8 d)
{
   if (d == ':')
   {
      charLenOfIPD[countIPD - commaPos - 1] = 0;
      commaFlag++;
      commaPos = countIPD;
      countIPD++;
      lenOfIPD = CharToLong(&charLenOfIPD[0]); // Parse length

      if ((SSL_CONNECTED) && SERVER_IPD)
      {
         if (FIRMWARE_DATA_PACKET_ADD)
         {
            RESET_FIRMWARE_DATA_PACKET_ADD;
            commaFlag = e_FIRMWARE;
         }
         else
         {
            commaFlag = e_HTTP200OK;
            cntr_content_length = 0;
            cntr_incoming_byte = 0;
         }
      }
   }
   else if (countIPD > commaPos + 3) // We are waiting for length not more than 3 division number, so if it is bigger that that, zero everything.
   {
      countIPD = 0;
      commaFlag = e_ID;
      commaPos = 0;
   }
   else // Enters here until all length is parsed
   {
      charLenOfIPD[countIPD - commaPos - 1] = d; // Accumulate length bytes
      countIPD++;
   }
}

inline void ParseData(u8 d)
{
   dataOfIPD[countIPD - commaPos - 1] = d; // Accumulate data bytes
   if (countIPD < 255)
      countIPD++;

   if (countIPD - commaPos == lenOfIPD + 1)
   {

      receivedLenOfIPD = lenOfIPD;
      SET_DATA_IPD_RECEIVED;

      charLenOfIPD[0] = 0;
      countIPD = 0;
      commaFlag = e_ID;
      commaPos = 0;
      lenOfIPD = 0;
   }
}

inline void CheckHTTP200OK(u8 d)
{
   countIPD++;
   if (d == 0x0D)
   {
      if (cntr_incoming_byte == sizeof(HTTP200OK)) // if everything good, go further (to contentLength)
      {
         commaFlag++;
         cntr_incoming_byte = 0;
         cntr_content_length = 0;
         contentLength[0] = 0;
      }
      else
      {
         RESET_SSL_CONNECTED;

         charLenOfIPD[0] = 0;
         countIPD = 0;
         commaFlag = e_ID;
         commaPos = 0;
         lenOfIPD = 0;
      }
   }
   else if (d == HTTP200OK[cntr_incoming_byte])
      cntr_incoming_byte++;
   else if (d == HTTP200OK[0])
      cntr_incoming_byte = 1;
   else
      cntr_incoming_byte = 0;
}

inline void ParseContentLength(u8 d)
{
   countIPD++;
   if (d == 0x0D)
   {
      if (cntr_incoming_byte >= sizeof(ContentLength40))
      {
         lenOfContent = CharToLong(&contentLength[0]);
         switch (lenOfContent)
         {
         case e_FRAGMENT_FIRMWARE_LENGTH:
            countIPD = 0;
            commaFlag = e_ID;
            charLenOfIPD[0] = 0;
            if (FRAGMENT_FIRMWARE)
            {
               RESET_FRAGMENT_FIRMWARE;
               if (firmware_device_type)
               {
                  firmware_device_type = 0;
                  time_restart = 28;
               }
            }
            break;
         case e_TELEMETRY_LEGNTH:
            commaFlag = e_TELEMETRY;
            break;
         case e_TOKEN_AND_DATE_LENGTH:
            commaFlag = e_TOKEN_AND_DATE;
            break;
         case e_FIRMWAE_LENGTH:
            commaFlag = e_FIRMWARE;
            break;
         default:
            commaFlag = e_ID;
            if (FRAGMENT_FIRMWARE)
            {
               RESET_FRAGMENT_FIRMWARE;
               commaFlag = e_FRAGMENT_FIRMWARE;
            }
            break;
         }
         cntr_content_length = 0;
         cntr_incoming_byte = 0;
      }
   }
   else if (cntr_incoming_byte < 16)
   {
      if ((d == ContentLength40[cntr_incoming_byte]) || ((d == ContentLength40[cntr_incoming_byte] + 32)))
         cntr_incoming_byte++;
      else
         cntr_incoming_byte = 0;
   }
   else
   {
      contentLength[cntr_content_length] = d;

      if (cntr_content_length < sizeof(contentLength))
      {
         cntr_content_length++;
         contentLength[cntr_content_length] = 0;
      }
   }
}

inline void ParseToken(u8 d)
{
   if (cntr_incoming_byte == sizeof(xDxAxDxA))
   {
      token40[cntr_content_length++] = d;

      if (cntr_content_length == VAL_TOKEN_LENGTH)
      {
         SET_TX_TOKEN40;
         SET_SEND_TO_SERVER_POWER_ON; // отослать версии на сервер сразу после получения токена

         charLenOfIPD[0] = 0;
         countIPD = 0;
         commaFlag = e_ID;
         cntr_incoming_byte = 0;
         cntr_content_length = 0;
      }
   }
   else if (d == xDxAxDxA[cntr_incoming_byte])
      cntr_incoming_byte++;
   else
      cntr_incoming_byte = 0;
}

/* void ParseDateTypeX(u8 d, u8 type)
 *
 * d - incoming data
 *
 * type - type of date (RTC_DAY, RTC_MONTH, RTC_YEAR, ... etc.)
 *
 * return - non
 */
void ParseDateTypeX(u8 d, u8 type)
{
   contentLength[cntr_content_length] = 0;
   switch (type)
   {
   case RTC_MONTH:
      lenOfContent = MonthToLong(&contentLength[0]);
      break;

   case RTC_HOUR:
      lenOfContent = CharToLong(&contentLength[0]) + 5;
      break;

   default:
      lenOfContent = CharToLong(&contentLength[0]);
      break;
   }
   time[type] = lenOfContent;
   parce_date_zone++;
   cntr_content_length = 0;
}

inline void ParseDate(u8 d)
{
   if (cntr_parce_date == sizeof(Date_))
   {
      switch (parce_date_zone)
      {
      case 0:
         if (d == ' ')
         {
            parce_date_zone++;
            cntr_content_length = 0;
         }
         break;

      case 1: // parce day
         if (d == ' ')
            ParseDateTypeX(d, RTC_DAY);
         else
            contentLength[cntr_content_length++] = d;
         break;

      case 2: // parce month
         if (d == ' ')
            ParseDateTypeX(d, RTC_MONTH);
         else
            contentLength[cntr_content_length++] = d;
         break;

      case 3: // skip 2 of 2019

      case 4: // skip 0 of 2019
         parce_date_zone++;
         break;

      case 5: // parce year
         if (d == ' ')
            ParseDateTypeX(d, RTC_YEAR);
         else
            contentLength[cntr_content_length++] = d;
         break;

      case 6: // parce hour
         if (d == ':')
            ParseDateTypeX(d, RTC_HOUR);
         else
            contentLength[cntr_content_length++] = d;
         break;

      case 7: // parce minute
         if (d == ':')
            ParseDateTypeX(d, RTC_MINUTE);
         else
            contentLength[cntr_content_length++] = d;
         break;

      case 8: // parce second
         if (d == ' ')
            ParseDateTypeX(d, RTC_SECOND);
         else
            contentLength[cntr_content_length++] = d;
         break;

      default:
         break;
      }
   }
   else
   {
      if ((d == Date_[cntr_parce_date]) || (d == Date_[cntr_parce_date] + 32))
         cntr_parce_date++;
      else
      {
         cntr_parce_date = 0;
         parce_date_zone = 0;
      }
   }
}

inline void ParseTelemetry(u8 d)
{
   if (cntr_incoming_byte == sizeof(xDxAxDxA))
   {
      contentLength[cntr_content_length++] = d;
      if (cntr_content_length == 2)
      {
         lenOfContent = CharToLong(&contentLength[0]);

         if (lenOfContent == 10)
            SET_TX_TELEMETRY_TO_SERVER;
         else if (lenOfContent == 11)
            RESET_TX_TELEMETRY_TO_SERVER;

         charLenOfIPD[0] = 0;
         countIPD = 0;
         commaFlag = e_ID;
         cntr_incoming_byte = 0;
         cntr_content_length = 0;
      }
   }
   else if (d == xDxAxDxA[cntr_incoming_byte])
      cntr_incoming_byte++;
   else
      cntr_incoming_byte = 0;
}

inline void ParseFragmentFirmware(u8 d)
{
   if (cntr_incoming_byte == sizeof(xDxAxDxA))
   {
      {
         firmware_data.ch[cntr_content_length++] = d;

         if (cntr_content_length == lenOfContent)
         {
            u8 ii, ss;
            u8 parts[12];

            firmware_data.ch[cntr_content_length] = 0;
            charLenOfIPD[0] = 0;
            countIPD = 0;
            commaFlag = e_ID;
            cntr_incoming_byte = 0;
            cntr_content_length = 0;

            ii = 0;
            ss = 0;

            while (firmware_data.ch[ss] != 32)
               parts[ii++] = firmware_data.ch[ss++];

            parts[ii] = 0;
            firmware_length = CharToLong(&parts[0]);
            ii = 0;
            ss++;

            while (firmware_data.ch[ss] != 32)
               parts[ii++] = firmware_data.ch[ss++];

            parts[ii] = 0;
            firmware_blocks = CharToLong(&parts[0]);
            ii = 0;
            ss++;

            while (firmware_data.ch[ss] != 0)
               parts[ii++] = firmware_data.ch[ss++];

            parts[ii] = 0;
            firmware_crc32_source = CharToLong(&parts[0]);
            cntr_firmware_block = 0;
            SET_CLEAR_BEFORE_WRITE_FIRMWARE;
            RESET_FIRMWARE_NEED_TO_ADD_LINE;
            SET_REQUEST_FIRMWARE_DATA;
         }
      }
   }
   else if (d == xDxAxDxA[cntr_incoming_byte])
      cntr_incoming_byte++;
   else
      cntr_incoming_byte = 0;
}

inline void ParseFirmware(u8 d)
{
   countIPD++;

   if (cntr_incoming_byte == sizeof(xDxAxDxA))
   {
      {
         if (cntr_content_length < 200)
            firmware_data.ch[cntr_content_length++] = d;

         if (cntr_content_length >= lenOfContent)
         {
            firmware_data.ch[cntr_content_length] = 0;
            charLenOfIPD[0] = 0;
            countIPD = 0;
            commaFlag = e_ID;
            cntr_incoming_byte = 0;
            cntr_content_length = 0;
            cntr_firmware_data = 0;

            if (cntr_firmware_block < firmware_blocks)
            {
               cntr_firmware_block++;
               SET_WRITE_DOWNLOADED_FIRMWARE_TO_FLASH;
            }
         }
         else if (countIPD >= lenOfIPD + commaPos + 1)
         {
            charLenOfIPD[0] = 0;
            countIPD = 0;
            commaFlag = e_ID;
            SET_FIRMWARE_DATA_PACKET_ADD;
         }
      }
   }
   else if (d == xDxAxDxA[cntr_incoming_byte])
      cntr_incoming_byte++;
   else if (d == xDxAxDxA[0])
      cntr_incoming_byte = 1;
   else
      cntr_incoming_byte = 0;
}

/*
 * inline void DetectMessage(u8 d)
 * * Функция мониторит пакеты, получаемые через TCP-канал.
 * Обрабатывает на ходу, сохраняя только полезную часть в память (Zero copy method).
 *
 * * Формат приходящих пакетов:
 * 0x0D|0x0A|+IPD,<ID>,<len>:<data>
 *
 * * Например:
 * 0x0D|0x0A|+IPD,0,5:|0xFF|0x02|0x01|0x00|0x03
 *
 * * Задача функции - вычленять полезную информацию, отбрасывать заголовки.
 */
inline void DetectMessage(u8 d)
{
   if (countIPD > 6)
   {
      /* FOR DEBUG */
      // if ((SSL_CONNECTED) && SERVER_IPD)
      // {
      //    if (cntr_debug >= 16)
      //       cntr_debug = 0;
      //    _debug[cntr_debug++] = d;
      // }

      switch (commaFlag)
      {
      case e_ID:
         ParseID(d);
         break;

      case e_LENGTH:
         ParseLength(d);
         break;

      case e_DATA:
         ParseData(d);
         break;

      case e_HTTP200OK:
         CheckHTTP200OK(d);
         break;

      case e_CONTENT_LENGTH:
         ParseContentLength(d);
         break;

      case e_TOKEN_AND_DATE:
         ParseToken(d);
         ParseDate(d);
         break;

      case e_TELEMETRY:
         ParseTelemetry(d);
         break;

      case e_FRAGMENT_FIRMWARE:
         ParseFragmentFirmware(d);
         break;

      case e_FIRMWARE:
         ParseFirmware(d);
         break;

      default:
         countIPD = 0;
         commaFlag = e_ID;
         cntr_incoming_byte = 0;
         lenOfIPD = 0;
         cntr_content_length = 0;
         break;
      }
   }
   else if (d == plusIPD[countIPD]) // Waiting for "0x0D0x0A+IPD,"
      countIPD++;
   else if (d == plusIPD[0])
      countIPD = 1;
   else
      countIPD = 0;
}

/* Detects different ESP responses, depending in connectionState */
inline void  HandleConnectionState(u8 rec2_byte)
{
   switch (connectionState)
   {
   case ATCMD:
      if (fResp == WRUN)
         ControlOfResponse(rec2_byte);
      break;

   case UART2TCP_SEND:
      if (rec2_byte == _send_ok[cntr_send_ok])
      {
         if (++cntr_send_ok >= sizeof(_send_ok))
         {
            cntr_send_ok = 0;
            fResp = WERROR;
            connectionState = UART2TCP_STANBY;
         }
      }
      else
      {
         cntr_send_ok = 0;

         if (rec2_byte == _send_ok[cntr_send_ok])
            cntr_send_ok++;
      }

      if (rec2_byte == _send_fail[cntr_send_fail])
      {
         if (++cntr_send_fail >= sizeof(_send_fail))
         {
            if (++cntr_fails >= FAILS_UNTIL_RESTART)
            {
               cntr_fails = 0;
               wifi_setup_zone = 99;
            }

            cntr_send_fail = 0;
            fResp = WERROR;
            connectionState = UART2TCP_STANBY;

            if (DOWNLOAD_FIRMWARE_IN_PROGRESS)
               SET_REQUEST_FIRMWARE_DATA;

            if (SEND_DATA_TO_TABLET && EXT_WIFI_CONNECTED)
               wifi_senddata_zone = 10;
         }
      }
      else
      {
         cntr_send_fail = 0;

         if (rec2_byte == _send_fail[cntr_send_fail])
            cntr_send_fail++;
      }
      break;
   
   default:
      break;
   }
}

inline void DetectSSLClosed(u8 rec2_byte)
{
   if (SSL_CONNECTED)
   {
      if (rec2_byte == 0x0D)
      {
         if (countClosedSSL == sizeof(ClosedSSL))
         {
            countClosedSSL = 0;
            RESET_SSL_CONNECTED;
         }
      }
      else if (rec2_byte == ClosedSSL[countClosedSSL])
         countClosedSSL++;
      else
         countClosedSSL = 0;
   }
}

void ProcessESPData(USART_TypeDef* USARTx)
{
   vu8 rec2_byte;
   rec2_byte = (u8)USART_ReceiveData(USARTx);

   /* It detects > sign, that indicates, that ESP is ready to receive data*/
   if (rec2_byte == '>')
      SET_WIFI_MORE_MESSAGE;

   HandleConnectionState(rec2_byte);

   DetectMessage(rec2_byte);

   DetectSSLClosed(rec2_byte);
}

void SendDataToESP(USART_TypeDef* USARTx)
{
   if (packet2tx_count >= end_trans_buff2)
   {
      packet2tx_count = 0;
      USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);

      if (connectionState == UART2TCP_SEND)
      {
         RESET_SEND_DATA_TO_TABLET;
         RESET_SEND_DATA_TO_SERVER;
      }
   }
   else
   {
      USART_SendData(USARTx, trans_buff2[packet2tx_count++]);

      if (packet2tx_count >= MAX_DATA_SIZE * 4)
         packet2tx_count = 0;
   }
}

inline void FilterOutFFs(u8 rec_byte)
{
   if (PACKET_FF)
   {
      if (rec_byte == 0xFF)
      {
         if (packet_count)
         {
            ptr_byte--;
            packet_count--;
         }

         RESET_PACKET_FF;
      }
      else
      {
         packet_check = rec_byte;
         ptr_byte = &packet.device_num;
         packet_count = 1;
         RESET_PACKET_FF;
      }
   }
   else
   {
      packet_check += rec_byte;

      if (rec_byte == 0xFF)
         SET_PACKET_FF;
      else
         RESET_PACKET_FF;
   }
}

void ProcessCBData(USART_TypeDef* USARTx)
{
   u8 rec_byte;
   /* Read one byte from the receive data register */
   rec_byte = (u8)USART_ReceiveData(USARTx);
   uart_rec_byte = rec_byte;

   if (write_firmware_pech_zone != 5)
   {
      if (USARTx->SR & 0x08) //(OERR)  // Проверяем ошибку скорости
      {}
      else if (USARTx->SR & 0x02) // (FERR) Проверяем ошибку кадра
      {}
      else // Если ошибки не было
      {
         FilterOutFFs(rec_byte);

         *ptr_byte++ = rec_byte;

         if (++packet_count >= MAX_DATA_SIZE)
         {
            packet_count = 0;
            ptr_byte = &packet.marker;
         }

         if (packet_count > 4)
         {
            if (packet_count > packet.data_size + 4)
            {
               if (rec_byte == (u8)(packet_check - rec_byte))
                  SET_REPLACE_PACKET;

               packet_count = 0;
               ptr_byte = &packet.marker;
               RESET_PACKET_FF;
            }
         }
      }
   }
}

void SendDataToCB(USART_TypeDef* USARTx)
{
   if (packet_count >= end_trans_buff)
   {
      packet_count = 0;
      USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
      SET_BYTES_TRANSFERRED;
   }
   else
      USART_SendData(USARTx, trans_buff[packet_count++]);
}

inline void PacketDetect(u8 rec2_byte)
{
   if (PACKET2_FF)
   {
      if (rec2_byte == 0xFF)
      {
         if (packet2rx_count)
         {
            ptr2_byte--;
            packet2rx_count--;
         }
         RESET_PACKET2_FF;
      }
      else
      {
         packet2_check = rec2_byte;
         ptr2_byte = &packet2rx.device_num;
         packet2rx_count = 1;
         RESET_PACKET2_FF;
      }
   }
   else
   {
      packet2_check += rec2_byte;

      if (rec2_byte == 0xFF)
         SET_PACKET2_FF;
      else
         RESET_PACKET2_FF;
   }

   *ptr2_byte++ = rec2_byte;

   if (++packet2rx_count >= MAX_DATA_SIZE)
   {
      packet2rx_count = 0;
      ptr2_byte = &packet2rx.marker;
   }

   if (packet2rx_count > 4)
   {
      if (packet2rx_count > packet2rx.data_size + 4)
      {
         if (rec2_byte == (u8)(packet2_check - rec2_byte))
         {
            if (wifi_param.w.wifi_flags & 0x04)
            {
               SET_PACKET2_COMPLETE;
               SET_EXT_WIFI_CONNECTED;
            }
         }
         packet2rx_count = 0;
         ptr2_byte = &packet2rx.marker;
         RESET_PACKET2_FF;
      }
   }
}

void ProcessPacketData(void)
{
   if (DATA_IPD_RECEIVED)
   {
      RESET_DATA_IPD_RECEIVED;

      for (u8 ii = 0; ii < receivedLenOfIPD; ii++)
         PacketDetect(dataOfIPD[ii]);
   }
}

