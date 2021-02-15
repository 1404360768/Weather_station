/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "smartconfig_main.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY

static const char *TAG = "wifi station";

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            ESP_LOGE(TAG, "retry over max...");
        }
        ESP_LOGE(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        ESP_LOGI(TAG, "connect to the AP successful");
    }
}

void wifi_init_sta(void)
{
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    ESP_LOGI(TAG, "wifi_init_sta begin.");
    wifi_config_t wifi_config;
    nvs_handle_t my_handle;
    esp_err_t err;
    uint8_t flag = 0;
    uint32_t len = sizeof(wifi_config);
    memset(&wifi_config, 0, sizeof(wifi_config));

    err = nvs_open(NVS_CUSTOMER, NVS_READWRITE, &my_handle); //尝试打开外部flash 读取wifi配置信息
    if (err != ESP_OK)
    {
        nvs_close(my_handle);
        ESP_LOGE(TAG, "nvs_open failed.");
    }

    err = nvs_get_u8(my_handle, NVS_SMARTCONFIG_FLAG, &flag);
    if ((err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) || (flag != 1))
    {
        nvs_close(my_handle);
        if (flag != 1)
            ESP_LOGE(TAG, "smartconfig no flag.");
        else
            ESP_LOGE(TAG, "nvs_get_u8 failed.");
        smartconfig_main();
        return;
    }
    printf("Restart counter = %d\n", flag);

    err = nvs_get_blob(my_handle, NVS_SMARTCONFIG_DATA, &wifi_config, &len);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        nvs_close(my_handle);
        ESP_LOGE(TAG, "nvs_get_blob failed.");
        return;
    }

    err = nvs_get_blob(my_handle, NVS_SMARTCONFIG_DATA, &wifi_config, &len);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        nvs_close(my_handle);
        ESP_LOGE(TAG, "nvs_get_blob failed.");
        return;
    }

    nvs_close(my_handle);
    uint8_t ssid[33] = {0};
    uint8_t password[65] = {0};
    memcpy(ssid, wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid));
    memcpy(password, wifi_config.sta.password, sizeof(wifi_config.sta.password));

    ESP_LOGI(TAG, "SSID:%s", ssid);
    ESP_LOGI(TAG, "PASSWORD:%s", password);

    // uint8_t name[] = "nihsi";
    // memcpy(wifi_config.sta.ssid,name , sizeof(name));                //测试找不到WiFi会怎么样
    // memcpy(wifi_config.sta.password,name , sizeof(name));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

void wifi_station_init(void)
{
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
}
