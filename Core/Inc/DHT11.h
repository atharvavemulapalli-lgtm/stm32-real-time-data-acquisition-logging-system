#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f4xx_hal.h"

typedef enum {
    DHT_OK = 0,
    DHT_ERROR_TIMEOUT,
    DHT_ERROR_CHECKSUM
} Dht_State;

typedef struct {
    TIM_HandleTypeDef *htim;
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
} DHT11_HandleTypeDef;


Dht_State DHT11_Init(DHT11_HandleTypeDef *hdht,
                     TIM_HandleTypeDef *htim,
                     GPIO_TypeDef *GPIOx,
                     uint16_t GPIO_Pin);

Dht_State DHT11_Read(DHT11_HandleTypeDef *hdht,
                     uint8_t *temperature,
                     uint8_t *humidity);

#endif
