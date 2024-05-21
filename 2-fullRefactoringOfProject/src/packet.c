
#ifdef STM32L1XX_MD
#include "stm32l1xx.h"
#else
#include "stm32f10x.h"
#endif
#include "packet.h" // Файл с определениями структуры пакетов
#include "esp8266.h"

/* User includes */
#include "def.h" // Файл с определениями структур и констант
#include "glob.h"
#include "programs.h"
#include "flash.h"

/*Flag variables*/
/* Variables that used elsewhere */
volatile struct template_packets packet, packet2rx, packet2tx;

vu8 *ptr_byte;
vu8 *ptr2_byte;
vu8 trans_buff[MAX_DATA_SIZE * 2];
vu8 end_trans_buff;
vu8 trans_buff2[MAX_DATA_SIZE * 4];
vu32 end_trans_buff2;
vu8 packet_count;
vu32 packet2rx_count;
vu32 packet2tx_count;
vu32 RX_recount, rx2_timeout;
u8 masterIDofIPD;
vu8 packet_check;
vu8 packet2_check;
vu32 rx_wifi_timeout;

u8 STA_to_send; // to send available stas

void TransmitByte(u8 byte) // output one byte
{
   trans_buff[end_trans_buff++] = byte;
}

void TransmitByte2(u8 byte) // output one byte
{
   trans_buff2[end_trans_buff2++] = byte;
}

void TransmitPacket(USART_TypeDef *USARTx)
{
   u8 count_data;
   u8 check_summ; // Обнулить контрольную сумму

   end_trans_buff = 0;
   packet.device_num = 1;

   TransmitByte(0xFF);               // Поместить "Сигнатура устройства" в буфер порта RS232
   TransmitByte(packet.device_num);  // Поместить "Номер устройства (сетевой адрес)" в буфер порта RS232
   TransmitByte(packet.command_num); // Поместить "Номер команды" в буфер порта RS232
   TransmitByte(packet.data_size);   // Поместить "Размер блока данных" в буфер порта RS232

   check_summ = packet.device_num;
   check_summ += packet.command_num;
   check_summ += packet.data_size;

   if (packet.data_size > 0)
   {
      for (count_data = 0; count_data < packet.data_size; count_data++)
      {
         check_summ += packet.packet_data.data[count_data]; // Подсчитать контрольную сумму данных
         TransmitByte(packet.packet_data.data[count_data]); // Блок данных

         if (packet.packet_data.data[count_data] == 0xFF)
            TransmitByte(0xFF); // Дублировать код 0xFF
      }
   }

   TransmitByte(check_summ); // Поместить "Контрольная сумма" пакета в буфер порта RS232

   while (!(USARTx->SR & USART_SR_TC))
      ;

   USART_SendData(USARTx, trans_buff[packet_count++]);
   USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
}

void TransmitPacket2(void)
{
   u8 count_data;
   u8 check_summ; // Обнулить контрольную сумму

   end_trans_buff2 = 0;
   packet2tx_count = 0;
   packet2tx.device_num = 1;

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
   wifi_senddata_zone = 10;
   SET_SEND_DATA_TO_TABLET;
   //   SET_BUSY_WIFI_UART;
}

void FWTransmitPacket(USART_TypeDef *USARTx)
{
   u8 count_data;
   u8 check_summ; // Обнулить контрольную сумму

   rx2_timeout = 0;
   packet_count = 0;
   end_trans_buff = 0;
   packet2rx.device_num = 1;

   TransmitByte(0xFF);                  // Поместить "Сигнатура устройства" в буфер порта RS232
   TransmitByte(packet2tx.device_num);  // Поместить "Номер устройства (сетевой адрес)" в буфер порта RS232
   TransmitByte(packet2tx.command_num); // Поместить "Номер команды" в буфер порта RS232
   TransmitByte(packet2tx.data_size);   // Поместить "Размер блока данных" в буфер порта RS232

   check_summ = packet2tx.device_num;
   check_summ += packet2tx.command_num;
   check_summ += packet2tx.data_size;

   if (packet2tx.data_size > 0)
   {
      for (count_data = 0; count_data < packet2tx.data_size; count_data++)
      {
         check_summ += packet2tx.packet_data.data[count_data]; // Подсчитать контрольную сумму данных
         TransmitByte(packet2tx.packet_data.data[count_data]); // Блок данных

         if (packet2tx.packet_data.data[count_data] == 0xFF)
            TransmitByte(0xFF); // Дублировать код 0xFF
      }
   }

   TransmitByte(check_summ); // Поместить "Контрольная сумма" пакета в буфер порта RS232

   while (!(USARTx->SR & 0x40))
      ;

   USART_SendData(USARTx, trans_buff[packet_count++]);
   USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
}

void FWTransmitPacket2(void)
{
   u8 count_data;
   u8 check_summ; // Обнулить контрольную сумму

   //  ptr_byte2 = &packet2.marker; /* Установить указатель на позицию номера устройства в пакете */
   end_trans_buff2 = 0;
   packet2tx_count = 0;
   packet.device_num = 1;

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
                              //  if(check_summ == 0xFF) TransmitByte2(0xFF); // Дублировать код 0xFF

   wifi_senddata_zone = 0;
}

