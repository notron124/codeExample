#ifndef FLASH_H_
#define FLASH_H_

#ifdef STM32L1XX_MD
#define FLASH_RESET_STATE 0UL
#define FLASH_PAGE_SIZE 0x100
#else
#define FLASH_RESET_STATE 0xFFFFFFFFUL
#endif

#define ARCHIVE_LENGTH 8
union archive
{
   struct
   {
      u16 mode_arch;
      u16 term_arch;
      u16 time_arch;
      u16 prg_num_arch;
   } w;
   u8 ch[ARCHIVE_LENGTH];
   u16 chw[ARCHIVE_LENGTH / 2];
   u32 chdw[ARCHIVE_LENGTH / 4];
};

#define FIRMWARE_DATA_BLOCK_LENGTH 200

union _firmware_data
{
	vu8 ch[FIRMWARE_DATA_BLOCK_LENGTH + 4];
	vu16 chw[(FIRMWARE_DATA_BLOCK_LENGTH + 4) / 2];
	vu32 chdw[(FIRMWARE_DATA_BLOCK_LENGTH + 4) / 4];
};

#ifdef STM32L1XX_MD
#define FLASH_PAGE_ARCHIVE 0x8002F00UL // 0x8002F00UL//
#define NUM_ARCHIVES_IN_FLASH 32
#else
#define FLASH_PAGE_ARCHIVE		0x8002C00UL
#define NUM_ARCHIVES_IN_FLASH	128
#endif

#define FLASH_ARCHIVE_LENGHT	8

#define FLASH_PAGE_ADDRESS		0x8003000UL //(0x8003000UL)//
#define FLASH_MARKER_ADDRESS	FLASH_PAGE_ADDRESS

#ifdef STM32L1XX_MD
#define FLASH_PROGRAM_ADDRESS (FLASH_PAGE_ADDRESS + 4)
#define FLASH_WIFI_PARAMS (FLASH_PROGRAM_ADDRESS + 100)
#define FLASH_LANGUAGE (FLASH_WIFI_PARAMS + WIFI_PARAMS)

#define FLASH_PARAMS_LENGTH (4 + (100) + WIFI_PARAMS + 4 + 44)
#else
#define FLASH_PROGRAM_ADDRESS (FLASH_PAGE_ADDRESS + 4)
#define FLASH_WIFI_PARAMS (FLASH_PROGRAM_ADDRESS + 100)
#define FLASH_LANGUAGE (FLASH_WIFI_PARAMS + WIFI_PARAMS)

#define FLASH_PARAMS_LENGTH (4 + (100) + WIFI_PARAMS + 4 + 44)
#endif
#define NUM_PARAMS_IN_FLASH (1024 / FLASH_PARAMS_LENGTH) // 4

#define PROG_START 			0x8003400UL

#define BOOTLDR_BASEADDRESS 	(PROG_START - 2) //(MEM_TOP-0x400) //0x8077FFEUL

#define FLASH_SIZE_REGISTER 	0x1FFFF7E0UL

#define START_MIRROR_FIRMWARE	0x8011800UL
#define END_MIRROR_FIRMWARE 	0x8020000UL // constant do not touch (it is the end of flash)

vu32 flash_flags;
#define F_CLEAR_BEFORE_WRITE_FIRMWARE 			0x00000001
#define F_WRITE_DOWNLOADED_FIRMWARE_TO_FLASH	0x00000002
#define F_FIRMWARE_NEED_TO_ADD_LINE 			0x00000004
#define F_FIRMWARE_DATA_PACKET_ADD 				0x00000008

#define F_PULT_FIRMWARE_DOWNLOADED 				0x00000010
#define F_PECH_FIRMWARE_DOWNLOADED 				0x00000020
#define F_DOWNLOAD_FIRMWARE_IN_PROGRESS 		0x00000040
#define F_FIRMWARE_CORRECTION 					0x00000080

#define F_SAVE_PRMS 							      0x00000100
#define F_SAVE_BTLDR_STATE 						0x00000200

#define SET_CLEAR_BEFORE_WRITE_FIRMWARE 				flash_flags |= F_CLEAR_BEFORE_WRITE_FIRMWARE
#define RESET_CLEAR_BEFORE_WRITE_FIRMWARE 			flash_flags &= ~F_CLEAR_BEFORE_WRITE_FIRMWARE
#define CLEAR_BEFORE_WRITE_FIRMWARE 					(flash_flags & F_CLEAR_BEFORE_WRITE_FIRMWARE)

