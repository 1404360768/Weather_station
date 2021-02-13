#include "lcd_ssd_1351.h"
#include "image.h"

extern uint8_t **lcd_buf;
lcd_device_t lcd_device;

static void SSD1351_Reset()
{
    lcd_reset_pin_write(1);
    lcd_delay_ms(500);
    lcd_reset_pin_write(0);
    lcd_delay_ms(500);
    lcd_reset_pin_write(1);
    lcd_delay_ms(500);
}

static void SSD1351_WriteCommand(uint8_t cmd)
{
    lcd_cmd(lcd_get_spi(), cmd);
}

static void SSD1351_WriteData(uint8_t *buff, size_t buff_size)
{
    lcd_data(lcd_get_spi(), buff, buff_size);
}

static void SSD1351_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // column address set
    SSD1351_WriteCommand(0x15); // SETCOLUMN
    {
        uint8_t data[] = {x0 & 0xFF, x1 & 0xFF};
        SSD1351_WriteData(data, sizeof(data));
    }

    // row address set
    SSD1351_WriteCommand(0x75); // SETROW
    {
        uint8_t data[] = {y0 & 0xFF, y1 & 0xFF};
        SSD1351_WriteData(data, sizeof(data));
    }

    // write to RAM
    SSD1351_WriteCommand(0x5C); // WRITERAM
}

void SSD1351_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if ((x >= SSD1351_WIDTH) || (y >= SSD1351_HEIGHT))
        return;

    SSD1351_SetAddressWindow(x, y, x + 1, y + 1);
    uint8_t data[] = {color >> 8, color & 0xFF};
    SSD1351_WriteData(data, sizeof(data));
}

static void SSD1351_WriteChar(uint16_t x, uint16_t y, char ch, ssFONT font, uint16_t color, uint16_t bgcolor)
{
    uint32_t i, b, j;

    SSD1351_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);

    for (i = 0; i < font.height; i++)
    {
        b = font.data[(ch - 32) * font.height + i];
        for (j = 0; j < font.width; j++)
        {
            if ((b << j) & 0x8000)
            {
                uint8_t data[] = {color >> 8, color & 0xFF};
                SSD1351_WriteData(data, sizeof(data));
            }
            else
            {
                uint8_t data[] = {bgcolor >> 8, bgcolor & 0xFF};
                SSD1351_WriteData(data, sizeof(data));
            }
        }
    }
}

void SSD1351_WriteString(uint16_t x, uint16_t y, const char *str, ssFONT font, uint16_t color, uint16_t bgcolor)
{
    while (*str)
    {
        if (x + font.width >= SSD1351_WIDTH)
        {
            x = 0;
            y += font.height;
            if (y + font.height >= SSD1351_HEIGHT)
            {
                break;
            }

            if (*str == ' ')
            {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        SSD1351_WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }
}

void SSD1351_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    // clipping
    if ((x >= SSD1351_WIDTH) || (y >= SSD1351_HEIGHT))
        return;
    if ((x + w - 1) >= SSD1351_WIDTH)
        w = SSD1351_WIDTH - x;
    if ((y + h - 1) >= SSD1351_HEIGHT)
        h = SSD1351_HEIGHT - y;

    SSD1351_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    uint8_t data[] = {color >> 8, color & 0xFF};
    for (y = h; y > 0; y--)
    {
        for (x = w; x > 0; x--)
        {
            SSD1351_WriteData(data, sizeof(data));
        }
    }
}

void SSD1351_FillScreen(uint16_t color)
{
    SSD1351_FillRectangle(0, 0, SSD1351_WIDTH, SSD1351_HEIGHT, color);
}

void SSD1351_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
    if ((x >= SSD1351_WIDTH) || (y >= SSD1351_HEIGHT))
        return;
    if ((x + w - 1) >= SSD1351_WIDTH)
        return;
    if ((y + h - 1) >= SSD1351_HEIGHT)
        return;

    SSD1351_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    for (; y < h; y++)
    {
        SSD1351_WriteData((uint8_t *)data + y * (sizeof(uint16_t) * w), sizeof(uint16_t) * w);
    }
}

void SSD1351_InvertColors(bool invert)
{
    SSD1351_WriteCommand(invert ? 0xA7 /* INVERTDISPLAY */ : 0xA6 /* NORMALDISPLAY */);
}

void SSD1351_Init()
{
    SSD1351_Reset();
    // command list is based on https://github.com/adafruit/Adafruit-SSD1351-library

    SSD1351_WriteCommand(0xFD); // COMMANDLOCK
    {
        uint8_t data[] = {0x12};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xFD); // COMMANDLOCK
    {
        uint8_t data[] = {0xB1};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xAE); // DISPLAYOFF
    SSD1351_WriteCommand(0xB3); // CLOCKDIV
    SSD1351_WriteCommand(0xF1); // 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
    SSD1351_WriteCommand(0xCA); // MUXRATIO
    {
        uint8_t data[] = {0x5F}; // 127/96
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xA0); // SETREMAP
    {
        uint8_t data[] = {0x74};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0x15); // SETCOLUMN
    {
        uint8_t data[] = {0x00, 0x7F};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0x75); // SETROW
    {
        uint8_t data[] = {0x00, 0x7F};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xA1); // STARTLINE
    {
        uint8_t data[] = {0x00}; // 96 if display height == 96
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xA2); // DISPLAYOFFSET
    {
        uint8_t data[] = {0x00};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xB5); // SETGPIO
    {
        uint8_t data[] = {0x00};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xAB); // FUNCTIONSELECT
    {
        uint8_t data[] = {0x01};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xB1); // PRECHARGE
    {
        uint8_t data[] = {0x32};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xBE); // VCOMH
    {
        uint8_t data[] = {0x05};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xA6); // NORMALDISPLAY (don't invert)
    SSD1351_WriteCommand(0xC1); // CONTRASTABC
    {
        uint8_t data[] = {0xC8, 0x80, 0xC8};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xC7); // CONTRASTMASTER
    {
        uint8_t data[] = {0x0F};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xB4); // SETVSL
    {
        uint8_t data[] = {0xA0, 0xB5, 0x55};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xB6); // PRECHARGE2
    {
        uint8_t data[] = {0x01};
        SSD1351_WriteData(data, sizeof(data));
    }
    SSD1351_WriteCommand(0xAF); // DISPLAYON

    SSD1351_FillScreen(SSD1351_BLACK);

    // SSD1351_WriteString(0, 0, "Font_7x10, red on black, lorem ipsum dolor sit amet", Font_7x10, SSD1351_RED, SSD1351_BLACK);
    // SSD1351_WriteString(0, 3 * 10, "Font_11x18, green, lorem ipsum", Font_11x18, SSD1351_GREEN, SSD1351_BLACK);
    // SSD1351_WriteString(0, 3 * 10 + 3 * 18, "Font_16x26, blue, lorem ipsum dolor sit amet", Font_16x26, SSD1351_BLUE, SSD1351_BLACK);

    // SSD1351_DrawImage(0, 0, 128, 128, (const uint16_t *)test_img_128x128);
}
