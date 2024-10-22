#ifndef _EPD_H_
#define _EPD_H_
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
enum
{
    PIN_NUM_MOSI = GPIO_NUM_5,
    PIN_NUM_CLK  = GPIO_NUM_3,
    PIN_NUM_DC = GPIO_NUM_6,
    PIN_NUM_RST = GPIO_NUM_7,
};

#define delay_ms(x)  vTaskDelay(x / portTICK_PERIOD_MS)

#define EPD_W	152 
#define EPD_H	296

#define WHITE 0xFF
#define BLACK 0x00


void EPD_READBUSY(void);
void EPD_HW_RESET(void);
void EPD_Update(void);
void EPD_PartUpdate(void);
void EPD_FastUpdate(void);
void EPD_DeepSleep(void);
void EPD_Init(void);
void EPD_FastInit(void);
void EPD_Display_Clear(void);
void EPD_Clear_R26H(void);
void EPD_Display(const uint8_t *image);
#endif




