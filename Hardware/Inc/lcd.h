#ifndef _OLED_H
#define _OLED_H

#include "ch32v30x.h"


#define RED 0XF800    // ��ɫ
#define GREEN 0X07E0  // ��ɫ
#define BLUE 0X001F   // ��ɫ
#define WHITE 0XFFFF  // ��ɫ

#define TFT_COLUMN_NUMBER 240
#define TFT_LINE_NUMBER 240
#define TFT_COLUMN_OFFSET 0

#define PIC_NUM 28800  // ͼƬ���ݴ�С


/**********SPI���ŷ��䣬����TFT��������ʵ������޸�*********/

#define SPI_SCK_0 GPIOC->BCR = GPIO_Pin_5
#define SPI_SCK_1 GPIOC->BSHR = GPIO_Pin_5
#define SPI_SDA_0 GPIOC->BCR = GPIO_Pin_6
#define SPI_SDA_1 GPIOC->BSHR = GPIO_Pin_6
#define SPI_RST_0 GPIOC->BCR = GPIO_Pin_7
#define SPI_RST_1 GPIOC->BSHR = GPIO_Pin_7
#define SPI_DC_0 GPIOC->BCR = GPIO_Pin_8
#define SPI_DC_1 GPIOC->BSHR = GPIO_Pin_8
#define SPI_BLK_0 GPIOC->BCR = GPIO_Pin_9
#define SPI_BLK_1 GPIOC->BSHR = GPIO_Pin_9


void SYS_init (unsigned char PLL);
void IO_init (void);
void TFT_init (void);
void TFT_full (unsigned int color);
void TFT_clear (void);
void Picture_Display (const unsigned char *ptr_pic);
void LCD_FillRect (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color);
void LCD_DrawChar (unsigned int x, unsigned int y, unsigned int color, unsigned int bg, char c);
void LCD_DrawString (unsigned int x, unsigned int y, unsigned int color, unsigned int bg, const char *s);

#endif