#define SET_WRITE_DOWNLOADED_FIRMWARE_TO_FLASH 		flash_flags |= F_WRITE_DOWNLOADED_FIRMWARE_TO_FLASH
#define RESET_WRITE_DOWNLOADED_FIRMWARE_TO_FLASH 	flash_flags &= ~F_WRITE_DOWNLOADED_FIRMWARE_TO_FLASH
#define WRITE_DOWNLOADED_FIRMWARE_TO_FLASH 			(flash_flags & F_WRITE_DOWNLOADED_FIRMWARE_TO_FLASH)

#define SET_FIRMWARE_NEED_TO_ADD_LINE 					flash_flags |= F_FIRMWARE_NEED_TO_ADD_LINE
#define RESET_FIRMWARE_NEED_TO_ADD_LINE 				flash_flags &= ~F_FIRMWARE_NEED_TO_ADD_LINE
#define FIRMWARE_NEED_TO_ADD_LINE 						(flash_flags & F_FIRMWARE_NEED_TO_ADD_LINE)

#define SET_FIRMWARE_DATA_PACKET_ADD 					flash_flags |= F_FIRMWARE_DATA_PACKET_ADD
#define RESET_FIRMWARE_DATA_PACKET_ADD 				flash_flags &= ~F_FIRMWARE_DATA_PACKET_ADD
#define FIRMWARE_DATA_PACKET_ADD 						(flash_flags & F_FIRMWARE_DATA_PACKET_ADD)

#define SET_PULT_FIRMWARE_DOWNLOADED 					flash_flags |= F_PULT_FIRMWARE_DOWNLOADED
#define RESET_PULT_FIRMWARE_DOWNLOADED 				flash_flags &= ~F_PULT_FIRMWARE_DOWNLOADED
#define PULT_FIRMWARE_DOWNLOADED 						(flash_flags & F_PULT_FIRMWARE_DOWNLOADED)

#define SET_PECH_FIRMWARE_DOWNLOADED 					flash_flags |= F_PECH_FIRMWARE_DOWNLOADED
#define RESET_PECH_FIRMWARE_DOWNLOADED 				flash_flags &= ~F_PECH_FIRMWARE_DOWNLOADED
#define PECH_FIRMWARE_DOWNLOADED 						(flash_flags & F_PECH_FIRMWARE_DOWNLOADED)

#define SET_DOWNLOAD_FIRMWARE_IN_PROGRESS 			flash_flags |= F_DOWNLOAD_FIRMWARE_IN_PROGRESS
#define RESET_DOWNLOAD_FIRMWARE_IN_PROGRESS 			flash_flags &= ~F_DOWNLOAD_FIRMWARE_IN_PROGRESS
#define DOWNLOAD_FIRMWARE_IN_PROGRESS 					(flash_flags & F_DOWNLOAD_FIRMWARE_IN_PROGRESS)

#define SET_FIRMWARE_CORRECTION 		flash_flags |= F_FIRMWARE_CORRECTION
#define RESET_FIRMWARE_CORRECTION 	flash_flags &= ~F_FIRMWARE_CORRECTION
#define TGL_FIRMWARE_CORRECTION 		flash_flags ^= F_FIRMWARE_CORRECTION
#define FIRMWARE_CORRECTION 			(flash_flags & F_FIRMWARE_CORRECTION)

#define SET_SAVE_PRMS 					flash_flags |= F_SAVE_PRMS
#define RESET_SAVE_PRMS 				flash_flags &= ~F_SAVE_PRMS
#define SAVE_PRMS 						(flash_flags & F_SAVE_PRMS)

#define SET_SAVE_BTLDR_STATE 			flash_flags |= F_SAVE_BTLDR_STATE
#define RESET_SAVE_BTLDR_STATE 		flash_flags &= ~F_SAVE_BTLDR_STATE
#define SAVE_BTLDR_STATE 				(flash_flags & F_SAVE_BTLDR_STATE)

void FLASHClearFlags(void);
void SavePrg(void);
void RestorePrg(u8 prg);
void RestoreAll(void);
void RestoreArchive(void);
void SaveArchive(void);
void WriteFirmwareToFlash(void);

extern union archive archiv;
extern vu32 firmware_length, firmware_blocks, firmware_crc32_source;
extern u32 cntr_firmware_length, cntr_firmware_write_to_flash;
extern u8 download_firmware_percent;
// extern u8 cntr_firmware_line, read_firmware_line;
// extern u8 firmware_line[];
extern u8 firmware_cksum;
extern u8 write_firmware_pech_zone;
extern vu8 cntr_firmware_data;
extern u32 flash_page_size; // размер страницы памяти контроллера

#endif
