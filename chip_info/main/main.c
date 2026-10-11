#include "esp_chip_info.h"
#include "esp_log.h"

static const char *TAG = "CHIPINFO";

void app_main(void)
{
    esp_chip_info_t info;
    esp_chip_info(&info);

    ESP_LOGI(TAG, "Numero di core: %d", info.cores);
    ESP_LOGI(TAG, "Revisione del chip: %lu", (unsigned long)info.revision);
    ESP_LOGI(TAG, "Wi-Fi disponibile: %s",
             (info.features & CHIP_FEATURE_WIFI_BGN) ? "si" : "no");
}