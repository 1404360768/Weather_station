#include "lcd_task.h"
#include "lcd_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lcd_ssd_1351.h"

static const char *TAG = "lcd_task";

static void lcd_task(void *parm)
{
    char str[10];
    while (1)
    {
        sprintf(str, "%d", esp_get_free_heap_size()); //将esp_get_free_heap_size转为10进制表示的字符串。
        // SSD1351_WriteString(0, 0, str, Font_7x10, SSD1351_RED, SSD1351_BLACK);
        lcd_delay_ms(10);
        taskYIELD(); //主动要求切换任务
    }
}

void lcd_task_init(void)
{
    ESP_LOGI(TAG, "lcd_task start up");
    lcd_dev_init();
    xTaskCreate(lcd_task, "lcd_task", 4096, NULL, 3, NULL);
}
