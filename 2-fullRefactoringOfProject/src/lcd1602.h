#ifndef LCD1602_H_
#define LCD1602_H_

#define LCD_DELAY_MULTIPLYER  16

#define _comm 1
#define _data 0

#define P_LCD_EN     GPIO_ODR_ODR0
#define P_LCD_RS     GPIO_ODR_ODR1

#define LCD_ENABLE	GPIOB->ODR |= P_LCD_EN //#v8
#define LCD_DISABLE	GPIOB->ODR &= ~P_LCD_EN //#v8
#define LCD_DATA	   GPIOB->ODR |= P_LCD_RS //#v8
#define LCD_COMMAND	GPIOB->ODR &= ~P_LCD_RS //#v8
#define LCD_GPIO_ODR GPIOB->ODR

vu8 lcd_flags;
#define F_REFRESH_LCD		0x01
#define F_CLEAR_LCD			0x02
#define F_BLINK_PARAM		0x04
#define F_REINIT_LCD		   0x08 //#reinit_lcd
#define F_IGNORE_OVERHEAT	0x10 //#ignore_overheat

#define SET_REFRESH_LCD		lcd_flags |= F_REFRESH_LCD
#define RESET_REFRESH_LCD	lcd_flags &= ~F_REFRESH_LCD
#define REFRESH_LCD			(lcd_flags & F_REFRESH_LCD)

#define SET_CLEAR_LCD		lcd_flags |= F_CLEAR_LCD
#define RESET_CLEAR_LCD		lcd_flags &= ~F_CLEAR_LCD
#define CLEAR_LCD			   (lcd_flags & F_CLEAR_LCD)

#define SET_BLINK_PARAM		lcd_flags |= F_BLINK_PARAM
#define RESET_BLINK_PARAM	lcd_flags &= ~F_BLINK_PARAM
#define TGL_BLINK_PARAM		lcd_flags ^= F_BLINK_PARAM
#define BLINK_PARAM			(lcd_flags & F_BLINK_PARAM)

#define SET_REINIT_LCD		lcd_flags |= F_REINIT_LCD
#define RESET_REINIT_LCD	lcd_flags &= ~F_REINIT_LCD
#define REINIT_LCD			(lcd_flags & F_REINIT_LCD)

#define BLINK     0x80

u8 cntr_reinit_lcd;

void delay_LCD(u32 time);
void PutChr_LCD(u8 status, u16 data);
void PutClear_LCD(void);
void InitLCD(void);
void PutStr_LCD(u8 x, u8 y, const u8 * data, u8 attrib);
void PutDgt_LCD(u8 x, u8 y, u8 numdigits, u16 digit, u8 attrib, u8 lead);

#endif /* LCD1602_H_ */
