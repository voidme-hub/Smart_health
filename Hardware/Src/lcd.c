#include "lcd.h"
#include "ch32v30x_rcc.h"

static const unsigned char lcd_font_space[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char lcd_font_dot[8] = {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00};
static const unsigned char lcd_font_dash[8] = {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00};
static const unsigned char lcd_font_0[8] = {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00};
static const unsigned char lcd_font_1[8] = {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00};
static const unsigned char lcd_font_2[8] = {0x3C,0x66,0x06,0x1C,0x30,0x60,0x7E,0x00};
static const unsigned char lcd_font_3[8] = {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00};
static const unsigned char lcd_font_4[8] = {0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00};
static const unsigned char lcd_font_5[8] = {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00};
static const unsigned char lcd_font_6[8] = {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00};
static const unsigned char lcd_font_7[8] = {0x7E,0x66,0x06,0x0C,0x18,0x18,0x18,0x00};
static const unsigned char lcd_font_8[8] = {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00};
static const unsigned char lcd_font_9[8] = {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00};
static const unsigned char lcd_font_B[8] = {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00};
static const unsigned char lcd_font_T[8] = {0x7E,0x5A,0x18,0x18,0x18,0x18,0x3C,0x00};
static const unsigned char lcd_font_H[8] = {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00};
static const unsigned char lcd_font_R[8] = {0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00};
static const unsigned char lcd_font_S[8] = {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00};
static const unsigned char lcd_font_M[8] = {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00};
static const unsigned char lcd_font_Q[8] = {0x3C,0x66,0x66,0x66,0x6E,0x3C,0x0E,0x00};
static const unsigned char lcd_font_colon[8] = {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00};

static const unsigned char *LCD_GetCharBitmap(char c)
{
    switch (c) {
    case '0': return lcd_font_0;
    case '1': return lcd_font_1;
    case '2': return lcd_font_2;
    case '3': return lcd_font_3;
    case '4': return lcd_font_4;
    case '5': return lcd_font_5;
    case '6': return lcd_font_6;
    case '7': return lcd_font_7;
    case '8': return lcd_font_8;
    case '9': return lcd_font_9;
    case 'B': return lcd_font_B;
    case 'T': return lcd_font_T;
    case 'H': return lcd_font_H;
    case 'R': return lcd_font_R;
    case 'S': return lcd_font_S;
    case 'M': return lcd_font_M;
    case 'Q': return lcd_font_Q;
    case ':': return lcd_font_colon;
    case '.': return lcd_font_dot;
    case '-': return lcd_font_dash;
    case ' ': return lcd_font_space;
    default:  return lcd_font_space;
    }
}




// const unsigned char chines_word[][32] =  // ������
//     {
//         0x00, 0x00, 0xE4, 0x3F, 0x28, 0x20, 0x28, 0x25, 0x81, 0x08, 0x42, 0x10, 0x02, 0x02, 0x08, 0x02,
//         0xE8, 0x3F, 0x04, 0x02, 0x07, 0x07, 0x84, 0x0A, 0x44, 0x12, 0x34, 0x62, 0x04, 0x02, 0x00, 0x02, /*"��",0*/

//         0x88, 0x20, 0x88, 0x24, 0x88, 0x24, 0x88, 0x24, 0x88, 0x24, 0xBF, 0x24, 0x88, 0x24, 0x88, 0x24,
//         0x88, 0x24, 0x88, 0x24, 0x88, 0x24, 0xB8, 0x24, 0x87, 0x24, 0x42, 0x24, 0x40, 0x20, 0x20, 0x20, /*"��",1*/

//         0x80, 0x00, 0x80, 0x00, 0x40, 0x01, 0x20, 0x02, 0x10, 0x04, 0x08, 0x08, 0xF4, 0x17, 0x83, 0x60,
//         0x80, 0x00, 0xFC, 0x1F, 0x80, 0x00, 0x88, 0x08, 0x90, 0x08, 0x90, 0x04, 0xFF, 0x7F, 0x00, 0x00, /*"��",2*/

//         0x80, 0x00, 0x82, 0x00, 0x84, 0x0F, 0x44, 0x08, 0x20, 0x04, 0xF0, 0x3F, 0x27, 0x22, 0x24, 0x22,
//         0xE4, 0x3F, 0x04, 0x05, 0x84, 0x0C, 0x84, 0x54, 0x44, 0x44, 0x24, 0x78, 0x0A, 0x00, 0xF1, 0x7F, /*"��",3*/

//         0xF8, 0x0F, 0x08, 0x08, 0xF8, 0x0F, 0x08, 0x08, 0xF8, 0x0F, 0x00, 0x00, 0xFC, 0x3F, 0x04, 0x00,
//         0xF4, 0x1F, 0x04, 0x00, 0xFC, 0x7F, 0x94, 0x10, 0x14, 0x09, 0x12, 0x06, 0x52, 0x18, 0x31, 0x60, /*"��",4*/

//         0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xFC, 0x1F, 0x84, 0x10, 0x84, 0x10, 0x84, 0x10, 0xFC, 0x1F,
//         0x84, 0x10, 0x84, 0x10, 0x84, 0x10, 0xFC, 0x1F, 0x84, 0x50, 0x80, 0x40, 0x80, 0x40, 0x00, 0x7F, /*"��",5*/

//         0x00, 0x00, 0xFE, 0x1F, 0x00, 0x08, 0x00, 0x04, 0x00, 0x02, 0x80, 0x01, 0x80, 0x00, 0xFF, 0x7F,
//         0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xA0, 0x00, 0x40, 0x00, /*"��",6*/
// };



void SYS_init (unsigned char PLL) {
    uint32_t pll_mul;

#ifdef CH32V30x_D8
    if (PLL < 2) {
        PLL = 2;
    }
    if (PLL > 16) {
        PLL = 16;
    }

    switch (PLL) {
    case 2:
        pll_mul = RCC_PLLMul_2;
        break;
    case 3:
        pll_mul = RCC_PLLMul_3;
        break;
    case 4:
        pll_mul = RCC_PLLMul_4;
        break;
    case 5:
        pll_mul = RCC_PLLMul_5;
        break;
    case 6:
        pll_mul = RCC_PLLMul_6;
        break;
    case 7:
        pll_mul = RCC_PLLMul_7;
        break;
    case 8:
        pll_mul = RCC_PLLMul_8;
        break;
    case 9:
        pll_mul = RCC_PLLMul_9;
        break;
    case 10:
        pll_mul = RCC_PLLMul_10;
        break;
    case 11:
        pll_mul = RCC_PLLMul_11;
        break;
    case 12:
        pll_mul = RCC_PLLMul_12;
        break;
    case 13:
        pll_mul = RCC_PLLMul_13;
        break;
    case 14:
        pll_mul = RCC_PLLMul_14;
        break;
    case 15:
        pll_mul = RCC_PLLMul_15;
        break;
    default:
        pll_mul = RCC_PLLMul_16;
        break;
    }
#else
    if (PLL < 3) {
        PLL = 3;
    }
    if (PLL > 16) {
        PLL = 16;
    }

    switch (PLL) {
    case 3:
        pll_mul = RCC_PLLMul_3_EXTEN;
        break;
    case 4:
        pll_mul = RCC_PLLMul_4_EXTEN;
        break;
    case 5:
        pll_mul = RCC_PLLMul_5_EXTEN;
        break;
    case 6:
        pll_mul = RCC_PLLMul_6_EXTEN;
        break;
    case 7:
        pll_mul = RCC_PLLMul_7_EXTEN;
        break;
    case 8:
        pll_mul = RCC_PLLMul_8_EXTEN;
        break;
    case 9:
        pll_mul = RCC_PLLMul_9_EXTEN;
        break;
    case 10:
        pll_mul = RCC_PLLMul_10_EXTEN;
        break;
    case 11:
        pll_mul = RCC_PLLMul_11_EXTEN;
        break;
    case 12:
        pll_mul = RCC_PLLMul_12_EXTEN;
        break;
    case 13:
        pll_mul = RCC_PLLMul_13_EXTEN;
        break;
    case 14:
        pll_mul = RCC_PLLMul_14_EXTEN;
        break;
    case 15:
        pll_mul = RCC_PLLMul_15_EXTEN;
        break;
    default:
        pll_mul = RCC_PLLMul_16_EXTEN;
        break;
    }
#endif

    RCC_DeInit();
    RCC_HSEConfig (RCC_HSE_ON);
    if (RCC_WaitForHSEStartUp() == READY) {
        RCC_HCLKConfig (RCC_SYSCLK_Div1);
        RCC_PCLK1Config (RCC_HCLK_Div2);
        RCC_PCLK2Config (RCC_HCLK_Div1);

        RCC_PREDIV1Config (RCC_PREDIV1_Source_HSE, RCC_PREDIV1_Div1);
#ifdef CH32V30x_D8
        RCC_PLLConfig (RCC_PLLSource_HSE_Div1, pll_mul);
#else
        RCC_PLLConfig (RCC_PLLSource_PREDIV1, pll_mul);
#endif
        RCC_PLLCmd (ENABLE);
        while (RCC_GetFlagStatus (RCC_FLAG_PLLRDY) == RESET) {
        }
        RCC_SYSCLKConfig (RCC_SYSCLKSource_PLLCLK);
        while (RCC_GetSYSCLKSource() != 0x08) {
        }
    }
    SystemCoreClockUpdate();
}

void IO_init (void) {
    GPIO_InitTypeDef gpio_init;

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOC, ENABLE);

    gpio_init.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (GPIOC, &gpio_init);

    GPIO_SetBits (GPIOC, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
}

void Delay_us (unsigned int _us_time) {
    volatile unsigned char x = 0;
    for (; _us_time > 0; _us_time--) {
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
        x++;
    }
}

void Delay_ms (unsigned int _ms_time) {
    volatile unsigned int i;
    for (i = 0; i < _ms_time; i++);
}

static void SPI_Delay (void) {
    volatile unsigned int i;
    for (i = 0; i < 12; i++) {
    }
}

/*************SPI���ú���*******************
SCL����ʱ�͵�ƽ����һ�������ز���
ģ��SPI
******************************************/

/**************************SPIģ�鷢�ͺ���************************************************

 *************************************************************************/
void SPI_SendByte (unsigned char byte)  // ��Һ����дһ��8λ����
{

    unsigned char counter;

    for (counter = 0; counter < 8; counter++) {
        SPI_SCK_0;
        if ((byte & 0x80) == 0) {
            SPI_SDA_0;
        } else
            SPI_SDA_1;
        byte = byte << 1;
        SPI_SCK_1;
    }
    SPI_SCK_0;
}

void TFT_SEND_CMD (unsigned char o_command) {
    SPI_DC_0;
    SPI_SCK_0;
    SPI_SendByte (o_command);
}

// ��Һ����дһ��8λ����
void TFT_SEND_DATA (unsigned char o_data) {
    SPI_DC_1;
    SPI_SCK_0;
    SPI_SendByte (o_data);
}

void TFT_clear (void) {
    unsigned int ROW, column;
    TFT_SEND_CMD (0x2a);   // Column address set
    TFT_SEND_DATA (0x00);  // start column
    TFT_SEND_DATA (0x00);
    TFT_SEND_DATA (0x00);  // end column
    TFT_SEND_DATA (0xF0);

    TFT_SEND_CMD (0x2b);                         // Row address set
    TFT_SEND_DATA (0x00);                        // start row
    TFT_SEND_DATA (0x00);
    TFT_SEND_DATA (0x00);                        // end row
    TFT_SEND_DATA (0xF0);
    TFT_SEND_CMD (0x2C);                         // Memory write
    for (ROW = 0; ROW < TFT_LINE_NUMBER; ROW++)  // ROW loop
    {

        for (column = 0; column < TFT_COLUMN_NUMBER; column++)  // column loop
        {

            TFT_SEND_DATA (0xFF);
            TFT_SEND_DATA (0xFF);
        }
    }
}

void TFT_full (unsigned int color) {
    unsigned int ROW, column;
    TFT_SEND_CMD (0x2a);   // Column address set
    TFT_SEND_DATA (0x00);  // start column
    TFT_SEND_DATA (0x00);
    TFT_SEND_DATA (0x00);  // end column
    TFT_SEND_DATA (0xF0);

    TFT_SEND_CMD (0x2b);                         // Row address set
    TFT_SEND_DATA (0x00);                        // start row
    TFT_SEND_DATA (0x00);
    TFT_SEND_DATA (0x00);                        // end row
    TFT_SEND_DATA (0xF0);
    TFT_SEND_CMD (0x2C);                         // Memory write
    for (ROW = 0; ROW < TFT_LINE_NUMBER; ROW++)  // ROW loop
    {

        for (column = 0; column < TFT_COLUMN_NUMBER; column++)  // column loop
        {

            TFT_SEND_DATA (color >> 8);
            TFT_SEND_DATA (color);
        }
    }
}

void TFT_init (void)  ////ST7789V2
{
    SPI_SCK_1;        // �ر�ע�⣡��
    SPI_BLK_1;
    SPI_RST_0;
    Delay_ms (1000);
    SPI_RST_1;
    Delay_ms (1000);
    TFT_SEND_CMD (0x11);  // Sleep Out
    Delay_ms (120);       // DELAY120ms
                     //-----------------------ST7789V Frame rate setting-----------------//
    //************************************************
    TFT_SEND_CMD (0x3A);  // 65k mode
    TFT_SEND_DATA (0x05);
    TFT_SEND_CMD (0xC5);  // VCOM
    TFT_SEND_DATA (0x1A);
    TFT_SEND_CMD (0x36);  // ��Ļ��ʾ��������
    TFT_SEND_DATA (0x00);
    //-------------ST7789V Frame rate setting-----------//
    TFT_SEND_CMD (0xb2);  // Porch Setting
    TFT_SEND_DATA (0x05);
    TFT_SEND_DATA (0x05);
    TFT_SEND_DATA (0x00);
    TFT_SEND_DATA (0x33);
    TFT_SEND_DATA (0x33);

    TFT_SEND_CMD (0xb7);   // Gate Control
    TFT_SEND_DATA (0x05);  // 12.2v   -10.43v
    //--------------ST7789V Power setting---------------//
    TFT_SEND_CMD (0xBB);  // VCOM
    TFT_SEND_DATA (0x3F);

    TFT_SEND_CMD (0xC0);  // Power control
    TFT_SEND_DATA (0x2c);

    TFT_SEND_CMD (0xC2);  // VDV and VRH Command Enable
    TFT_SEND_DATA (0x01);

    TFT_SEND_CMD (0xC3);   // VRH Set
    TFT_SEND_DATA (0x0F);  // 4.3+( vcom+vcom offset+vdv)

    TFT_SEND_CMD (0xC4);   // VDV Set
    TFT_SEND_DATA (0x20);  // 0v

    TFT_SEND_CMD (0xC6);   // Frame Rate Control in Normal Mode
    TFT_SEND_DATA (0X01);  // 111Hz

    TFT_SEND_CMD (0xd0);   // Power Control 1
    TFT_SEND_DATA (0xa4);
    TFT_SEND_DATA (0xa1);

    TFT_SEND_CMD (0xE8);  // Power Control 1
    TFT_SEND_DATA (0x03);

    TFT_SEND_CMD (0xE9);  // Equalize time control
    TFT_SEND_DATA (0x09);
    TFT_SEND_DATA (0x09);
    TFT_SEND_DATA (0x08);
    //---------------ST7789V gamma setting-------------//
    TFT_SEND_CMD (0xE0);  // Set Gamma
    TFT_SEND_DATA (0xD0);
    TFT_SEND_DATA (0x05);
    TFT_SEND_DATA (0x09);
    TFT_SEND_DATA (0x09);
    TFT_SEND_DATA (0x08);
    TFT_SEND_DATA (0x14);
    TFT_SEND_DATA (0x28);
    TFT_SEND_DATA (0x33);
    TFT_SEND_DATA (0x3F);
    TFT_SEND_DATA (0x07);
    TFT_SEND_DATA (0x13);
    TFT_SEND_DATA (0x14);
    TFT_SEND_DATA (0x28);
    TFT_SEND_DATA (0x30);

    TFT_SEND_CMD (0XE1);  // Set Gamma
    TFT_SEND_DATA (0xD0);
    TFT_SEND_DATA (0x05);
    TFT_SEND_DATA (0x09);
    TFT_SEND_DATA (0x09);
    TFT_SEND_DATA (0x08);
    TFT_SEND_DATA (0x03);
    TFT_SEND_DATA (0x24);
    TFT_SEND_DATA (0x32);
    TFT_SEND_DATA (0x32);
    TFT_SEND_DATA (0x3B);
    TFT_SEND_DATA (0x14);
    TFT_SEND_DATA (0x13);
    TFT_SEND_DATA (0x28);
    TFT_SEND_DATA (0x2F);

    TFT_SEND_CMD (0x21);  // ����

    TFT_SEND_CMD (0x29);  // ������ʾ
}

void LCD_FillRect (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color)
{
    unsigned int row;
    unsigned int col;
    unsigned int x_end = x + w - 1;
    unsigned int y_end = y + h - 1;

    TFT_SEND_CMD (0x2a);
    TFT_SEND_DATA (x >> 8);
    TFT_SEND_DATA (x);
    TFT_SEND_DATA (x_end >> 8);
    TFT_SEND_DATA (x_end);

    TFT_SEND_CMD (0x2b);
    TFT_SEND_DATA (y >> 8);
    TFT_SEND_DATA (y);
    TFT_SEND_DATA (y_end >> 8);
    TFT_SEND_DATA (y_end);

    TFT_SEND_CMD (0x2C);
    for (row = 0; row < h; row++) {
        for (col = 0; col < w; col++) {
            TFT_SEND_DATA (color >> 8);
            TFT_SEND_DATA (color);
        }
    }
}

void LCD_DrawChar (unsigned int x, unsigned int y, unsigned int color, unsigned int bg, char c)
{
    unsigned int row;
    unsigned int col;
    const unsigned char *bitmap = LCD_GetCharBitmap(c);

    TFT_SEND_CMD (0x2a);
    TFT_SEND_DATA (x >> 8);
    TFT_SEND_DATA (x);
    TFT_SEND_DATA ((x + 7) >> 8);
    TFT_SEND_DATA (x + 7);

    TFT_SEND_CMD (0x2b);
    TFT_SEND_DATA (y >> 8);
    TFT_SEND_DATA (y);
    TFT_SEND_DATA ((y + 7) >> 8);
    TFT_SEND_DATA (y + 7);
    TFT_SEND_CMD (0x2C);

    for (row = 0; row < 8; row++) {
        unsigned char line = bitmap[row];
        for (col = 0; col < 8; col++) {
            if (line & (0x80 >> col)) {
                TFT_SEND_DATA (color >> 8);
                TFT_SEND_DATA (color);
            } else {
                TFT_SEND_DATA (bg >> 8);
                TFT_SEND_DATA (bg);
            }
        }
    }
}

void LCD_DrawString (unsigned int x, unsigned int y, unsigned int color, unsigned int bg, const char *s)
{
    unsigned int pos = x;
    while (*s) {
        if (pos + 7 >= TFT_COLUMN_NUMBER) {
            break;
        }
        LCD_DrawChar (pos, y, color, bg, *s++);
        pos += 8;
    }
}

// void display_char16_16 (unsigned int x, unsigned int y, unsigned long color, unsigned char word_serial_number) {
//     unsigned int column;
//     unsigned char tm = 0, temp = 0, xxx = 0;

//     TFT_SEND_CMD (0x2a);     // Column address set
//     TFT_SEND_DATA (x >> 8);  // start column
//     TFT_SEND_DATA (x);
//     x = x + 15;
//     TFT_SEND_DATA (x >> 8);  // end column
//     TFT_SEND_DATA (x);

//     TFT_SEND_CMD (0x2b);     // Row address set
//     TFT_SEND_DATA (y >> 8);  // start row
//     TFT_SEND_DATA (y);
//     y = y + 15;
//     TFT_SEND_DATA (y >> 8);  // end row
//     TFT_SEND_DATA (y);
//     TFT_SEND_CMD (0x2C);     // Memory write


//     for (column = 0; column < 32; column++)  // column loop
//     {
//         temp = chines_word[word_serial_number][xxx];
//         for (tm = 0; tm < 8; tm++) {
//             if (temp & 0x01) {
//                 TFT_SEND_DATA (color >> 8);
//                 TFT_SEND_DATA (color);
//             } else {
//                 TFT_SEND_DATA (0XFF);
//                 TFT_SEND_DATA (0XFF);
//             }
//             temp >>= 1;
//         }
//         xxx++;
//     }
// }

void Picture_Display (const unsigned char *ptr_pic) {
    unsigned long number;
    TFT_SEND_CMD (0x2a);   // Column address set
    TFT_SEND_DATA (0x00);  // start column
    TFT_SEND_DATA (0x00);
    TFT_SEND_DATA (0x00);  // end column
    TFT_SEND_DATA (0x77);

    TFT_SEND_CMD (0x2b);   // Row address set
    TFT_SEND_DATA (0x00);  // start row
    TFT_SEND_DATA (0x00);
    TFT_SEND_DATA (0x00);  // end row
    TFT_SEND_DATA (0x78);
    TFT_SEND_CMD (0x2C);   // Memory write

    for (number = 0; number < PIC_NUM; number++) {
        //  data=*ptr_pic++;
        //  data=~data;
        TFT_SEND_DATA (*ptr_pic++);
    }
}
