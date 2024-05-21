#ifdef STM32L1XX_MD
#include "stm32l1xx.h"
#else
#include "stm32f10x.h"
#endif
#include "char_table.h"
#include "port.h"
#include "lcd1602.h"

const u16 TableDelay[] = {800, 200, 100, 1200};
const u8 TableInitLCD[] = {0x28, 0x0C, 0x01, 0x06};

extern vu8 lcd_flags;

u8 const alt_sym[] = {
    //
    0b00000000, //  0 -
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,

    0x00, // 0b00000000,
    0x1F, // 0b00011111, // 1 Wi-Fi
    0x00, // 0b00000000,
    0x0E, // 0b00001110,
    0x00, // 0b00000000,
    0x04, // 0b00000100,
    0x00, // 0b00000000,
    0x00, // 0b00000000,

    0b00000110, //  2 -
    0b00001001,
    0b00001001,
    0b00000110,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,

    0b00001000, //  3 -   :)
    0b00001100,
    0b00001110,
    0b00001111,
    0b00001110,
    0b00001100,
    0b00001000,
    0b00000000,

    0b00000000, //  4 -
    0b00000000,
    0b00000000,
    0b00011111,
    0b00001110,
    0b00000100,
    0b00000000,
    0b00000000,

    0b00000000, //  5 -
    0b00000000,
    0b00000000,
    0b00000100,
    0b00001110,
    0b00011111,
    0b00000000,
    0b00000000,

    0b00000000, //  6 - V
    0b00000000,
    0b00000000,
    0b00000001,
    0b00000010,
    0b00010100,
    0b00001000,
    0b00000000,
};

void delay_LCD(u32 time)
{
   while (time--)
   {
      __NOP();
   }
}

void PutChr_LCD(u8 status, u16 data)
{
   if (status)
      LCD_COMMAND;
   else
      LCD_DATA;
   LCD_GPIO_ODR &= 0x0FFF; // #v8
   LCD_ENABLE;
   LCD_GPIO_ODR |= (data & 0x00F0) << 8; // #v8
   __NOP();
   __NOP();
   LCD_DISABLE;
   LCD_GPIO_ODR &= 0x0FFF; // #v8
   LCD_ENABLE;
   LCD_GPIO_ODR |= ((data << 4) & 0x00F0) << 8;
   __NOP();
   __NOP();
   LCD_DISABLE;
   delay_LCD(200 * 16);
}

void PutClear_LCD(void)
{
   PutChr_LCD(_comm, 0x01);
   delay_LCD(2000 * 10);
}

void InitLCD(void)
{
   u8 cntr;
   delay_LCD(9000 * 16);
   
   for (cntr = 0; cntr < sizeof(TableInitLCD); cntr++)
   {
      delay_LCD(TableDelay[cntr] * 16);
      PutChr_LCD(_comm, TableInitLCD[cntr]);
   }

   delay_LCD(1000 * 16);

   PutChr_LCD(_comm, 0x40);

   for (cntr = 0; cntr < sizeof(alt_sym); cntr++)
      PutChr_LCD(_data, alt_sym[cntr]);

   PutChr_LCD(_comm, 0x80);
}

void ReInitLCD(void)
{
   PutChr_LCD(_comm, 0x28);
   delay_LCD(TableDelay[2]);
   PutChr_LCD(_comm, 0x2C);
   delay_LCD(TableDelay[2]);
   PutChr_LCD(_comm, 0x0C);
   delay_LCD(TableDelay[2]);
   PutChr_LCD(_comm, 0x6);
   delay_LCD(TableDelay[2]);
}

/* Prints string on lcd.
   x - horizontal cooridnate. 0-15. If x = 255 will print after last character.
   y - vertical cooridnate. 0 or 1.
   *data - pointer to array, that will be printed.
   attrib - 0 or BLINK_FLAG.
*/
void PutStr_LCD(u8 x, u8 y, const u8 *data, u8 attrib)
{
   u8 k;

   k = 0x80 | (y << 6);

   if (x < 16)
      PutChr_LCD(_comm, k + x);

   while (*data)
   {
      if ((attrib & 0x80) && (BLINK_PARAM))
         PutChr_LCD(_data, 32);
      else
         PutChr_LCD(_data, SymbolToLCD(*data));
      data++;
   }
}

/* Prints digit on lcd.
   x - horizontal cooridnate. 0-15. If x = 255 will print after last character.
   y - vertical cooridnate. 0 or 1.
   numdigits - number of digits.
   digit - variable/number to print.
   attrib - 0 or BLINK_FLAG.
   lead - character that will be printed instead of blank spaces.
*/
void PutDgt_LCD(u8 x, u8 y, u8 numdigits, u16 digit, u8 attrib, u8 lead)
{
   u8 k, n, no_lead, lcd;
   u16 dgt, mul;

   dgt = digit;
   no_lead = 0;

   k = 0x80 | (y << 6);

   if (x < 16)
      PutChr_LCD(_comm, k + x);
      
   for (k = numdigits; k; k--)
   {
      lcd = 0;
      mul = 1;
      for (n = 0; n < (k - 1); n++)
         mul *= 10;
      while (dgt >= mul)
      {
         dgt -= mul;
         lcd++;
      }

      if (lcd)
      {
         no_lead = 1;
         lcd += 48;
      }
      else
      {
         if (no_lead)
            lcd = 48;
         else
         {
            if (lead == '0')
               lcd = 48;
            else
            {
               if (k == 1)
                  lcd = 48;
               else
                  lcd = 32;
            }
         }
      }
      if ((attrib & 0x80) && (BLINK_PARAM))
         PutChr_LCD(_data, 32);
      else
         PutChr_LCD(_data, lcd);
   }
}