void TransmitTermopress(u8 NoCom)
{
   u8 data;
   u16 prs;

   RX_recount = 0;
   SET_START_BYTE;

   packet.command_num = NoCom; // Номер команды

   switch (NoCom)
   {
   case 8:
      packet.data_size = 4;
      packet.packet_data.data[0] = LOW_L(serial_number);
      packet.packet_data.data[1] = HIGH_L(serial_number);
      packet.packet_data.data[2] = LOW_H(serial_number);
      packet.packet_data.data[3] = HIGH_H(serial_number);
      break;

   case 17:
      packet.data_size = 2;
      packet.packet_data.idata[0] = Tstart;
      break;

   case 21:
      packet.data_size = 10;
      packet.packet_data.data[0] = HIGH(tar_prm.cold_bias);
      packet.packet_data.data[1] = LOW(tar_prm.cold_bias);
      packet.packet_data.data[2] = HIGH(tar_prm.couple_bias);
      packet.packet_data.data[3] = LOW(tar_prm.couple_bias);
      packet.packet_data.data[4] = HIGH(tar_prm.couple_kfc);
      packet.packet_data.data[5] = LOW(tar_prm.couple_kfc);
      packet.packet_data.data[6] = HIGH(tar_prm.press_bias);
      packet.packet_data.data[7] = LOW(tar_prm.press_bias);
      packet.packet_data.data[8] = HIGH(tar_prm.press_kfc);
      packet.packet_data.data[9] = LOW(tar_prm.press_kfc);
      break;

   case 30:
      packet.data_size = 10;
      prs = prg_prm[prg_num].pressure / 10;

      if (prs > 60)
         prs = 250;

      packet.packet_data.data[0] = HIGH(prs);
      packet.packet_data.data[1] = LOW(prs);
      packet.packet_data.data[2] = HIGH(prg_prm[prg_num].temperature);
      packet.packet_data.data[3] = LOW(prg_prm[prg_num].temperature);
      packet.packet_data.data[4] = HIGH(prg_prm[prg_num].time_hold);
      packet.packet_data.data[5] = LOW(prg_prm[prg_num].time_hold);
      packet.packet_data.data[6] = HIGH(prg_prm[prg_num].time_press);
      packet.packet_data.data[7] = LOW(prg_prm[prg_num].time_press);

      data = 0;

      if (!prg_prm[prg_num].flags.auto_mode)
         data |= 0x01;

      if ((!prg_prm[prg_num].flags.auto_mode) && ((prm_num == PRM_PRESS) || (prm_num == PRM_TIME_PRESS) || (prm_num == PRM_FAN)))
         data |= 0x02;

      if (prg_prm[prg_num].flags.flow)
         data |= 0x04; // if(!FLOW) data |= 0x04;

      if (!prg_prm[prg_num].flags.heatOnly)
         data |= 0x08;

      if (!prg_prm[prg_num].flags.poluavtomat)
         data |= 0x10;

      // packet.packet_data.data[8] = HIGH(prg_prm[prg_num].wflags);
      // packet.packet_data.data[9] = LOW(prg_prm[prg_num].wflags);
      packet.packet_data.data[8] = 0;
      packet.packet_data.data[9] = data;
      break;

   case 32:
      packet.data_size = 3;
      packet.packet_data.idata[0] = prc_time;
      packet.packet_data.data[2] = count_zone;
      break;

   case e_SEND_TEST_CYLINDER:
      packet.data_size = 3;
      packet.packet_data.idata[0] = cylinderTestParams.params.temperature;
      packet.packet_data.data[2] = cylinderTestParams.params.pressure;
      break;

   case e_SEND_MAINTAIN_TEMPERATURE:
      packet.data_size = 2;
      packet.packet_data.idata[0] = maintain_temperature;
      break;

   case 98:
      packet.data_size = VAL_TOKEN_LENGTH;

      for (data = 0; data < VAL_TOKEN_LENGTH; data++)
         packet.packet_data.data[data] = token40[data];

      break;

   default:
      packet.data_size = 0;
      packet.packet_data.data[0] = 0; /* Команда неизвестна */
      break;
   }

   TransmitPacket(USART_Boards);
}

