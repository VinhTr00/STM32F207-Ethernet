#include "cmsis_os.h"
#include "timers.h"
#include "led-toggle.h"
#include "stm32f207xx.h"
#include "queue.h"

#define TCPECHO_THREAD_PRIO    (osPriority_t) osPriorityHigh
#define ONE_SHOT_TIMER pdMS_TO_TICKS(50)

TimerHandle_t STM_LED1_Timer, STM_LED2_Timer;
osThreadId_t ledtogglehandle;
extern QueueHandle_t xQueue1, xQueue2;

void StartLedToggle(void * argument);
static void Led1TimerCallback(TimerHandle_t xTimer);
static void Led2TimerCallback(TimerHandle_t xTimer);

void led_toggle_init(void){
    const osThreadAttr_t ledTask = {
        .name = "ledTask",
        .stack_size = 256*4,
        .priority = TCPECHO_THREAD_PRIO,
    };
    ledtogglehandle = osThreadNew(StartLedToggle, NULL, &ledTask);
    STM_LED1_Timer = xTimerCreate("Led1Timer", ONE_SHOT_TIMER, pdFALSE, 0, Led1TimerCallback);
    STM_LED2_Timer = xTimerCreate("Led2Timer", ONE_SHOT_TIMER, pdFALSE, 0, Led2TimerCallback);
}

void StartLedToggle(void * argument){
    const TickType_t shortdelay = pdMS_TO_TICKS(10);
    int32_t queueReceived;
    QueueSetMemberHandle_t xHandle;
    while (1)
    {
        if (xQueueReceive(xQueue1, &queueReceived, pdMS_TO_TICKS(100)) == pdPASS){
            HAL_GPIO_WritePin(STM_LED1_GPIO_Port, STM_LED1_Pin, GPIO_PIN_SET);
            xTimerReset(STM_LED1_Timer, shortdelay);
        }
        else if (xQueueReceive(xQueue2, &queueReceived, pdMS_TO_TICKS(100)) == pdPASS){
            HAL_GPIO_WritePin(STM_LED2_GPIO_Port, STM_LED2_Pin, GPIO_PIN_SET);
            xTimerReset(STM_LED2_Timer, shortdelay);
        }
    }
}

static void Led1TimerCallback(TimerHandle_t xTimer){
    HAL_GPIO_WritePin(STM_LED1_GPIO_Port, STM_LED1_Pin, GPIO_PIN_RESET);
}

static void Led2TimerCallback(TimerHandle_t xTimer){
    HAL_GPIO_WritePin(STM_LED2_GPIO_Port, STM_LED2_Pin, GPIO_PIN_RESET);
}