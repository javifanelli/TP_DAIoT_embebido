#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "../components/sntp_time.h"
#include "../components/mqtt_funcs.c"
#include "../components/bmp280/bmp280.h"
#include "../components/temp/temp.h"

bool time_sinc_ok = false;

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());
    
    initialize_sntp();
    while (!time_sinc_ok) vTaskDelay(100 * 1);
    obtain_time();
    

    mqtt_app_start();
    start_bmp();
    xTaskCreate(mqtt_send_info, "mqtt_send_info", 4096 * 8, NULL, 3, NULL);
    
}
