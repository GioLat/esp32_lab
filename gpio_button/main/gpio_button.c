#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h" 
#include "freertos/task.h"     
#include "driver/gpio.h"
#include "esp_log.h"      

#define BUTTON_GPIO GPIO_NUM_0

#define MODE_2  /*isr way*/

static gpio_config_t io_conf;


#ifdef MODE_1
static const char *TAG = "GPIO_DEMO";
static uint8_t prevState = 0;
#endif

#ifdef MODE_2
static void IRAM_ATTR button_isr_handler(void *arg)
{
// Lavoro minimo e non bloccante: qui NON si puo' chiamare
// ESP_LOGI, ne' altre funzioni non esplicitamente
// ISR-safe. Il modo corretto di delegare il lavoro vero
// a un task si vede nel Modulo 9.
}
#endif

void init_b(void);
void check_pression(void);

#if defined(MODE_1)

void init_b(void)
{
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO),
    io_conf.mode = GPIO_MODE_INPUT,
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE,
    io_conf.intr_type = GPIO_INTR_DISABLE,

    gpio_config(&io_conf);
}

void check_pression(void)
{
    if ( (prevState == 0) && (gpio_get_level(BUTTON_GPIO) == 0) ) 
    {
        ESP_LOGI(TAG, "Pulsante premuto");
        prevState = 1;
    }
    else if ( (prevState == 1) && (gpio_get_level(BUTTON_GPIO) == 1) )
    {
         ESP_LOGI(TAG, "Pulsante rilasciato");
        prevState = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
}

#elif defined(MODE_2)  /* MODE_2 */

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

void check_pression(void)
{
    /* pression is checked by isr */
    vTaskDelay(pdMS_TO_TICKS(1000));
}

#endif



void app_main(void)
{

    init_b();

    while (1) 
    {
        check_pression();
    }
}