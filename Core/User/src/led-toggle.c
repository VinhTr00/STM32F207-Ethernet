#include "cmsis_os.h"
#include "timers.h"
#include "led-toggle.h"
#include "stm32f207xx.h"

#define TCPECHO_THREAD_PRIO    (osPriority_t) osPriorityHigh

osThreadId_t ledtogglehandle;

extern osSemaphoreId_t xSemaphoreTCP;
extern osSemaphoreId_t xSemaphoreHTTP;

static void StartLedToggle(void * argument);

void led_toggle_init(void){
    const osThreadAttr_t ledTask = {
        .name = "ledTask",
        .stack_size = 256*4,
        .priority = TCPECHO_THREAD_PRIO,
    };
    ledtogglehandle = osThreadNew(StartLedToggle, NULL, &ledTask);
    (void)ledtogglehandle;
}

static void StartLedToggle(void * argument){
    while (1)
    {
        if (osSemaphoreAcquire(xSemaphoreTCP, pdMS_TO_TICKS(10)) == osOK){
            HAL_GPIO_WritePin(STM_LED1_GPIO_Port, STM_LED1_Pin, GPIO_PIN_SET);
            osDelay(50);
            HAL_GPIO_WritePin(STM_LED1_GPIO_Port, STM_LED1_Pin, GPIO_PIN_RESET);
        }
        else if (osSemaphoreAcquire(xSemaphoreHTTP, pdMS_TO_TICKS(10)) == osOK){
            HAL_GPIO_WritePin(STM_LED2_GPIO_Port, STM_LED2_Pin, GPIO_PIN_SET);
            osDelay(50);
            HAL_GPIO_WritePin(STM_LED2_GPIO_Port, STM_LED2_Pin, GPIO_PIN_RESET);
        }
    }
}

