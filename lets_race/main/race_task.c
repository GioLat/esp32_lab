#include <stdint.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define ITERATIONS 100u
#define USE_MUTEX 
#define BUTTON_GPIO GPIO_NUM_0


static uint32_t counter = 0u;
static const uint8_t task_num_1 = 1u;
static const uint8_t task_num_2 = 2u;
static const char *TAG = "SYNC";

static gpio_config_t io_conf;

static SemaphoreHandle_t sMutex;
static SemaphoreHandle_t sBinary;


static void IRAM_ATTR button_isr_handler(void *arg)
{
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(sBinary, &woken);
    portYIELD_FROM_ISR(woken);
}

void init_b(void)
{
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;

    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);
}

static void button_task(void *arg)
{
    (void)arg;

    init_b();

    while (1)
    {
        if (xSemaphoreTake(sBinary, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "Button Pressed"); 
        }
    }
}

static void increase(void * arg)
{
    const uint8_t task_num = *(const uint8_t *)(arg);
    ESP_LOGI(TAG, "Task %u created", task_num);
    
    while(1)
    {
        #ifdef USE_MUTEX
        /* Try Take Res */
        if (xSemaphoreTake(sMutex, pdMS_TO_TICKS(100)) == pdTRUE) 
        {
            ESP_LOGI(TAG, "Task %u res acquired", task_num);
            for(uint32_t i = 0; i<ITERATIONS; i++)
            {
                //ESP_LOGI(TAG, "Task %u counter %u", task_num, ++counter);
                counter = (counter + 1u) % (50u * ITERATIONS);
            }
            ESP_LOGI(TAG, "Task %u counter %u", task_num, counter);
            /* Release Res */
            xSemaphoreGive(sMutex);
            ESP_LOGI(TAG, "Task %u res released", task_num);
        } 
        else
        {
            ESP_LOGW(TAG, "Task %u res timeout", task_num);
        }
        #else

            for(uint32_t i = 0; i<ITERATIONS; i++)
            {
                //ESP_LOGI(TAG, "Task %u counter %u", task_num, ++counter);
                counter = (counter + 1u) % (50u * ITERATIONS);
                ESP_LOGI(TAG, "Task %u counter %u", task_num, counter);
            }    

        #endif /* USE_MUTEX */


        /* Sleep */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void app_main(void)
{
    /* Create Mutex */
    sMutex = xSemaphoreCreateMutex();
    if (sMutex == NULL) 
    {
        ESP_LOGE(TAG, "Mutex creation FAILED");
        return;
    }

    /* Create Task 1 */
    if (xTaskCreatePinnedToCore(increase, "task_1", 2048,(void *)&task_num_1, 5, NULL, 0) != pdPASS) 
    {
        ESP_LOGE(TAG, "task_1 creation FAILED");
    }

    /* Create Task 2 */
    if (xTaskCreatePinnedToCore(increase, "task_1", 2048,(void *)&task_num_2, 10, NULL, 1) != pdPASS) 
    {
        ESP_LOGE(TAG, "task_2 creation FAILED");
    }

    /* Create Binary */
    sBinary = xSemaphoreCreateBinary();

    /* Create Button Task */
    if (xTaskCreatePinnedToCore(button_task, "task_1", 2048, NULL, 10, NULL, 1) != pdPASS) 
    {
        ESP_LOGE(TAG, "task_2 creation FAILED");
    }

}