
#ifdef STM32L1XX_MD
#include "stm32l1xx.h"
#else
#include "stm32f10x.h"
#endif
#include "flash.h"
#include "glob.h"
#include "def.h"
#include "esp8266.h"
#include "programs.h"
#include "lcd.h"

union archive archiv;

vu32 firmware_length, firmware_blocks, firmware_crc32_source;
u32 cntr_firmware_length, cntr_firmware_write_to_flash;
u8 download_firmware_percent;
u8 cntr_firmware_line, read_firmware_line;
u8 firmware_line[100];
u8 firmware_cksum;
u8 write_firmware_pech_zone;
vu8 cntr_firmware_data;
u32 flash_page_size; // размер страницы памяти контроллера

void FLASHClearFlags(void)
{
#ifdef STM32L1XX_MD
   FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR | FLASH_FLAG_OPTVERRUSR);
#else
   FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
#endif
}
/*------------------------------------------
 пїЅпїЅпїЅпїЅпїЅпїЅ пїЅ EEPROM.
------------------------------------------*/
void SavePrg(void)
{
   u32 mem_addr, mem_offset;
   u32 *ptr;


   FLASH_Unlock();
   FLASHClearFlags();

   if (SAVE_BTLDR_STATE)
   {
      RESET_SAVE_BTLDR_STATE;
      mem_offset = 0;
#ifdef STM32L1XX_MD
      for (mem_addr = 0; mem_addr < 4; mem_addr++)
         while (FLASH_ErasePage(FLASH_PAGE_ADDRESS + (mem_addr * FLASH_PAGE_SIZE)) != FLASH_COMPLETE)
            ;

      while (FLASH_FastProgramWord(BOOTLDR_BASEADDRESS & 0xFFFFFFFC, (u32)btldr_state << 16) != FLASH_COMPLETE)
         ;
#else
      while (FLASH_ErasePage(FLASH_PAGE_ADDRESS) != FLASH_COMPLETE)
      {
      };

      while (FLASH_ProgramWord(BOOTLDR_BASEADDRESS & 0xFFFFFFFC, (u32)btldr_state << 16) != FLASH_COMPLETE)
               ;
#endif
   }
   else
   {
      for (mem_offset = 0; mem_offset < NUM_PARAMS_IN_FLASH; mem_offset++)
      {
         if ((*(u32 *)(FLASH_PAGE_ADDRESS + (mem_offset * FLASH_PARAMS_LENGTH))) == FLASH_RESET_STATE)
            break;
      }

      if (mem_offset == NUM_PARAMS_IN_FLASH) 
      {
         mem_offset = 0;
#ifdef STM32L1XX_MD
         for (mem_addr = 0; mem_addr < 4; mem_addr++)
            while (FLASH_ErasePage(FLASH_PAGE_ADDRESS + (mem_addr * FLASH_PAGE_SIZE)) != FLASH_COMPLETE)
               ;

         while (FLASH_FastProgramWord(BOOTLDR_BASEADDRESS & 0xFFFFFFFC, (u32)btldr_state << 16) != FLASH_COMPLETE)
            ;

#else
         while (FLASH_ErasePage(FLASH_PAGE_ADDRESS) != FLASH_COMPLETE)
            ;

         while (FLASH_ProgramWord(BOOTLDR_BASEADDRESS & 0xFFFFFFFC, (u32)btldr_state << 16) != FLASH_COMPLETE)
            ;

#endif
      }
   }
#ifdef STM32L1XX_MD
   while (FLASH_FastProgramWord(FLASH_MARKER_ADDRESS + (mem_offset * FLASH_PARAMS_LENGTH), 0xAA55AA55) != FLASH_COMPLETE)
      ;

   ptr = (u32 *)(&prg_prm[0]);

   for (mem_addr = 0; mem_addr < 100 / 4; mem_addr++)
      while (FLASH_FastProgramWord((FLASH_PROGRAM_ADDRESS) + (mem_offset * FLASH_PARAMS_LENGTH) + (4 * mem_addr), *ptr++) != FLASH_COMPLETE)
         ;

   for (mem_addr = 0; mem_addr < WIFI_PARAMS / 4; mem_addr++)
      while (FLASH_FastProgramWord((FLASH_WIFI_PARAMS) + (mem_offset * FLASH_PARAMS_LENGTH) + (4 * mem_addr), wifi_param.chdw[mem_addr]) != FLASH_COMPLETE)
         ;

   while (FLASH_FastProgramWord((FLASH_LANGUAGE) + (mem_offset * FLASH_PARAMS_LENGTH), lang_mode) != FLASH_COMPLETE)
      ;
#else
   while (FLASH_ProgramWord(FLASH_MARKER_ADDRESS + (mem_offset * FLASH_PARAMS_LENGTH), 0xAA55AA55) != FLASH_COMPLETE)
      ;
   ptr = (u32 *)(&prg_prm[0]);

   for (mem_addr = 0; mem_addr < 100 / 4; mem_addr++)
   {
      while (FLASH_ProgramWord((FLASH_PROGRAM_ADDRESS) + (mem_offset * FLASH_PARAMS_LENGTH) + (4 * mem_addr), *ptr++) != FLASH_COMPLETE)
         ;
   }

   for (mem_addr = 0; mem_addr < WIFI_PARAMS / 4; mem_addr++)
   {
      while (FLASH_ProgramWord((FLASH_WIFI_PARAMS) + (mem_offset * FLASH_PARAMS_LENGTH) + (4 * mem_addr), wifi_param.chdw[mem_addr]) != FLASH_COMPLETE)
         ;
   }

   while (FLASH_ProgramWord((FLASH_LANGUAGE) + (mem_offset * FLASH_PARAMS_LENGTH), lang_mode) != FLASH_COMPLETE)
      ;
#endif
   FLASH_Lock();
}

