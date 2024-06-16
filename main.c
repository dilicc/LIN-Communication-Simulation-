#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include "unicom.h"
#include "seg7mux.h"
#include "led_bar.h"

// Deklaracija taskova
void vBatteryReceiveTask(void *pvParameters);
void vPCTxTask(void *pvParameters);
void vPCRxTask(void *pvParameters);
void vDataProcessingTask(void *pvParameters);
void vDisplayTask(void *pvParameters);

// Kreiranje queue i semafora
QueueHandle_t xDataQueue;
SemaphoreHandle_t xSyncSemaphore;

int main(void)
{
    // Inicijalizacija hardverskih simulatora
    UniCom_Init();
    Seg7Mux_Init();
    LEDBar_Init();

    // Kreiranje queue i semafora
    xDataQueue = xQueueCreate(10, sizeof(int));
    xSyncSemaphore = xSemaphoreCreateBinary();

    // Kreiranje taskova
    xTaskCreate(vBatteryReceiveTask, "BatteryReceive", 128, NULL, 1, NULL);
    xTaskCreate(vPCTxTask, "PCTx", 128, NULL, 1, NULL);
    xTaskCreate(vPCRxTask, "PCRx", 128, NULL, 1, NULL);
    xTaskCreate(vDataProcessingTask, "DataProcessing", 128, NULL, 1, NULL);
    xTaskCreate(vDisplayTask, "Display", 128, NULL, 1, NULL);

    // Start scheduler
    vTaskStartScheduler();

    for (;;);
}

void vBatteryReceiveTask(void *pvParameters)
{
    // Task kod za prijem podataka od akumulatora
    // Implementacija UniCom za kanal 0
    int receivedData;
    for (;;)
    {
        // Simulacija prijema podataka sa UniCom kanala 0
        receivedData = UniCom_Receive(0);
        xQueueSend(xDataQueue, &receivedData, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vPCTxTask(void *pvParameters)
{
    // Task kod za slanje podataka na PC
    int dataToSend;
    for (;;)
    {
        if (xQueueReceive(xDataQueue, &dataToSend, portMAX_DELAY) == pdPASS)
        {
            UniCom_Send(1, dataToSend);
        }
    }
}

void vPCRxTask(void *pvParameters)
{
    // Task kod za prijem podataka od PC-a
    int receivedData;
    for (;;)
    {
        receivedData = UniCom_Receive(1);
        xQueueSend(xDataQueue, &receivedData, portMAX_DELAY);
    }
}

void vDataProcessingTask(void *pvParameters)
{
    // Task kod za obradu podataka
    int receivedData;
    for (;;)
    {
        if (xQueueReceive(xDataQueue, &receivedData, portMAX_DELAY) == pdPASS)
        {
            // Obrada podataka
            int processedData = receivedData * 2; // Primer obrade podataka
            xQueueSend(xDataQueue, &processedData, portMAX_DELAY);
        }
    }
}

void vDisplayTask(void *pvParameters)
{
    // Task kod za prikaz podataka na displeju
    int dataToDisplay;
    for (;;)
    {
        if (xQueueReceive(xDataQueue, &dataToDisplay, portMAX_DELAY) == pdPASS)
        {
            Seg7Mux_Display(dataToDisplay);
        }
    }
}
