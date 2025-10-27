#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* ================= Khai báo task & semaphore ================= */
TaskHandle_t handleBlink = NULL;
TaskHandle_t handleAlert = NULL;
SemaphoreHandle_t semButton = NULL;

/* ================= Nguyên m?u hàm ================= */
void Task_Blink(void *pvParameters);
void Task_Alert(void *pvParameters);
void GPIO_Config(void);
void EXTI_Config(void);

/* ==================== MAIN ==================== */
int main(void)
{
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // FreeRTOS yêu cau

    GPIO_Config();
    EXTI_Config();

    semButton = xSemaphoreCreateBinary(); // semaphore báo nhan nút

    xTaskCreate(Task_Blink, "Blink", 128, NULL, 1, &handleBlink);
    xTaskCreate(Task_Alert, "Alert", 128, NULL, 2, &handleAlert);

    vTaskStartScheduler();

    while (1);
}

/* ================= Cau hình GPIO ================= */
void GPIO_Config(void)
{
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOC, ENABLE);

    // LED nhap nháy (PC13)
    gpio.GPIO_Pin = GPIO_Pin_13;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &gpio);
    GPIO_SetBits(GPIOC, GPIO_Pin_13); // T?t LED (active low)

    // LED c?nh báo (PB12)
    gpio.GPIO_Pin = GPIO_Pin_12;
    GPIO_Init(GPIOB, &gpio);
    GPIO_SetBits(GPIOB, GPIO_Pin_12); // T?t LED c?nh báo (active low)

    // Nút nh?n (PA0 - kéo lên trong, nh?n = 0)
    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &gpio);
}

/* ================= C?u hình EXTI0 ================= */
void EXTI_Config(void)
{
    EXTI_InitTypeDef exti;
    NVIC_InitTypeDef nvic;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    exti.EXTI_Line = EXTI_Line0;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling; // Nút nh?n xu?ng = ng?t
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);

    EXTI_ClearITPendingBit(EXTI_Line0);

    nvic.NVIC_IRQChannel = EXTI0_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 10;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}


void Task_Blink(void *pvParameters)
{
    while (1)
    {
        GPIOC->ODR ^= GPIO_Pin_13;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void Task_Alert(void *pvParameters)
{
    for (;;)
    {
       
        if (xSemaphoreTake(semButton, portMAX_DELAY) == pdTRUE)
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_12); 
            vTaskDelay(pdMS_TO_TICKS(1000));
					GPIO_SetBits(GPIOB, GPIO_Pin_12);   
        }
    }
}


void EXTI0_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {EXTI_ClearITPendingBit(EXTI_Line0);

        if (semButton != NULL)
            xSemaphoreGiveFromISR(semButton, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}