void RestorePrg(u8 prg)
{
   if ((prg_prm[prg].pressure > MAX_SET_PRESSURE) || (prg_prm[prg].pressure < MIN_SET_PRESSURE))
      prg_prm[prg].pressure = MIN_SET_PRESSURE;

   if ((prg_prm[prg].temperature > MAX_SET_TEMPERATURE) || (prg_prm[prg].temperature < MIN_SET_TEMPERATURE))
      prg_prm[prg].temperature = MIN_SET_TEMPERATURE;

   if ((prg_prm[prg].time_hold > MAX_SET_TIME_HOLD) || (prg_prm[prg].time_hold < MIN_SET_TIME_HOLD))
      prg_prm[prg].time_hold = MIN_SET_TIME_HOLD;

   if ((prg_prm[prg].time_press > MAX_SET_TIME_PRESS) || (prg_prm[prg].time_press < MIN_SET_TIME_PRESS))
      prg_prm[prg].time_press = MIN_SET_TIME_PRESS;
   ;
}

const u16 eeprom_prm[] = { // пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ
    170,                   // 1.Perflex Acry Free
    260,
    1200,
    1200,
    0xFFFF,

    150, // 2.Evolon Flexy N512T
    250,
    660,
    900,
    0xFFFF,

    150, // 3.Perflex T-Crystal
    260,
    660,
    900,
    0xFFFF,

    150, // 4.Vertex ThermoSens
    270,
    960,
    900,
    0xFFEF, // 0xFFFF,

    150, // 5.пїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅ
    250,
    660,
    900,
    0xFFFF,

    170, // 6.Yamahachi Basis PA
    280,
    1500,
    1200,
    0xFFFF,

    150, // 7.Deflex
    275,
    900,
    900,
    0xFFFF,

    150, // 8.Yamahachi Basis POLYCA
    260,
    1800,
    1500,
    0xFFFF,

    150, // 9.Evolon Evoplast
    260,
    1200,
    1200,
    0xFFFF,

    600, // 10.techno
    300,
    1800,
    1800,
    0xFFFF};

