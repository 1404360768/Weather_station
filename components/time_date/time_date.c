#include "time_date.h"
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "lcd_ssd_1351.h"

//http组包宏，获取天气的http接口参数
#define WEB_SERVER "api.seniverse.com"
#define WEB_PORT "80"
#define WEB_URL "/v3/weather/now.json?key="
#define APIKEY "g3egns3yk2ahzb0p"
#define city "huanggang"
#define language "en"

//http请求包
static const char *REQUEST = "GET " WEB_URL "" APIKEY "&location=" city "&language=" language " HTTP/1.1\r\n"
                             "Host: " WEB_SERVER "\r\n"
                             "Connection: close\r\n"
                             "\r\n";

static const char *TAG = "time_data";

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGE(TAG, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

static void esp_wait_sntp_sync(void)
{
    char strftime_buf[64];
    time_t now = 0;
    struct tm timeinfo = {0};
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2021 - 1900))
    {
        ESP_LOGI(TAG, "Time is not set yet");
        // update 'now' variable with current time
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    // set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
    SSD1351_WriteString(0, 10, strftime_buf, Font_7x10, SSD1351_RED, SSD1351_BLACK);
}

static void time_date_task(void *parm)
{
    int8_t request_del = 30;
    struct addrinfo *res;
    struct in_addr *addr;
    int sock = 0;
    int r = 0;
    char recv_buf[1024];
    char mid_buf[1024];
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    time_t now = 0;
    struct tm timeinfo = {0};
    while (1)
    {
        esp_wait_sntp_sync();
        time(&now);
        localtime_r(&now, &timeinfo);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        if (timeinfo.tm_sec != request_del)
        {
            ESP_LOGI(TAG, "sec=%d\r\n", timeinfo.tm_sec);
            continue;
        }
        ESP_LOGE(TAG, "sec=%d\r\n", timeinfo.tm_sec);
        request_del = (timeinfo.tm_sec + request_del) % 60;
        //DNS域名解析
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);
        if (err != 0 || res == NULL)
        {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p\r\n", err, res);
                // ESP_LOGE(TAG, "DNS lookup failed errno: %d", errno);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        //打印获取的IP
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s\r\n", inet_ntoa(*addr));

        //新建socket
        sock = socket(res->ai_family, res->ai_socktype, IPPROTO_IP);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            close(sock);
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        //连接ip
        if (connect(sock, res->ai_addr, res->ai_addrlen) != 0)
        {
            ESP_LOGE(TAG, "socket connect failed: errno=%d\r\n", errno);
            close(sock);
            freeaddrinfo(res);
            continue;
        }
        //发送http包
        if (write(sock, REQUEST, strlen(REQUEST)) < 0)
        {
            ESP_LOGE(TAG, "socket send failed: errno=%d\r\n", errno);
            close(sock);
            continue;
        }
        //清缓存
        memset(mid_buf, 0, sizeof(mid_buf));
        //获取http应答包
        do
        {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(sock, recv_buf, sizeof(recv_buf) - 1);
            strcat(mid_buf, recv_buf);
        } while (r > 0);
        ESP_LOGI(TAG, "%s", mid_buf);

        if (sock != -1)
        {
            close(sock);
            shutdown(sock, 0);
        }

        taskYIELD(); //主动要求切换任务
    }
}

void time_date_init(void)
{
    ESP_LOGI(TAG, "time_data start up");
    initialize_sntp();
    xTaskCreate(time_date_task, "time_date_task", 8192, NULL, 5, NULL);
}
