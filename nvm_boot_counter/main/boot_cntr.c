#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"


static const char *TAG = "NVS_DEMO";
void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    nvs_handle_t handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &handle));
    int32_t avvii = 0;
    nvs_get_i32(handle, "avvii", &avvii);
    avvii++;
    ESP_LOGI(TAG, "Numero di avvii: %ld", (long)avvii);
    nvs_set_i32(handle, "avvii", avvii);
    nvs_commit(handle);
    nvs_close(handle);
}