#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

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

static void big_task(void *arg)
{
    while (1) 
    {
        ESP_LOGI(TAG, "[BIG] ring at %lld us", (long long)esp_timer_get_time());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


static void small_task(void *arg)
{
    while (1)
    {
        int64_t t0 = esp_timer_get_time();
        while (esp_timer_get_time() - t0 < 4000000)
         {
            /* ruuun */
        }
        ESP_LOGI(TAG, "[SMALL] task completed");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void app_main(void)
{

    TaskHandle_t task_handle = NULL;

    /* Task With Pinned Core */
    if (xTaskCreatePinnedToCore(led_task, "led_task", 2048, (void *)(&io_conf_led_task), 5, &task_handle, 1) != pdPASS) 
    {
        ESP_LOGE(TAG, "led_task creation FAILED");
    }

    /* Delete Task */
    vTaskDelete(task_handle);

    /* Normal Task */
    if (xTaskCreate(button_task, "button_task", 2048, (void *)(&io_conf_button_task), 5, NULL) != pdPASS) 
    {
        ESP_LOGE(TAG, "button_task creation FAILED");
    }

    /* Normal Task */
    if (xTaskCreate(big_task, "big_task", 2048, NULL, 7, NULL) != pdPASS) 
    {
        ESP_LOGE(TAG, "big_task creation FAILED");
    }

    /* Normal Task */
    if (xTaskCreate(small_task, "small_task", 2048, NULL, 3, NULL) != pdPASS) 
    {
        ESP_LOGE(TAG, "big_task creation FAILED");
    }

}