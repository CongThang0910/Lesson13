#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

/* ====== Prototype ====== */
void UART1_Init(void);
void UART1_SendChar(char c);
void UART1_SendString(const char *str);
void Task1_Print(void *pvParameters);
void Task2_Print(void *pvParameters);

/* ====== Main ====== */
int main(void)
{
    SystemInit();
    UART1_Init();

    xTaskCreate(Task1_Print, "Task1", 128, NULL, 1, NULL);
    xTaskCreate(Task2_Print, "Task2", 128, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
}

/* ====== UART1 Init ====== */
void UART1_Init(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef uart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // PA9 -> TX (Alternate function push-pull)
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // PA10 -> RX (input floating)
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    uart.USART_BaudRate = 115200;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &uart);

    USART_Cmd(USART1, ENABLE);
}

/* ====== UART send ====== */
void UART1_SendChar(char c)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, c);
}

void UART1_SendString(const char *str)
{
    while (*str)
    {
        UART1_SendChar(*str++);
    }
}

/* ====== TASK 1 ====== */
void Task1_Print(void *pvParameters)
{
    for (;;)
    {
        UART1_SendString(">>> Task 1 is sending data...\r\n");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* ====== TASK 2 ====== */
void Task2_Print(void *pvParameters)
{
    for (;;)
    {
        UART1_SendString("<<< Task 2 is writing something else...\r\n");
        vTaskDelay(pdMS_TO_TICKS(120));
    }
}