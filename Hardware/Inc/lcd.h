#ifndef _OLED_H
#define _OLED_H

#include "ch32v30x.h"

#define RED 0XF800
#define GREEN 0X07E0
#define BLUE 0X001F
#define WHITE 0XFFFF

#define TFT_COLUMN_NUMBER 240
#define TFT_LINE_NUMBER 240
#define TFT_COLUMN_OFFSET 0

#define PIC_NUM 28800


#define LCD_GPIO GPIOE
#define LCD_SCK_PIN GPIO_Pin_0
#define LCD_SDA_PIN GPIO_Pin_1
#define LCD_RST_PIN GPIO_Pin_2
#define LCD_DC_PIN GPIO_Pin_3
#define LCD_BLK_PIN GPIO_Pin_4

#define SPI_SCK_0 LCD_GPIO->BCR = LCD_SCK_PIN
#define SPI_SCK_1 LCD_GPIO->BSHR = LCD_SCK_PIN
#define SPI_SDA_0 LCD_GPIO->BCR = LCD_SDA_PIN
#define SPI_SDA_1 LCD_GPIO->BSHR = LCD_SDA_PIN
#define SPI_RST_0 LCD_GPIO->BCR = LCD_RST_PIN
#define SPI_RST_1 LCD_GPIO->BSHR = LCD_RST_PIN
#define SPI_DC_0 LCD_GPIO->BCR = LCD_DC_PIN
#define SPI_DC_1 LCD_GPIO->BSHR = LCD_DC_PIN
#define SPI_BLK_0 LCD_GPIO->BCR = LCD_BLK_PIN
#define SPI_BLK_1 LCD_GPIO->BSHR = LCD_BLK_PIN


void SYS_init (unsigned char PLL);
void IO_init (void);
void LCD_GPIOE_Init(void);
void TFT_init (void);
void TFT_full (unsigned int color);
void TFT_clear (void);
void Picture_Display (const unsigned char *ptr_pic);
void LCD_FillRect (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color);
void LCD_DrawPoint (unsigned int x, unsigned int y, unsigned int color);
void LCD_DrawChar (unsigned int x, unsigned int y, unsigned int color, unsigned int bg, char c);
void LCD_DrawString (unsigned int x, unsigned int y, unsigned int color, unsigned int bg, const char *s);
void LCD_DrawChar32 (unsigned int x, unsigned int y, unsigned int color, unsigned int bg, char c);
void LCD_DrawString32 (unsigned int x, unsigned int y, unsigned int color, unsigned int bg, const char *s);
void LCD_DrawChinese32 (unsigned int x, unsigned int y, unsigned int color, unsigned int bg, unsigned int index);
void LCD_DrawChar32Transparent (unsigned int x, unsigned int y, unsigned int color, char c);
void LCD_DrawString32Transparent (unsigned int x, unsigned int y, unsigned int color, const char *s);
void LCD_DrawChinese32Transparent (unsigned int x, unsigned int y, unsigned int color, unsigned int index);
void LCD_DrawImageFull (const unsigned char *ptr_pic);
void LCD_DrawImageRegion (const unsigned char *ptr_pic, unsigned int x, unsigned int y, unsigned int w, unsigned int h);

#endif