void ReplacePacket(void)
{
   switch (packet.command_num)
   {
   case 1:                        // Запрос версии ПО
      if (packet.data_size == 11) /* Размер блока данных соответствует команде */
      {
         RESET_RX_VERSION;
         termopress_id = packet.packet_data.data[1];
         major_firmware = packet.packet_data.data[2];
         minor_firmware = packet.packet_data.data[3];
         version_day = packet.packet_data.data[4];
         version_month = packet.packet_data.data[5];
         version_year = packet.packet_data.data[6];
         LOW_L(serial_number) = packet.packet_data.data[7];
         HIGH_L(serial_number) = packet.packet_data.data[8];
         LOW_H(serial_number) = packet.packet_data.data[9];
         HIGH_H(serial_number) = packet.packet_data.data[10];
         if (serial_number > 999999UL)
            serial_number = 0;
      }
      break;

   case 2:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_RESET;
      break;

   case 3:
      if (packet.data_size == 10)
      {
         RESET_RX_STATUS;
         LOW(mes_temperature) = packet.packet_data.data[0];
         HIGH(mes_temperature) = packet.packet_data.data[1];
         LOW(mes_pressure) = packet.packet_data.data[2];
         HIGH(mes_pressure) = packet.packet_data.data[3];
         mes_pressure *= 10;
         termopress_mode = packet.packet_data.data[4];
         alert_flag0 = packet.packet_data.data[5];
         LOW(real_room_temperature) = packet.packet_data.data[6];
         HIGH(real_room_temperature) = packet.packet_data.data[7];

         if (alertDelay != 0)
            alert_flag0 = 0;
      }
      break;

   case 4:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_VENTING;
      break;

   case 6:
      if (packet.data_size == 0)
         RESET_TX_VALVE1;
      break;

   case 7:
      if (packet.data_size == 0)
         RESET_TX_VALVE2;
      break;

   case 8:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_SERIAL_NUMBER;
      break;

   case 11:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_PRESSURE_ADJUST;
      break;

   case 13:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_NEXT_ZONE;
      break;

   case 14:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_TEMPERATURE_ADJUST;
      break;

   case 15:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_VENTING_OFF;
      break;

   case 16:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_VENTING_ON;
      break;

   case 17:
      if (packet.data_size == 1)
         //if (packet.packet_data.data[0] == 1)
            RESET_TX_TSTART;
      break;

   case 18:
      if (packet.data_size == 2)
      {
         Tstart = packet.packet_data.idata[0];
         RESET_RX_TSTART;
      }
      break;

   case 20:
      if (packet.data_size == 10)
      {
         RESET_RX_FACTORS;
         LOW(tar_prm.cold_bias) = packet.packet_data.data[0];
         HIGH(tar_prm.cold_bias) = packet.packet_data.data[1];
         LOW(tar_prm.couple_bias) = packet.packet_data.data[2];
         HIGH(tar_prm.couple_bias) = packet.packet_data.data[3];
         LOW(tar_prm.couple_kfc) = packet.packet_data.data[4];
         HIGH(tar_prm.couple_kfc) = packet.packet_data.data[5];
         LOW(tar_prm.press_bias) = packet.packet_data.data[6];
         HIGH(tar_prm.press_bias) = packet.packet_data.data[7];
         LOW(tar_prm.press_kfc) = packet.packet_data.data[8];
         HIGH(tar_prm.press_kfc) = packet.packet_data.data[9];
      }
      break;

   case 21:
      if (packet.data_size == 1)
         RESET_TX_FACTORS;
      break;

   case 22:
      if (packet.data_size == 6)
      {
         HIGH(adc_data.adc_temperature) = packet.packet_data.data[0];
         LOW(adc_data.adc_temperature) = packet.packet_data.data[1];
         HIGH(adc_data.adc_pressure) = packet.packet_data.data[2];
         LOW(adc_data.adc_pressure) = packet.packet_data.data[3];
         HIGH(adc_data.adc_couple) = packet.packet_data.data[4];
         LOW(adc_data.adc_couple) = packet.packet_data.data[5];
         RESET_RX_ADC_DATA;
      }
      break;

   case 23:
      if (packet.data_size == 1)
         RESET_TX_BIAS_FACTOR_UP;
      break;

   case 24:
      if (packet.data_size == 1)
         RESET_TX_BIAS_FACTOR_DN;
      break;

   case 25:
      if (packet.data_size == 1)
         RESET_TX_KFC_FACTOR_UP;
      break;

   case 26:
      if (packet.data_size == 1)
         RESET_TX_KFC_FACTOR_DN;
      break;

   case 27:
      if (packet.data_size == VAL_STATISTICS)
      {
         RESET_RX_STATISTICS;
         HIGH_H(statistics.w.cntr_heater) = packet.packet_data.data[0];
         LOW_H(statistics.w.cntr_heater) = packet.packet_data.data[1];
         HIGH_L(statistics.w.cntr_heater) = packet.packet_data.data[2];
         LOW_L(statistics.w.cntr_heater) = packet.packet_data.data[3];
         HIGH(statistics.w.cntr_termopresses) = packet.packet_data.data[4];
         LOW(statistics.w.cntr_termopresses) = packet.packet_data.data[5];
         HIGH(statistics.w.cntr_pnevmo) = packet.packet_data.data[6];
         LOW(statistics.w.cntr_pnevmo) = packet.packet_data.data[7];
         statistics.w.cntr_press_toggle_state = packet.packet_data.data[8];
      }
      break;

   case 30:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_RUN_PROG;
      break;

   case e_PROG:
      if (packet.data_size == 5)
      {
         RESET_RX_PROG;
         LOW(time_zone) = packet.packet_data.data[0];
         HIGH(time_zone) = packet.packet_data.data[1];
         count_zone = packet.packet_data.data[2];
         LOW(duration) = packet.packet_data.data[3];
         HIGH(duration) = packet.packet_data.data[4];
      }
      break;

   case 32:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_CONTINUE_PROG;
      break;

   case 33:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_CARTRIDGE_INSTALLED;
      break;

   case e_SEND_TEST_CYLINDER:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_TEST_CYLINDER;
      break;
   
   case e_SEND_MAINTAIN_TEMPERATURE: // SEND_MAINTAIN_TEMPERATURE
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_MAINTAIN_TEMPERATURE;
      break;

   case e_RECEIVE_MAINTAIN_TEMPERATURE: // RECEIVE_MAINTAIN_TEMPERATURE
      if (packet.data_size == 2)
      {
         maintain_temperature = packet.packet_data.idata[0];
         RESET_RX_MAINTAIN_TEMPERATURE;
      }
      break;

   case 51:
      if (packet.data_size == 1)
         if (packet.packet_data.data[0] == 1)
            RESET_TX_RESTORE_FIRMWARE;
      break;

   case 97:
      if (packet.data_size == VAL_TOKEN_LENGTH)
      {
         u32 mem_addr;
         u8 rpls_no;
         RESET_RX_TOKEN40;

         for (rpls_no = 0; rpls_no < VAL_TOKEN_LENGTH; rpls_no++)
            token40[rpls_no] = packet.packet_data.data[rpls_no];

         for (mem_addr = 0; mem_addr < VAL_TOKEN_LENGTH; mem_addr++)
         {
            if (token40[mem_addr] == 0)
               break;
         }

         if (mem_addr == VAL_TOKEN_LENGTH)
         {
            SET_SEND_TO_SERVER_POWER_ON;
            SET_TOKEN_VALID;
         }
      }
      break;

   case 98:
      if (packet.data_size == 1)
      {
         RESET_TX_TOKEN40;
      }
      break;

   default:
      break;
   }

   ptr_byte = &packet.marker; /* Установить указатель на позицию номера устройства в пакете */
   packet_count = 0;
   RESET_START_BYTE;
}

