#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"

#define ITERATIONS 100u
#define USE_MUTEX 

static uint32_t counter = 0u;
static const uint8_t task_num_1 = 1u;
static const uint8_t task_num_2 = 2u;
static const char *TAG = "SYNC";

static SemaphoreHandle_t mutex;


static void increase(void * arg)
{
    const uint8_t task_num = *(const uint8_t *)(arg);
    ESP_LOGI(TAG, "Task %u created", task_num);
    
    while(1)
    {
        #ifdef USE_MUTEX
        /* Try Take Res */
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) 
        {
            ESP_LOGI(TAG, "Task %u res acquired", task_num);
            for(uint32_t i = 0; i<ITERATIONS; i++)
            {
                //ESP_LOGI(TAG, "Task %u counter %u", task_num, ++counter);
                counter = (counter + 1u) % (50u * ITERATIONS);
            }
            ESP_LOGI(TAG, "Task %u counter %u", task_num, counter);
            /* Release Res */
            xSemaphoreGive(mutex);
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
    mutex = xSemaphoreCreateMutex();
    if (mutex == NULL) 
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

}