void RestoreAll(void)
{
   u32 mem_addr, mem_offset;
   u32 *ptr;

   u8 cntr;

   for (mem_offset = NUM_PARAMS_IN_FLASH; mem_offset; mem_offset--)
   {
#ifdef STM32L1XX_MD
      if ((*(u32 *)(FLASH_PAGE_ADDRESS + ((mem_offset - 1) * FLASH_PARAMS_LENGTH))) != FLASH_RESET_STATE)
         break;
#else
      if ((*(u32 *)(FLASH_PAGE_ADDRESS + ((mem_offset - 1) * FLASH_PARAMS_LENGTH))) != FLASH_RESET_STATE)
         break;
#endif
   }

   if (mem_offset == 0)
      mem_offset = 1;
#ifdef STM32L1XX_MD
   ptr = (u32 *)(&prg_prm[0]);

   for (mem_addr = 0; mem_addr < 100 / 4; mem_addr++) // пїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ
   {
      *ptr++ = (*(u32 *)(FLASH_PROGRAM_ADDRESS + ((mem_offset - 1) * FLASH_PARAMS_LENGTH) + 4 * mem_addr));
   }

   for (mem_addr = 0; mem_addr < WIFI_PARAMS / 4; mem_addr++)
      wifi_param.chdw[mem_addr] = (*(u32 *)(FLASH_WIFI_PARAMS + ((mem_offset - 1) * FLASH_PARAMS_LENGTH) + 4 * mem_addr));

   if (!(wifi_param.w.wifi_flags & 0x80))
      wifi_param.w.wifi_flags = 0xFF;

   lang_mode = (*(u32 *)(FLASH_LANGUAGE + ((mem_offset - 1) * FLASH_PARAMS_LENGTH)));
#else
   ptr = (u32 *)(&prg_prm[0]);

   for (mem_addr = 0; mem_addr < 100 / 4; mem_addr++) // пїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ
   {
      *ptr++ = (*(u32 *)(FLASH_PROGRAM_ADDRESS + ((mem_offset - 1) * FLASH_PARAMS_LENGTH) + 4 * mem_addr));
   }

   for (mem_addr = 0; mem_addr < WIFI_PARAMS / 4; mem_addr++)
      wifi_param.chdw[mem_addr] = (*(u32*)(FLASH_WIFI_PARAMS + ((mem_offset - 1) * FLASH_PARAMS_LENGTH) + 4 * mem_addr));

   lang_mode = (*(u32 *)(FLASH_LANGUAGE + ((mem_offset - 1) * FLASH_PARAMS_LENGTH)));
#endif
   if (lang_mode > 1)
      lang_mode = 0;

   if ((prg_prm[0].temperature > MAX_SET_TEMPERATURE) || (prg_prm[0].temperature < MIN_SET_TEMPERATURE))
   {
      for (cntr = 0; cntr < 10; cntr++)
      {
         prg_prm[cntr].pressure = eeprom_prm[cntr * 5 + 0];
         prg_prm[cntr].temperature = eeprom_prm[cntr * 5 + 1];
         prg_prm[cntr].time_hold = eeprom_prm[cntr * 5 + 2];
         prg_prm[cntr].time_press = eeprom_prm[cntr * 5 + 3];
         prg_prm[cntr].wflags = eeprom_prm[cntr * 5 + 4];
      }
   }

   RestorePrg(9);
}

