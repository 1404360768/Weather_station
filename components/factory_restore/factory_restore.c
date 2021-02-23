/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <string.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_sleep.h"
#include "nvs_flash.h"

#define FACTORY_QUICK_REBOOT_TIMEOUT (CONFIG_FACTORY_QUICK_REBOOT_TIMEOUT * 1000)
#define FACTORY_QUICK_REBOOT_MAX_TIMES CONFIG_FACTORY_QUICK_REBOOT_MAX_TIMES
#define FACTORY_QUICK_REBOOT_TIMES "q_rt"
#define FACTORY_QUICK_REBOOT_TIMES_FLAG "q_rt_1"

#define AWSS_KV_RST "awss.rst"

static const char *TAG = "factory_rst";

static esp_err_t factory_restore_handle(void)
{
    nvs_handle_t my_handle;
    esp_err_t ret = ESP_OK;
    uint8_t quick_reboot_times = 0;

    /**<如果设备在指令时间内重新启动，则event_mdoe值将增加一*/
    ret = nvs_open(FACTORY_QUICK_REBOOT_TIMES, NVS_READWRITE, &my_handle); //尝试打开外部flash 读取wifi配置信息
    if (ret != ESP_OK)
    {
        nvs_close(my_handle);
        ESP_LOGE(TAG, "nvs_open failed.");
    }
    nvs_get_u8(my_handle, FACTORY_QUICK_REBOOT_TIMES_FLAG, &quick_reboot_times);
    quick_reboot_times++;
    ESP_ERROR_CHECK(nvs_set_u8(my_handle, FACTORY_QUICK_REBOOT_TIMES_FLAG, quick_reboot_times)); //存储计数
    ESP_ERROR_CHECK(nvs_commit(my_handle));                                                      //提交保存信息
    nvs_close(my_handle);                                                                        //关闭nvs_flash

    if (quick_reboot_times >= FACTORY_QUICK_REBOOT_MAX_TIMES)
    {
        ESP_LOGW(TAG, "factory restore........................");
        //重新配网
        // conn_mgr_reset_wifi_config();
    }
    else
    {
        ESP_LOGI(TAG, "quick reboot times %d, don't need to restore", quick_reboot_times);
    }

    return ret;
}

static void factory_restore_timer_handler(void *timer)
{
    if (!xTimerStop(timer, 0))
    {
        ESP_LOGE(TAG, "xTimerStop timer %p", timer);
    }

    if (!xTimerDelete(timer, 0))
    {
        ESP_LOGE(TAG, "xTimerDelete timer %p", timer);
    }

    /*清除重启时间记录*/
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(FACTORY_QUICK_REBOOT_TIMES, NVS_READWRITE, &my_handle); //尝试打开外部flash 读取wifi配置信息
    if (err != ESP_OK)
    {
        nvs_close(my_handle);
        ESP_LOGE(TAG, "nvs_open failed.");
    }
    ESP_ERROR_CHECK(nvs_set_u8(my_handle, FACTORY_QUICK_REBOOT_TIMES_FLAG, 0)); //存储计数
    ESP_ERROR_CHECK(nvs_commit(my_handle));                                     //提交保存信息
    nvs_close(my_handle);                                                       //关闭nvs_flash

    ESP_LOGI(TAG, "Quick reboot timeout, clear reboot times");
}

esp_err_t factory_restore_init(void)
{
    TimerHandle_t timer = xTimerCreate("factory_clear", FACTORY_QUICK_REBOOT_TIMEOUT / portTICK_RATE_MS,
                                       false, NULL, factory_restore_timer_handler);

    xTimerStart(timer, portMAX_DELAY);

    return factory_restore_handle();
}
