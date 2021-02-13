#include "lcd_config.h"
#include "lcd_nokia_5110.h"
#include "lcd_ssd_1351.h"


uint8_t **lcd_buf = NULL;
lcd_device_t lcd_device = {0};

spi_device_handle_t lcd_spi;

void lcd_delay_ms(uint16_t xms)
{
    vTaskDelay(xms / portTICK_PERIOD_MS);
}

int DEV_Digital_Read(gpio_num_t _pin)
{
    return gpio_get_level(_pin);
}

spi_device_handle_t lcd_get_spi(void)
{
    return lcd_spi;
}

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));                   //Zero out the transaction
    t.length = 8;                               //Command is 8 bits
    t.tx_buffer = &cmd;                         //The data is the cmd itself
    t.user = (void *)0;                         //D/C needs to be set to 0
    ret = spi_device_polling_transmit(spi, &t); //Transmit!
    assert(ret == ESP_OK);                      //Should have had no issues.
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len == 0)
        return;                                 //no need to send anything
    memset(&t, 0, sizeof(t));                   //Zero out the transaction
    t.length = len * 8;                         //Len is in bytes, transaction length is in bits.
    t.tx_buffer = data;                         //Data
    t.user = (void *)1;                         //D/C needs to be set to 1
    ret = spi_device_polling_transmit(spi, &t); //Transmit!
    assert(ret == ESP_OK);                      //Should have had no issues.
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc = (int)t->user;
    gpio_set_level(DEV_DC_PIN, dc);
}

void lcd_spi_init(void)
{
    esp_err_t ret;
    gpio_pad_select_gpio(DEV_RST_PIN);                 // 选择要操作的GPIO
    gpio_pad_select_gpio(DEV_DC_PIN);                  // 选择要操作的GPIO
    gpio_pad_select_gpio(DEV_BL_PIN);                  // 选择要操作的GPIO
    gpio_set_direction(DEV_RST_PIN, GPIO_MODE_OUTPUT); // 设置GPIO为推挽输出模式
    gpio_set_direction(DEV_DC_PIN, GPIO_MODE_OUTPUT);  // 设置GPIO为推挽输出模式
    gpio_set_direction(DEV_BL_PIN, GPIO_MODE_OUTPUT);  // 设置GPIO为推挽输出模式

    spi_bus_config_t buscfg = {
        .miso_io_num = -1, //PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,      //Clock out at 10 MHz
        .mode = 0,                               //SPI mode 0
        .spics_io_num = PIN_NUM_CS,              //CS pin
        .queue_size = 128,                       //We want to be able to queue 7 transactions at a time
        .flags = SPI_DEVICE_3WIRE,               //Set half duplex mode (Full duplex mode can also be set by commenting this line
        .pre_cb = lcd_spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
    };

    //Initialize the SPI bus
    ret = spi_bus_initialize(LCD_HOST, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret = spi_bus_add_device(LCD_HOST, &devcfg, &lcd_spi);
    ESP_ERROR_CHECK(ret);
}

void lcd_bl_pin_write(uint8_t sta)
{
    if (sta)
        gpio_set_level(DEV_BL_PIN, 1);
    else
        gpio_set_level(DEV_BL_PIN, 0);
}

void lcd_reset_pin_write(uint8_t sta)
{
    if (sta)
        gpio_set_level(DEV_RST_PIN, 1);
    else
        gpio_set_level(DEV_RST_PIN, 0);
}

void lcd_dev_init(void)
{
    lcd_spi_init();
    // Nokia_Init();
    SSD1351_Init();
}

void lcd_dev_exit(void)
{
    printf("Exit dev_fifo ok!");
}