void RestoreArchive(void)
{
   u16 cntr;

   for (cur_ptr_archive_in_flash = NUM_ARCHIVES_IN_FLASH; cur_ptr_archive_in_flash; cur_ptr_archive_in_flash--)
   {
      if ((*(u32 *)(FLASH_PAGE_ARCHIVE + (FLASH_ARCHIVE_LENGHT * (cur_ptr_archive_in_flash - 1)))) != FLASH_RESET_STATE)
         break;
   }

   if (cur_ptr_archive_in_flash == 0)
      cur_ptr_archive_in_flash = 1;

   for (cntr = 0; cntr < ARCHIVE_LENGTH / 4; cntr++)
      archiv.chdw[cntr] = (*(u32 *)(4 * cntr + FLASH_PAGE_ARCHIVE + (FLASH_ARCHIVE_LENGHT * (cur_ptr_archive_in_flash - 1))));

   if (archiv.w.mode_arch == M_HEATING)
   {
      count_zone = 0;
      {
         if (archiv.w.term_arch <= (mes_temperature + 50))
         {
            mode = archiv.w.mode_arch;
            prg_num = archiv.w.prg_num_arch;

            if (prg_num > MAX_PRG_NUM)
            	prg_num = 0;

            set_temperature = prg_prm[prg_num].temperature;
            SET_PRE_CONTINUE_PROG;
            if (!prg_prm[prg_num].flags.poluavtomat)
            {
               SET_HEATING;
               {
                  stage = S_PREHEATING;
                  SET_PREHEATING_TSTART;
               }
            }
         }
      }
   }
   else if (archiv.w.mode_arch == M_HOLDING)
   {
      if (archiv.w.term_arch <= (mes_temperature + 50))
      {
         count_zone = 1;
         mode = M_HOLDING;
         prg_num = prg_num;

         if (prg_num > MAX_PRG_NUM)
         	prg_num = 0;

         prc_time = archiv.w.time_arch;
         set_temperature = prg_prm[prg_num].temperature;
         SET_HEATING;
         SET_PRE_CONTINUE_PROG;
      }
   }
   else if (archiv.w.mode_arch == M_PRESSING)
   {
      count_zone = 3;
      prg_num = archiv.w.prg_num_arch;

      if (prg_num > MAX_PRG_NUM)
      	prg_num = 0;

      StartPressing();
      mode = archiv.w.mode_arch;
      prc_time = archiv.w.time_arch;
      SET_PRE_CONTINUE_PROG;
   }
}

void SaveArchive(void)
{
   u32 cntr;
   cntr_save_archive = 0;
   FLASH_Unlock();
   FLASHClearFlags();
   archiv.w.mode_arch = mode;
   archiv.w.term_arch = mes_temperature;
   archiv.w.time_arch = time_zone;
   archiv.w.prg_num_arch = prg_num;
   if (++cur_ptr_archive_in_flash >= NUM_ARCHIVES_IN_FLASH)
   {
      cur_ptr_archive_in_flash = 0;
#ifdef STM32L1XX_MD
      while (FLASH_ErasePage(FLASH_PAGE_ARCHIVE) != FLASH_COMPLETE)
         ;
#else
      while (FLASH_ErasePage(FLASH_PAGE_ARCHIVE) != FLASH_COMPLETE)
         ;
#endif
   }
#ifdef STM32L1XX_MD
   for (cntr = 0; cntr < ARCHIVE_LENGTH / 4; cntr++)
   {
      while (FLASH_FastProgramWord(4 * cntr + FLASH_PAGE_ARCHIVE + (FLASH_ARCHIVE_LENGHT * (cur_ptr_archive_in_flash - 0)), archiv.chdw[cntr]) != FLASH_COMPLETE)
         ;
   }
#else
   for (cntr = 0; cntr < ARCHIVE_LENGTH / 4; cntr++)
   {
      while (FLASH_ProgramWord(4 * cntr + FLASH_PAGE_ARCHIVE + (FLASH_ARCHIVE_LENGHT * (cur_ptr_archive_in_flash - 0)), archiv.chdw[cntr]) != FLASH_COMPLETE)
         ;
   }
#endif
   FLASH_Lock();
}

/*--------- For writing downloaded from server firmware to flash ---------*/

