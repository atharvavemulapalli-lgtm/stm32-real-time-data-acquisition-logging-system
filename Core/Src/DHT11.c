#include "DHT11.h"

static void DHT11_SetPinOutput(DHT11_HandleTypeDef *hdht)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = hdht->GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(hdht->GPIOx, &GPIO_InitStruct);
}

static void DHT11_SetPinInput(DHT11_HandleTypeDef *hdht)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = hdht->GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(hdht->GPIOx, &GPIO_InitStruct);
}

Dht_State DHT11_Init(DHT11_HandleTypeDef *hdht,
                     TIM_HandleTypeDef *htim,
                     GPIO_TypeDef *GPIOx,
                     uint16_t GPIO_Pin)
{
    hdht->htim = htim;
    hdht->GPIOx = GPIOx;
    hdht->GPIO_Pin = GPIO_Pin;

    HAL_TIM_Base_Start(htim);

    return DHT_OK;
}

static Dht_State DHT11_WaitForPin(DHT11_HandleTypeDef *hdht,
                                  GPIO_PinState state,
                                  uint32_t timeout_us)
{
    __HAL_TIM_SET_COUNTER(hdht->htim, 0);
    while (HAL_GPIO_ReadPin(hdht->GPIOx, hdht->GPIO_Pin) != state)
    {
        if (__HAL_TIM_GET_COUNTER(hdht->htim) > timeout_us)
            return DHT_ERROR_TIMEOUT;
    }
    return DHT_OK;
}

Dht_State DHT11_Read(DHT11_HandleTypeDef *hdht,
                     uint8_t *temperature,
                     uint8_t *humidity)
{
    uint8_t data[40] = {0};

    // Start signal
    DHT11_SetPinOutput(hdht);
    HAL_GPIO_WritePin(hdht->GPIOx, hdht->GPIO_Pin, GPIO_PIN_RESET);
    HAL_Delay(18); // ≥18 ms
    HAL_GPIO_WritePin(hdht->GPIOx, hdht->GPIO_Pin, GPIO_PIN_SET);

    DHT11_SetPinInput(hdht);

    //  DHT11 RESPONSE
    if (DHT11_WaitForPin(hdht, GPIO_PIN_RESET, 150)) return DHT_ERROR_TIMEOUT;
    if (DHT11_WaitForPin(hdht, GPIO_PIN_SET,   150)) return DHT_ERROR_TIMEOUT;
    if (DHT11_WaitForPin(hdht, GPIO_PIN_RESET, 150)) return DHT_ERROR_TIMEOUT;

    // Reading 40 bits
    for (int i = 0; i < 40; i++)
    {
        if (DHT11_WaitForPin(hdht, GPIO_PIN_SET, 150)) return DHT_ERROR_TIMEOUT;

        __HAL_TIM_SET_COUNTER(hdht->htim, 0);
        if (DHT11_WaitForPin(hdht, GPIO_PIN_RESET, 150)) return DHT_ERROR_TIMEOUT;

        uint32_t duration = __HAL_TIM_GET_COUNTER(hdht->htim);

        int signal_state  = duration>40? 1 : 0;
        data[i] = signal_state;
    }

   //  Checksum
    uint8_t separated_data[5]={0};

    uint8_t index = 0;
    for(int i = 0;i<40;i++){
    	separated_data[index] = (separated_data[index] << 1) | data[i];
    	if ((i + 1) % 8 == 0) index++;
    }

    if ((uint8_t)(separated_data[0] + separated_data[1] + separated_data[2] + separated_data[3]) != separated_data[4])
        return DHT_ERROR_CHECKSUM;

    *humidity = separated_data[0];
    *temperature = separated_data[2];

    return DHT_OK;
}
