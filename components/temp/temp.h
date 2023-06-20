bmp280_params_t params;
bmp280_t devt;


void start_bmp (void)
{
    bmp280_init_default_params(&params);
    memset(&devt, 0, sizeof(bmp280_t));
    ESP_ERROR_CHECK(bmp280_init_desc(&devt, BMP280_I2C_ADDRESS_0, 0, SDA_GPIO, SCL_GPIO));
    ESP_ERROR_CHECK(bmp280_init(&devt, &params));
}

void get_temp(void)
{
    bool bme280p = devt.id == BME280_CHIP_ID;
    ESP_LOGI(TAG, "BMP280: found %s\n", bme280p ? "BME280" : "BMP280");

    float pressure, temperature, humidity;
    if (bmp280_read_float(&devt, &temperature, &pressure, &humidity) != ESP_OK) {
        ESP_LOGI(TAG, "Temperature/pressure reading failed\n");
        }
    else {
        ESP_LOGI(TAG, "Pressure: %.2f hPa, Temperature: %.2f C", pressure/100, temperature);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
}