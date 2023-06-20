#include "sntp_time.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_sntp.h"

static const char *TAG = "SNTP Module";

RTC_DATA_ATTR static int boot_count = 0;
extern bool time_sinc_ok;
char formatted_time[20];

#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_CUSTOM
void sntp_sync_time(struct timeval *tv)
{
   settimeofday(tv, NULL);
   ESP_LOGI(TAG, "Time is synchronized from custom code");
   sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}
#endif

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
    time_sinc_ok = true;
}

void obtain_time(void)
{
    ESP_LOGI(TAG, "Obtaining time...");
    time_t now = 0;
    struct tm timeinfo = { 0 };
    // Obtiene la hora actual
    time(&now);
    localtime_r(&now, &timeinfo);
    // Formatea la hora en el formato deseado (YYYY-MM-DD HH:MM:SS)
    strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "Obtained time: %s", formatted_time);
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
    // Espera hasta que se complete la sincronización de tiempo
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "Time synchronized");
}