u8 gx(void)
{
   u8 tmp, res;

   do
   {
      tmp = firmware_line[read_firmware_line++];
   } while (tmp < '0');

   if ((tmp >= 'a') && (tmp <= 'f'))
      res = ((tmp - (u8)'a') + 10);
   else if ((tmp >= 'A') && (tmp <= 'F'))
      res = ((tmp - (u8)'A') + 10);
   else
      res = (tmp - (u8)'0');

   return res;
}

u8 g2x(void)
{
   u8 tmp;

   tmp = ((gx() << 4) + gx());
   firmware_cksum += tmp;

   return tmp;
}

void checksum(void)
{
   g2x();

   if (firmware_cksum) // if checksum does not add to zero, bad check, reset
   {
      cntr_firmware_block = 0;
      RESET_WRITE_DOWNLOADED_FIRMWARE_TO_FLASH;
      RESET_FIRMWARE_NEED_TO_ADD_LINE;
   }
}

void ClearBeforeWrite(void)
{
   if (CLEAR_BEFORE_WRITE_FIRMWARE)
   {
      RESET_CLEAR_BEFORE_WRITE_FIRMWARE;
      FLASH_Unlock();
      FLASHClearFlags();

      for (u32 erase = START_MIRROR_FIRMWARE; erase < END_MIRROR_FIRMWARE; erase += flash_page_size)
         while (FLASH_ErasePage(erase) != FLASH_COMPLETE)
            ;

      FLASH_Lock();
      cntr_firmware_length = 0;
      cntr_firmware_write_to_flash = 0;
   }
}

void WriteFirmwareToCB(void) // CB - Controll Board
{
   u8 c;
   FLASH_Unlock();
   FLASHClearFlags();
#ifdef STM32L1XX_MD
   for (c = 0; c < FIRMWARE_DATA_BLOCK_LENGTH / 4; c++)
   {
      cntr_firmware_length += 4;

      while (FLASH_FastProgramWord(START_MIRROR_FIRMWARE + cntr_firmware_write_to_flash, firmware_data.chdw[c]) != FLASH_COMPLETE)
         ;

      cntr_firmware_write_to_flash += 4;
   }
#else
   for (c = 0; c < FIRMWARE_DATA_BLOCK_LENGTH / 4; c++)
   {
      cntr_firmware_length += 4;

      while (FLASH_ProgramWord(START_MIRROR_FIRMWARE + cntr_firmware_write_to_flash, firmware_data.chdw[c]) != FLASH_COMPLETE)
         ;

      cntr_firmware_write_to_flash += 4;
   }
#endif
   FLASH_Lock();

   RESET_WRITE_DOWNLOADED_FIRMWARE_TO_FLASH;
   if (cntr_firmware_length >= firmware_length)
   {
      SET_PECH_FIRMWARE_DOWNLOADED;
      RESET_DOWNLOAD_FIRMWARE_IN_PROGRESS;
   }
   else
      SET_REQUEST_FIRMWARE_DATA;

   download_firmware_percent = 100UL * cntr_firmware_block / firmware_blocks;
}

