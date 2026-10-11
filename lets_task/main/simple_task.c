#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

//#define GET_STACK

#define LED_GPIO    GPIO_NUM_2
#define BUTTON_GPIO GPIO_NUM_0


static const char *TAG = "TASKS";

static const gpio_config_t io_conf_led_task = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
};

static const gpio_config_t io_conf_button_task = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
};

static void led_task(void *arg)
{

    const gpio_config_t *io_conf = (const gpio_config_t *)arg;
    gpio_config(io_conf);

    int pin = __builtin_ctzll(io_conf->pin_bit_mask);
    uint8_t state = 0;
    while(1)
    {
        state = !state;
        gpio_set_level(pin, state);

#ifdef GET_STACK
        UBaseType_t libero = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG, "Stack minimo libero: %u byte", (unsigned)libero);
#endif

        vTaskDelay(pdMS_TO_TICKS(500));
    }

    /* autodelete unreachable */
    vTaskDelete(NULL);
}


static void button_task(void *arg)
{

    const gpio_config_t *io_conf = (const gpio_config_t *)arg;
    gpio_config(io_conf);

    int pin = __builtin_ctzll(io_conf->pin_bit_mask);
    uint8_t prevState = 0;

    while (1)
    {
        if ( (prevState == 0) && (gpio_get_level(pin) == 0) ) 
        {
            ESP_LOGI(TAG, "Pulsante premuto");
            prevState = 1;
        }
        else if ( (prevState == 1) && (gpio_get_level(pin) == 1) )
        {
            ESP_LOGI(TAG, "Pulsante rilasciato");
            prevState = 0;
        }

#ifdef GET_STACK
UBaseType_t libero = uxTaskGetStackHighWaterMark(NULL);
ESP_LOGI(TAG, "Stack minimo libero: %u byte", (unsigned)libero);
#endif

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


void app_main(void)
{

    TaskHandle_t task_handle = NULL;

    /* Task With Pinned Core */
    if (xTaskCreatePinnedToCore(led_task, "led_task", 2048, (void *)(&io_conf_led_task), 5, &task_handle, 1) != pdPASS) 
    {
        ESP_LOGE(TAG, "Creazione di led_task fallita");
    }

    /* Normal Task */
    if (xTaskCreate(button_task, "button_task", 2048, (void *)(&io_conf_button_task), 5, NULL) != pdPASS) 
    {
        ESP_LOGE(TAG, "Creazione di button_task fallita");
    }

    /* Delete Task */
    vTaskDelete(task_handle);

}