/*for commands 5,6,7 return 0*/

void ReplacePacket2(void)
{
   if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
   {
      rx2_timeout = 0;
   }

   packet2tx.command_num = packet2rx.command_num;

   switch (packet2tx.command_num)
   {
   case e_VERSION:
      if (packet2rx.data_size == 0)
      {
         SET_RX_VERSION;
         packet2tx.data_size = 16;
         packet2tx.packet_data.data[0] = 1;
         packet2tx.packet_data.data[1] = termopress_id;
         packet2tx.packet_data.data[2] = major_firmware;
         packet2tx.packet_data.data[3] = minor_firmware;
         packet2tx.packet_data.data[4] = version_day;
         packet2tx.packet_data.data[5] = version_month;
         packet2tx.packet_data.data[6] = version_year;
         packet2tx.packet_data.data[7] = LOW_L(serial_number);
         packet2tx.packet_data.data[8] = HIGH_L(serial_number);
         packet2tx.packet_data.data[9] = LOW_H(serial_number);
         packet2tx.packet_data.data[10] = HIGH_H(serial_number);
         packet2tx.packet_data.data[11] = VERS_PULT_H;
         packet2tx.packet_data.data[12] = VERS_PULT_L;
         packet2tx.packet_data.data[13] = VERS_PULT_DAY;
         packet2tx.packet_data.data[14] = VERS_PULT_MONTH;
         packet2tx.packet_data.data[15] = VERS_PULT_YEAR;
      }

      TransmitPacket2();
      break;

   case e_RESET:
      if (packet2rx.data_size == 0)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
         {
            SET_TX_RESET;
            // mode = 0;
            count_zone = 0;
            prm_num = 0;
            prg_num = 0;
            alertDelay = 30;
            alert_flag0 = 0;
         }
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_STATUS:
      if (packet2rx.data_size == 0)
      {
         SET_RX_STATUS;

         if (alertDelay != 0)
            alert_flag0 = 0;

         packet2tx.data_size = 10;
         packet2tx.packet_data.data[0] = LOW(mes_temperature);
         packet2tx.packet_data.data[1] = HIGH(mes_temperature);
         packet2tx.packet_data.data[2] = LOW(mes_pressure);
         packet2tx.packet_data.data[3] = HIGH(mes_pressure);
         packet2tx.packet_data.data[4] = termopress_mode;
         packet2tx.packet_data.data[5] = alert_flag0;
         packet2tx.packet_data.data[6] = LOW(real_room_temperature);
         packet2tx.packet_data.data[7] = HIGH(real_room_temperature);
         packet2tx.packet_data.data[8] = status_device;
         packet2tx.packet_data.data[9] = cntr_seconds;
      }
      TransmitPacket2();
      break;

   case e_VENTING:
      if (packet2rx.data_size == 0)
      {
         SET_TX_VENTING;
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 0;
      TransmitPacket2();

      break;

   case e_FLOW_STATUS:
      if (packet2rx.data_size == 0)
      {
         packet2tx.packet_data.data[0] = FlowFlags;
      }
      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_VALVE1:
      if (packet2rx.data_size == 0)
      {
         SET_STOCK_DN;
         SET_TX_VALVE1;
         packet2tx.packet_data.data[0] = 1;
      }

      packet2tx.data_size = 0;
      TransmitPacket2();
      break;

   case e_VALVE2:
      if (packet2rx.data_size == 0)
      {
         SET_STOCK_UP;
         SET_TX_VALVE2;
      }

      packet2tx.data_size = 0;
      TransmitPacket2();
      break;

   case e_GET_SERIAL_NUMBER:
      if (packet2rx.data_size == 4)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
         {
            LOW_L(serial_number) = packet2rx.packet_data.data[0];
            HIGH_L(serial_number) = packet2rx.packet_data.data[1];
            LOW_H(serial_number) = packet2rx.packet_data.data[2];
            HIGH_H(serial_number) = packet2rx.packet_data.data[3];
            SET_TX_SERIAL_NUMBER;
         }
         packet2tx.packet_data.data[0] = 1;
      }

      packet2tx.data_size = 0;
      TransmitPacket2();
      break;

   case e_PRESSURE_ADJUST:
      if (packet2rx.data_size == 0)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_TX_PRESSURE_ADJUST;
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_NEXT_ZONE:
      if (packet2rx.data_size == 0)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_TX_NEXT_ZONE;
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_TEMPERATURE_ADJUST:

      if (packet2rx.data_size == 0)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_TX_TEMPERATURE_ADJUST;
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_VENTING_OFF:
      if (packet2rx.data_size == 0)
      {
         // SET_TX_VENTING_OFF;
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            RESET_FLOW;
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_VENTING_ON:
      if (packet2rx.data_size == 0)
      {
         // SET_TX_VENTING_ON;
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_FLOW;
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_TSTART_TX:
      if (packet2rx.data_size == 2)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
         {
            SET_TX_TSTART;
            Tstart = packet2rx.packet_data.idata[0];
         }
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_TSTART_RX:
      //SET_RX_TSTART;
      packet2tx.data_size = 2;
      packet2tx.packet_data.idata[0] = Tstart;
      TransmitPacket2();
      break;

   case e_FACTORS_RX:
      SET_RX_FACTORS;
      packet2tx.data_size = 10;
      packet2tx.packet_data.data[0] = HIGH(tar_prm.cold_bias);
      packet2tx.packet_data.data[1] = LOW(tar_prm.cold_bias);
      packet2tx.packet_data.data[2] = HIGH(tar_prm.couple_bias);
      packet2tx.packet_data.data[3] = LOW(tar_prm.couple_bias);
      packet2tx.packet_data.data[4] = HIGH(tar_prm.couple_kfc);
      packet2tx.packet_data.data[5] = LOW(tar_prm.couple_kfc);
      packet2tx.packet_data.data[6] = HIGH(tar_prm.press_bias);
      packet2tx.packet_data.data[7] = LOW(tar_prm.press_bias);
      packet2tx.packet_data.data[8] = HIGH(tar_prm.press_kfc);
      packet2tx.packet_data.data[9] = LOW(tar_prm.press_kfc);
      TransmitPacket2();
      break;

   case e_FACTORS_TX:

      if (packet2rx.data_size == 10)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
         {
            SET_TX_FACTORS;
            HIGH(tar_prm.cold_bias) = packet2rx.packet_data.data[0];
            LOW(tar_prm.cold_bias) = packet2rx.packet_data.data[1];
            HIGH(tar_prm.couple_bias) = packet2rx.packet_data.data[2];
            LOW(tar_prm.couple_bias) = packet2rx.packet_data.data[3];
            HIGH(tar_prm.couple_kfc) = packet2rx.packet_data.data[4];
            LOW(tar_prm.couple_kfc) = packet2rx.packet_data.data[5];
            HIGH(tar_prm.press_bias) = packet2rx.packet_data.data[6];
            LOW(tar_prm.press_bias) = packet2rx.packet_data.data[7];
            HIGH(tar_prm.press_kfc) = packet2rx.packet_data.data[8];
            LOW(tar_prm.press_kfc) = packet2rx.packet_data.data[9];
         }
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_ADC_DATA:
      if (!packet2rx.data_size)
      {
         SET_RX_ADC_DATA;
         packet2tx.data_size = 6;
         packet2tx.packet_data.data[0] = HIGH(adc_data.adc_temperature);
         packet2tx.packet_data.data[1] = LOW(adc_data.adc_temperature);
         packet2tx.packet_data.data[2] = HIGH(adc_data.adc_couple);
         packet2tx.packet_data.data[3] = LOW(adc_data.adc_couple);
         packet2tx.packet_data.data[4] = HIGH(adc_data.adc_pressure);
         packet2tx.packet_data.data[5] = LOW(adc_data.adc_pressure);
      }

      TransmitPacket2();
      break;

   case e_BIAS_FACTOR_UP:
      if (packet2rx.data_size == 0)
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_TX_BIAS_FACTOR_UP;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_BIAS_FACTOR_DN:
      if (packet2rx.data_size == 0)
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_TX_BIAS_FACTOR_DN;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_KFC_FACTOR_UP:
      if (packet2rx.data_size == 0)
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_TX_KFC_FACTOR_UP;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_KFC_FACTOR_DN:
      if (packet2rx.data_size == 0)
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_TX_KFC_FACTOR_DN;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_STATISTICS:
      if (!packet2rx.data_size)
      {
         SET_RX_STATISTICS;
         packet2tx.data_size = VAL_STATISTICS;
         packet2tx.packet_data.data[0] = HIGH_H(statistics.w.cntr_heater);
         packet2tx.packet_data.data[1] = LOW_H(statistics.w.cntr_heater);
         packet2tx.packet_data.data[2] = HIGH_L(statistics.w.cntr_heater);
         packet2tx.packet_data.data[3] = LOW_L(statistics.w.cntr_heater);
         packet2tx.packet_data.data[4] = HIGH(statistics.w.cntr_termopresses);
         packet2tx.packet_data.data[5] = LOW(statistics.w.cntr_termopresses);
         packet2tx.packet_data.data[6] = HIGH(statistics.w.cntr_pnevmo);
         packet2tx.packet_data.data[7] = LOW(statistics.w.cntr_pnevmo);
         packet2tx.packet_data.data[8] = statistics.w.cntr_press_toggle_state;
      }

      TransmitPacket2();
      break;

   case e_RUN_PROG:
      if (packet2rx.data_size == 10)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
         {
            HIGH(prg_prm[WIFI_NUM_PROG].pressure) = packet2rx.packet_data.data[0];
            LOW(prg_prm[WIFI_NUM_PROG].pressure) = packet2rx.packet_data.data[1];
            HIGH(prg_prm[WIFI_NUM_PROG].temperature) = packet2rx.packet_data.data[2];
            LOW(prg_prm[WIFI_NUM_PROG].temperature) = packet2rx.packet_data.data[3];
            HIGH(prg_prm[WIFI_NUM_PROG].time_hold) = packet2rx.packet_data.data[4];
            LOW(prg_prm[WIFI_NUM_PROG].time_hold) = packet2rx.packet_data.data[5];
            HIGH(prg_prm[WIFI_NUM_PROG].time_press) = packet2rx.packet_data.data[6];
            LOW(prg_prm[WIFI_NUM_PROG].time_press) = packet2rx.packet_data.data[7];
            HIGH(prg_prm[WIFI_NUM_PROG].wflags) = packet2rx.packet_data.data[8];
            LOW(prg_prm[WIFI_NUM_PROG].wflags) = packet2rx.packet_data.data[9];

            if (prg_prm[WIFI_NUM_PROG].wflags & 0x04)
               prg_prm[WIFI_NUM_PROG].wflags &= 0xFB;
            else
               prg_prm[WIFI_NUM_PROG].wflags |= 0x04;

            if (!(prg_prm[WIFI_NUM_PROG].flags.press_after_heat))
            {
               prg_prm[WIFI_NUM_PROG].flags.auto_mode = 0;
               prm_num = PRM_PRESS;
            }

            prg_num = WIFI_NUM_PROG;
            SET_TX_RUN_PROG;
         }
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_PROG:
      if (packet2rx.data_size == 0)
      {
         SET_RX_PROG;
         packet2tx.packet_data.data[0] = LOW(time_zone);
         packet2tx.packet_data.data[1] = HIGH(time_zone);
         packet2tx.packet_data.data[2] = count_zone;
         packet2tx.packet_data.data[3] = LOW(duration);
         packet2tx.packet_data.data[4] = HIGH(duration);

         packet2tx.data_size = 5;
      }
      TransmitPacket2();
      break;

   case e_CONTINUE_PROG:
      if (packet2rx.data_size == 4)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
         {
            SET_TX_CONTINUE_PROG;
            prc_time = packet2tx.packet_data.idata[0];
            count_zone = packet2tx.packet_data.data[2];
         }

         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_CARTRIDGE_INSTALLED:
      if (packet2rx.data_size == 0)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_TX_CARTRIDGE_INSTALLED;

         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_SEND_MAINTAIN_TEMPERATURE:
      if (packet2rx.data_size == 2)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
         {
            maintain_temperature = packet2rx.packet_data.idata[0];
            SET_TX_MAINTAIN_TEMPERATURE;
         }

         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;
      
      packet2tx.data_size = 1;
      TransmitPacket2();
      break;
   
   case e_RECEIVE_MAINTAIN_TEMPERATURE:
      if (packet2rx.data_size == 0)
      {
         packet2tx.data_size = 2;
         packet2tx.packet_data.idata[0] = maintain_temperature;
         SET_RX_MAINTAIN_TEMPERATURE;
      }
      TransmitPacket2();
      break;

   case 47:
	   if (packet2rx.data_size == 0)
	   {
		   prg_prm[prg_num].flags.flow_tmp ^= 1;
	   }
	   packet2tx.packet_data.data[0] = prg_prm[prg_num].flags.flow_tmp;
	   packet2tx.data_size = 1;
	   TransmitPacket2();
	   break;

   case e_SYSTEM_RESET:
      if (packet2rx.data_size == 0)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_SYSTEM_RESET;
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_RESTORE_FIRMWARE:
      if (packet2rx.data_size == 0)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_TX_RESTORE_FIRMWARE;
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_TOKEN40_RX:
      if (packet2rx.data_size == 0)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            SET_TX_TOKEN40;
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_TOKEN40_TX:
      SET_RX_TOKEN40;
      packet2tx.data_size = VAL_TOKEN_LENGTH;

      for (uint8_t i = 0; i < VAL_TOKEN_LENGTH; i++)
         packet2tx.packet_data.data[i] = token40[i];

      TransmitPacket2();
      break;

   case 110:
      if (packet2rx.data_size == 0)
      {
         packet2tx.data_size = 10;
         packet2tx.packet_data.data[0] = LOW(prg_prm[prg_num].pressure);
         packet2tx.packet_data.data[1] = HIGH(prg_prm[prg_num].pressure);
         packet2tx.packet_data.data[2] = LOW(prg_prm[prg_num].temperature);
         packet2tx.packet_data.data[3] = HIGH(prg_prm[prg_num].temperature);
         packet2tx.packet_data.data[4] = LOW(prg_prm[prg_num].time_hold);
         packet2tx.packet_data.data[5] = HIGH(prg_prm[prg_num].time_hold);
         packet2tx.packet_data.data[6] = LOW(prg_prm[prg_num].time_press);
         packet2tx.packet_data.data[7] = HIGH(prg_prm[prg_num].time_press);

         u16 tmp = prg_prm[prg_num].wflags;

         // Reverse flow flag
         if (prg_prm[prg_num].flags.flow)
            tmp &= 0xFB;
         else
            tmp |= 0x04;

         packet2tx.packet_data.data[8] = LOW(tmp);
         packet2tx.packet_data.data[9] = HIGH(tmp);
      }
      else
      {
         packet2tx.data_size = 1;
         packet2tx.packet_data.data[0] = 3;
      }

      TransmitPacket2();
      break;

   case e_GET_ALL_PARAMS: // get all programs parameters
      if (packet2rx.data_size == 100)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
         {
            for (uint8_t i = 0; i < 10; i++)
            {
               prg_prm[i].pressure = packet2rx.packet_data.command_117.pressure[i];
               prg_prm[i].temperature = packet2rx.packet_data.command_117.temperature[i];
               prg_prm[i].time_hold = packet2rx.packet_data.command_117.timeHold[i];
               prg_prm[i].time_press = packet2rx.packet_data.command_117.timePress[i];
               prg_prm[i].wflags = packet2rx.packet_data.command_117.flags[i];

               if (prg_prm[i].wflags & 0x04)
                  prg_prm[i].wflags &= 0xFB;
               else
                  prg_prm[i].wflags |= 0x04;
            }

            SavePrg();
         }
         packet2tx.packet_data.data[0] = 1;
      }
      else
         packet2tx.packet_data.data[0] = 3;

      packet2tx.data_size = 1;
      TransmitPacket2();
      break;

   case e_TRANSMIT_ALL_PARAMS: // transmit all programs parameters
      if (!packet2rx.data_size)
      {
         for (uint8_t i = 0; i < 10; i++)
         {
            packet2tx.packet_data.command_117.pressure[i] = prg_prm[i].pressure;
            packet2tx.packet_data.command_117.temperature[i] = prg_prm[i].temperature;
            packet2tx.packet_data.command_117.timeHold[i] = prg_prm[i].time_hold;
            packet2tx.packet_data.command_117.timePress[i] = prg_prm[i].time_press;
            packet2tx.packet_data.command_117.flags[i] = prg_prm[i].wflags;

            if (packet2tx.packet_data.command_117.flags[i] & 0x04)
               packet2tx.packet_data.command_117.flags[i] &= 0xFB;
            else
               packet2tx.packet_data.command_117.flags[i] |= 0x04;
         }
         packet2tx.data_size = 100;
      }

      TransmitPacket2();
      break;

   case e_SEND_CURRENT_STA:
      if (!packet2rx.data_size)
      {
         u8 c = 0;

         packet2tx.data_size = 34;

         if (wifi_param.w.wifi_flags & 0x01)
         {
            char temp_char[11];
            u8 ssid[11] = "TRP#000000";

            for (u8 i = 0; i < DecToChar(&temp_char[0], serial_number); i++)
               ssid[i + 4] = temp_char[i];

            for (u8 i = 0; i < 34; i++)
            {
               if (c <= 9)
               {
                  packet2tx.packet_data.data[i] = ssid[c];
                  c++;
               }
               else
                  packet2tx.packet_data.data[i] = 0;
            }
         }
         else
         {
            for (u8 i = 0; i < 34; i++)
            {
               if (wifi_param.w.ssid[c] != 0)
               {
                  packet2tx.packet_data.data[i] = wifi_param.w.ssid[c];
                  c++;
               }
               else
                  packet2tx.packet_data.data[i] = 0;
            }
         }
      }
      else
         packet2tx.data_size = 0;

      TransmitPacket2();
      break;

   case e_SEND_AVAILABLE_STA:
      if (!packet2rx.data_size)
      {
         if (WiFiSTAs[0][0] == 0)
         {
            if (!STA_REQUEST && !STA_REQUEST_IN_PORGRESS)
               SET_STA_REQUEST;

            packet2tx.data_size = 1;
            packet2tx.packet_data.data[0] = 1;
         }
         else
         {
            if (STA_to_send != 0)
            {
               u8 c = 0;
               packet2tx.data_size = 34;
               for (u8 j = 0; j < 34; j++)
               {
                  if (WiFiSTAs[STA_to_send - 1][c] != 0)
                  {
                     packet2tx.packet_data.data[j] = WiFiSTAs[STA_to_send - 1][c];
                     c++;
                  }
                  else
                     packet2tx.packet_data.data[j] = 0;
               }
               STA_to_send--;

               while (WiFiSTAs[STA_to_send - 1][0] == 0 && STA_to_send > 0)
                  STA_to_send--;
            }
            else
            {
               RESET_STA_REQUEST;
               RESET_STA_REQUEST_IN_PORGRESS;
               WiFiSTAs[0][0] = 0;
               STA_to_send = current_name_sta_wifi;
               packet2tx.data_size = 0;
            }
         }
      }

      TransmitPacket2();
      break;

   case e_TABLET_CONNECTED: // подключение планшета
      if (packet2rx.data_size == 0)
      {
         SET_EXT_WIFI_CONNECTED;

         if (!MASTER_IPD) // so the last connected tablet is MASTER
         {
            SET_MASTER_IPD;
            masterIDofIPD = IDofIPD[cntr_RX_IPD];
         }

         packet2tx.data_size = 1;

         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
            packet2tx.packet_data.data[0] = 1;
         else
            packet2tx.packet_data.data[0] = 0;
      }

      TransmitPacket2();
      break;

   case e_TABLET_DISCONNECTED: // отключение планшета
      if (packet2rx.data_size == 0)
      {
         if (masterIDofIPD == IDofIPD[cntr_RX_IPD])
         {
            RESET_MASTER_IPD;
            RESET_EXT_WIFI_CONNECTED;
         }

         packet2tx.data_size = 1;
         packet2tx.packet_data.data[0] = 1;
      }

      TransmitPacket2();
      break;

   case e_SYS_RESET: // System reset
      RESET_EXT_WIFI_CONNECTED;

      if (!packet2rx.data_size)
      {
         btldr_state = 0xFFFD;
         SET_SAVE_BTLDR_STATE;
         SET_SAVE_PRMS;
         SET_SYSTEM_RESET;
         system_reset_mode = 0;
         packet_tx_flags2 = 0;
         packet_tx_flags &= 0xFFFE;
         packet_rx_flags = 0;
         mode = M_SYSTEM_RESET;
      }
      break;

   case e_SEND_DEVICE_PARAMS: // send device parameters
      if (!packet2rx.data_size)
      {
         RESET_EXT_WIFI_CONNECTED;
         packet2tx.data_size = 20;
         packet2tx.packet_data.command_124.ser_num = serial_number;
         packet2tx.packet_data.command_124.temperature = mes_temperature;
         packet2tx.packet_data.command_124.time_process = prc_time;
         packet2tx.packet_data.command_124.vacuum = mes_pressure / 10;
         packet2tx.packet_data.command_124.status_device = status_device;
         packet2tx.packet_data.command_124.dev_mode = termopress_mode;
         packet2tx.packet_data.command_124.dev_id = termopress_id;
      }

      TransmitPacket2();
      break;

   case e_GET_WIFI_PARAMS: // get wifi parameters
      if (packet2rx.data_size == 98)
      {
         u8 c;
         wifi_setup_zone = 99;
         wifi_param.w.wifi_flags = packet2rx.packet_data.command_125.mode;

         for (c = 0; c < 33; c++)
            wifi_param.w.ssid[c] = packet2rx.packet_data.command_125.ssid[c];

         for (c = 0; c < 64; c++)
            wifi_param.w.pswd[c] = packet2rx.packet_data.command_125.pswd[c];

         packet2tx.data_size = 1;
         packet2tx.packet_data.data[0] = 1;

         SET_SAVE_PRMS;
      }

      TransmitPacket2();
      break;

   default:
      if ((mode >= M_HEATING) && (mode <= M_COMPLETED))
         RESET_EXT_WIFI_CONNECTED;

      packet2tx.data_size = 1;
      packet2tx.packet_data.data[0] = 3;
      TransmitPacket2();
      break;
   }
}