void WriteFirmwareToIB() // IB - Idication Board
{
	u8 rectype, bcount, err = 0, buff[4];
   static u32 erase;
	u32 addr;

   if (FIRMWARE_NEED_TO_ADD_LINE)
   {
      RESET_FIRMWARE_NEED_TO_ADD_LINE;
      cntr_firmware_length = 0;

      while ((firmware_data.ch[cntr_firmware_data] == ':') || (firmware_data.ch[cntr_firmware_data] == 10))
         cntr_firmware_data++;

      while ((firmware_data.ch[cntr_firmware_data] != 13) && (cntr_firmware_data < FIRMWARE_DATA_BLOCK_LENGTH))
         firmware_line[cntr_firmware_line++] = firmware_data.ch[cntr_firmware_data++];

      if (cntr_firmware_data >= FIRMWARE_DATA_BLOCK_LENGTH)
         err = 1;
   }
   else
   {
      cntr_firmware_line = 0;

      while ((firmware_data.ch[cntr_firmware_data] != ':') && (cntr_firmware_data < FIRMWARE_DATA_BLOCK_LENGTH))
         cntr_firmware_data++;

      cntr_firmware_data++;

      if (cntr_firmware_data < FIRMWARE_DATA_BLOCK_LENGTH)
      {
         while ((firmware_data.ch[cntr_firmware_data] != 13) && (cntr_firmware_data < FIRMWARE_DATA_BLOCK_LENGTH))
            firmware_line[cntr_firmware_line++] = firmware_data.ch[cntr_firmware_data++];
         if (cntr_firmware_data >= FIRMWARE_DATA_BLOCK_LENGTH)
            err = 1;
      }
      else
         err = 1;
   }

   if (err)
   {
      SET_REQUEST_FIRMWARE_DATA;
      SET_FIRMWARE_NEED_TO_ADD_LINE;
      RESET_WRITE_DOWNLOADED_FIRMWARE_TO_FLASH;
      download_firmware_percent = 100UL * cntr_firmware_block / firmware_blocks;
   }
   else
   {
      read_firmware_line = 0;
      firmware_cksum = g2x();
      bcount = firmware_cksum;
      addr = (g2x() << 8) | g2x();
      erase &= 0xFFFF0000;
      erase |= (addr & 0x0000FFFF);
      rectype = g2x();

      switch (rectype)
      {
      case 0x00:
         FLASH_Unlock();
         FLASHClearFlags();

         while (bcount--)
         {
#ifdef STM32L1XX_MD
            buff[addr & 0x03] = g2x();

            if ((addr & 0x03) == 3)
            {
               u32 data_to_flash;
               LOW_L(data_to_flash) = buff[0];
               HIGH_L(data_to_flash) = buff[1];
               LOW_H(data_to_flash) = buff[2];
               HIGH_H(data_to_flash) = buff[3];

               while (FLASH_FastProgramWord(erase + (START_MIRROR_FIRMWARE - PROG_START), data_to_flash) != FLASH_COMPLETE)
                  ;

               erase += 4;
            }
#else
            buff[addr & 0x03] = g2x();

            if ((addr & 0x03) == 3)
            {
               u32 data_to_flash;
               LOW_L(data_to_flash) = buff[0];
               HIGH_L(data_to_flash) = buff[1];
               LOW_H(data_to_flash) = buff[2];
               HIGH_H(data_to_flash) = buff[3];

               while (FLASH_ProgramWord(erase + (START_MIRROR_FIRMWARE - PROG_START), data_to_flash) != FLASH_COMPLETE)
                  ;

               erase += 4;
            }
#endif
            addr++;
            cntr_firmware_length++;
         }

         FLASH_Lock();
         checksum();
         break;

      case 0x01: // end of hex file
         checksum();
         RESET_WRITE_DOWNLOADED_FIRMWARE_TO_FLASH;
         ClearDisplay();
         SET_PULT_FIRMWARE_DOWNLOADED;
         RESET_DOWNLOAD_FIRMWARE_IN_PROGRESS;
         break;

      case 0x04: // extended address record
         addr = (g2x() << 8) | g2x();
         erase &= 0x0000FFFF;
         erase |= ((addr << 16) & 0xFFFF0000);
         checksum();
         break;

      default:
         while (bcount--)
            g2x();

         checksum();
         break;
      }
   }
}

void WriteFirmwareToFlash(void)
{
   if (WRITE_DOWNLOADED_FIRMWARE_TO_FLASH)
   {
      SET_DOWNLOAD_FIRMWARE_IN_PROGRESS;

      ClearBeforeWrite();

      if (firmware_device_type) // pech
         WriteFirmwareToCB();
      else // pult
         WriteFirmwareToIB();
   }
}
