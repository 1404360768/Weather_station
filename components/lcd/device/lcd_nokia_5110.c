#include "lcd_nokia_5110.h"

extern uint8_t **lcd_buf;

void Nokia_Write_Byte(uint8_t dat, uint8_t cmd)
{
    if (cmd == 0)
    {
        lcd_cmd(lcd_get_spi(), dat);
    }
    else
    {
        lcd_data(lcd_get_spi(), &dat, 1);
    }
}

void Nokia_Clear(void)
{
    for (uint8_t i = 0; i < NOKIA_5110_WIDE; i++) // 扫描NOKIA_5110_HIGH列
    {
        memset(lcd_buf[i], 0xFF, NOKIA_5110_HIGH / 8 * (sizeof(uint8_t)));
    }
}

void Nokia_Set_Position(uint8_t X, uint8_t Y)
{
    Nokia_Write_Byte(0x40 | Y, 0); // column
    Nokia_Write_Byte(0x80 | X, 0); // row
}

void Nokia_Write_Char_nMemory(uint8_t c)
{
    c -= 32;
    for (uint8_t line = 0; line < 5; line++)
    {
        Nokia_Write_Byte(Font8.data[c+line], 1);
    }
}

void Nokia_Write_Char(uint8_t x, uint8_t y, uint8_t c)
{
    c -= 32;
    for (uint8_t line = 0; line < 5; line++)
    {
        if ((x + line) >= NOKIA_5110_WIDE)
            x = 0;
        if ((y) >= NOKIA_5110_HIGH / 8)
            y = 0;
        lcd_buf[x + line][y] = Font8.data[c+line];
    }
}

void Nokia_Write_English_String(uint8_t x, uint8_t y, char *s)
{
    while (*s)
    {
        Nokia_Write_Char(x, y, *s);
        x = x + 6;
        s++;
    }
}

/**
 * 简单画点
 * @param y y还是以8个像素为一行的行
 * @param x x则是1个像素一列
 */
void Nokia_Draw_Point_nMemory(unsigned char y, unsigned char x)
{
    Nokia_Set_Position(y, x);
    Nokia_Write_Byte(0xFF, 1); // Nokia_DC = 0;		// 传送命令
}

//画点
//x:0~83
//y:0~47
//t:1 填充 0,清空
void Nokia_Draw_Point(uint8_t x, uint8_t y, uint8_t t)
{
    uint8_t i, m, n;
    i = y / 8;
    m = y % 8;
    n = 1 << m;
    if (t)
    {
        lcd_buf[x][i] |= n;
    }
    else
    {
        lcd_buf[x][i] = ~lcd_buf[x][i];
        lcd_buf[x][i] |= n;
        lcd_buf[x][i] = ~lcd_buf[x][i];
    }
}

/**
 * 刷新显示
 */
void Nokia_RefreshMemory(void)
{
    for (uint8_t i = 0; i < NOKIA_5110_HIGH / 8; i++)
    {
        Nokia_Set_Position(0, 0 + i);
        for (uint8_t n = 0; n < NOKIA_5110_WIDE; n++)
        {
            lcd_data(lcd_get_spi(), &lcd_buf[n][i], 1);
        }
        printf("这是第：%d列\r\n", i);
    }
}

int32_t Nokia_Request_Buff(void)
{
    int32_t size = 0;
    if (lcd_buf != NULL)
    {
        printf("lcd_buff is not NULL.......\r\n");
        size = -1;
    }
    else
    {
        // 为二维数组分配NOKIA_5110_WIDE行
        lcd_buf = (uint8_t **)malloc(NOKIA_5110_WIDE * (sizeof(uint8_t *)));
        for (uint8_t i = 0; i < NOKIA_5110_WIDE; i++) // 为二维数组分配NOKIA_5110_HIGH列
        {
            lcd_buf[i] = (uint8_t *)malloc(NOKIA_5110_HIGH / 8 * (sizeof(uint8_t)));
            memset(lcd_buf[i], 0, NOKIA_5110_HIGH / 8 * (sizeof(uint8_t)));
        }
        if ((lcd_buf != NULL) && (*lcd_buf != NULL))
            size = 1;
        //输出测试
        for (uint8_t i = 0; i < NOKIA_5110_HIGH / 8; ++i)
        {
            for (uint8_t j = 0; j < NOKIA_5110_WIDE; ++j)
            {
                printf("%d", lcd_buf[j][i]);
            }
            printf("\n");
        }
    }

    return size;
}

/**
 * 初始化
 */
void Nokia_Init(void)
{
    int32_t ret = Nokia_Request_Buff();
    if (ret < 0)
    {
        printf("Nokia_Request_Buff failed....\r\n");
        return;
    }

    lcd_bl_pin_write(0);
    lcd_delay_ms(1000);
    lcd_bl_pin_write(1);
    //复位脉冲
    lcd_reset_pin_write(1);
    lcd_delay_ms(2);
    lcd_reset_pin_write(0);
    lcd_delay_ms(2);
    lcd_reset_pin_write(1);

    Nokia_Write_Byte(0x21, 0); // 使用扩展命令设置LCD模式
    Nokia_Write_Byte(0xc8, 0); // 设置偏置电压
    Nokia_Write_Byte(0x06, 0); // 温度校正
    Nokia_Write_Byte(0x13, 0); // 1:48
    Nokia_Write_Byte(0x20, 0); // 使用基本命令
    Nokia_Clear();             // 清屏
    Nokia_Write_Byte(0x0c, 0); // 设定显示模式，正常显示
    // Nokia_Write_English_String(0, 0, "chen shi xiang");
    Nokia_Write_Char(0, 0, '1');
    Nokia_Write_Char(0, 1, '2');
    Nokia_Write_Char(0, 2, '3');
    Nokia_Write_Char(0, 3, '4');
    Nokia_Write_Char(0, 4, '5');
    Nokia_Write_Char(0, 5, '6');
    Nokia_Clear(); // 清屏
    Nokia_Write_Char(0, 0, 'A');
    Nokia_Write_Char(0, 1, 'B');
    Nokia_Write_Char(0, 2, 'C');
    Nokia_Write_Char(0, 3, 'D');
    Nokia_Write_Char(0, 4, 'E');
    Nokia_Write_Char(0, 5, 'F');
    Nokia_Write_English_String(8, 0, "chen");
    Nokia_RefreshMemory();
}
