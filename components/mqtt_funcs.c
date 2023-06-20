extern const uint8_t client_cert_pem_start[] asm("_binary_client_pem_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_pem_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_pem_end");

#include "sntp_time.h"
#define BROKER_URI "mqtts://192.168.0.70"
#define seconds 30 // Segundos de delay en actualizar temperatura

static const char *ID ="1";
static char MAC[18];
static int8_t rssi = 0;
static char buffer_mqtt[150];
static const char *TAG = "sensor";
static esp_mqtt_client_handle_t client;
static char TOPIC[50];
bool mqtt_client_connected = false;

void build_topic(void) {
    sprintf(TOPIC, "/home/temperatura/data");
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    
    build_topic();
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    /* esp_mqtt_client_handle_t  */client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        mqtt_client_connected = true;
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, TOPIC, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        /* buffer_in = event->data; */
        break;
    
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    uint8_t mac[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    sprintf(MAC, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = BROKER_URI,
        .client_cert_pem = (const char *)client_cert_pem_start,
        .client_key_pem = (const char *)client_key_pem_start,
        .cert_pem = (const char *)server_cert_pem_start,
    };
    
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void mqtt_send_info(void *pvParameter)
{
    wifi_ap_record_t ap_info;
    buffer_mqtt[0] = 0;
    char RSSI_CHAR[10];
    RSSI_CHAR[0] = 0;
    esp_wifi_sta_get_ap_info(&ap_info);
    int secs = 0;
    char ch_secs[4];
    int temperatura = 20;  // Temperatura inicial
    int incremento = 1;  // Incremento de temperatura por ciclo

    while (1) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        settimeofday(&tv, NULL);
        struct tm *timeinfo;
        time_t now = tv.tv_sec;
        timeinfo = localtime(&now);
        strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", timeinfo);
        memset(buffer_mqtt, 0, sizeof(buffer_mqtt));
        sprintf(ch_secs, "%02d", secs);
        rssi = ap_info.rssi;
        sprintf(RSSI_CHAR, "%d", rssi);
        ESP_LOGI(TAG, "RSSI: %d ", ap_info.rssi);
        strcat(buffer_mqtt, "{\n\"ID\": ");
        strcat(buffer_mqtt, ID);
        strcat(buffer_mqtt, ",\n");
        strcat(buffer_mqtt, "\"RSSI\": ");
        strcat(buffer_mqtt, RSSI_CHAR);
        strcat(buffer_mqtt, ",\n");
        strcat(buffer_mqtt, "\"time\": \"");
        strcat(buffer_mqtt, formatted_time);
        strcat(buffer_mqtt, "\",\n");
        strcat(buffer_mqtt, "\"valor\": ");
        char temperatura_str[10];
        sprintf(temperatura_str, "%d", temperatura);
        strcat(buffer_mqtt, temperatura_str);
        strcat(buffer_mqtt, ",\n");
        strcat(buffer_mqtt, "\"MAC\": \"");
        strcat(buffer_mqtt, MAC);
        strcat(buffer_mqtt, "\"\n}");
        esp_mqtt_client_publish(client, TOPIC, buffer_mqtt, 0, 0, 0);
        vTaskDelay(1000 * seconds / portTICK_PERIOD_MS);
        // Incrementar temperatura
        temperatura += incremento;
        // Verificar límites de temperatura
        if (temperatura >= 30) {
            incremento = -1;  // Cambiar dirección de incremento
        } else if (temperatura < 20) {
            incremento = 1;  // Cambiar dirección de incremento
        }
    }
    vTaskDelete(NULL);
}