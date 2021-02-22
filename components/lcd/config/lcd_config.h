#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"


extern SemaphoreHandle_t lcdSemaphMutex;

#define LCD_HOST HSPI_HOST
#define DMA_CHAN 2

#define PIN_NUM_MISO GPIO_NUM_12
#define PIN_NUM_MOSI GPIO_NUM_13
#define PIN_NUM_CLK GPIO_NUM_14
#define PIN_NUM_CS GPIO_NUM_15

#define DEV_RST_PIN GPIO_NUM_16
#define DEV_DC_PIN GPIO_NUM_21
#define DEV_BL_PIN GPIO_NUM_12

#define GPIO_OUTPUT_PIN_SEL ((1ULL << DEV_RST_PIN) | (1ULL << DEV_DC_PIN) | (1ULL << DEV_BL_PIN))

typedef struct
{
    uint16_t wide;
    uint16_t high;
    uint16_t scan_dir;
} lcd_device_t;

void lcd_delay_ms(uint16_t xms);
spi_device_handle_t lcd_get_spi(void);
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd);
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len);
void lcd_bl_pin_write(uint8_t sta);
void lcd_reset_pin_write(uint8_t sta);
void lcd_dev_init(void);
void lcd_dev_exit(void);

#endif
