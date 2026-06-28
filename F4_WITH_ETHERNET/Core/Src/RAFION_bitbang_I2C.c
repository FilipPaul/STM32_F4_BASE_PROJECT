#include "RAFION_bitbang_I2C.h"
#include "globals.h"

#define RAFION_BITBANG_OK        0
#define RAFION_BITBANG_ERROR    -1
#define RAFION_BITBANG_MIN_HZ    1000U
#define RAFION_BITBANG_DEFAULT_CLOCK_STRETCH_MS 10U

static uint32_t RafionBitbangGetPeriodUs(const RafionBitbangHandle* handle)
{
    uint32_t clock_speed = handle->clock_speed;

    if (clock_speed < RAFION_BITBANG_MIN_HZ)
    {
        clock_speed = 100000U;
    }

    if (clock_speed >= 1000000U)
    {
        return 1U;
    }

    return (1000000U + clock_speed - 1U) / clock_speed;
}

static uint32_t RafionBitbangGetLowPhaseUs(const RafionBitbangHandle* handle)
{
    uint32_t period_us = RafionBitbangGetPeriodUs(handle);

    if (period_us <= 1U)
    {
        return 1U;
    }

    return period_us / 2U;
}

static uint32_t RafionBitbangGetHighPhaseUs(const RafionBitbangHandle* handle)
{
    uint32_t period_us = RafionBitbangGetPeriodUs(handle);
    uint32_t low_phase_us = RafionBitbangGetLowPhaseUs(handle);

    if (period_us <= low_phase_us)
    {
        return 1U;
    }

    return period_us - low_phase_us;
}

static uint32_t RafionBitbangGetStretchTimeoutMs(const RafionBitbangHandle* handle)
{
    if (handle->clock_stretch_timeout_ms == 0U)
    {
        return RAFION_BITBANG_DEFAULT_CLOCK_STRETCH_MS;
    }

    return handle->clock_stretch_timeout_ms;
}

static void RafionBitbangDelayUs(uint32_t delay_us)
{
    if (delay_us == 0U)
    {
        return;
    }

    uint32_t start = TIM_7_tick_1_us;
    while ((uint32_t)(TIM_7_tick_1_us - start) < delay_us)
    {
        //DO NOP TO PREVENT OPTIMIZATION OF THE LOOP
        __NOP();
    }
}

static void RafionBitbangConfigurePin(GPIO_TypeDef* port, uint16_t pin)
{
    GPIO_InitTypeDef gpio_init = {0};

    gpio_init.Pin = pin;
    gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(port, &gpio_init);
}

static void RafionBitbangReleaseSda(RafionBitbangHandle* handle)
{
    handle->sda_port->BSRR = handle->sda_pin;
}

static void RafionBitbangPullSdaLow(RafionBitbangHandle* handle)
{
    handle->sda_port->BSRR = ((uint32_t)handle->sda_pin << 16U);
}

static void RafionBitbangReleaseScl(RafionBitbangHandle* handle)
{
    handle->scl_port->BSRR = handle->scl_pin;
}

static void RafionBitbangPullSclLow(RafionBitbangHandle* handle)
{
    handle->scl_port->BSRR = ((uint32_t)handle->scl_pin << 16U);
}

static GPIO_PinState RafionBitbangReadSda(const RafionBitbangHandle* handle)
{
    return ((handle->sda_port->IDR & handle->sda_pin) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

static GPIO_PinState RafionBitbangReadScl(const RafionBitbangHandle* handle)
{
    return ((handle->scl_port->IDR & handle->scl_pin) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

static HAL_StatusTypeDef RafionBitbangWaitForSclHigh(RafionBitbangHandle* handle, uint32_t timeout_ms)
{
    uint32_t tickstart_us = TIM_7_tick_1_us;
    uint32_t timeout_us = timeout_ms * 1000U;

    while (RafionBitbangReadScl(handle) != GPIO_PIN_SET)
    {
        if ((timeout_ms != HAL_MAX_DELAY) && ((uint32_t)(TIM_7_tick_1_us - tickstart_us) >= timeout_us))
        {
            handle->last_error = HAL_I2C_ERROR_TIMEOUT;
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

static HAL_StatusTypeDef RafionBitbangRecoverBus(RafionBitbangHandle* handle)
{
    RafionBitbangReleaseSda(handle);
    RafionBitbangReleaseScl(handle);
    RafionBitbangDelayUs(RafionBitbangGetPeriodUs(handle));

    if (RafionBitbangReadSda(handle) == GPIO_PIN_SET)
    {
        return HAL_OK;
    }

    for (uint32_t pulse = 0U; pulse < 9U; pulse++)
    {
        if (RafionBitbangDoClockPulse(handle) != RAFION_BITBANG_OK)
        {
            return HAL_ERROR;
        }

        if (RafionBitbangReadSda(handle) == GPIO_PIN_SET)
        {
            break;
        }
    }

    if (RafionBitbangDoStopCondition(handle) != RAFION_BITBANG_OK)
    {
        return HAL_ERROR;
    }

    RafionBitbangReleaseSda(handle);
    RafionBitbangReleaseScl(handle);
    RafionBitbangDelayUs(RafionBitbangGetPeriodUs(handle));

    if ((RafionBitbangReadSda(handle) != GPIO_PIN_SET) || (RafionBitbangReadScl(handle) != GPIO_PIN_SET))
    {
        handle->last_error = HAL_I2C_ERROR_BERR;
        return HAL_ERROR;
    }

    return HAL_OK;
}

static HAL_StatusTypeDef RafionBitbangPrepareBus(RafionBitbangHandle* handle)
{
    if (RafionBitbangI2C_Init(handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (RafionBitbangRecoverBus(handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    RafionBitbangReleaseSda(handle);
    RafionBitbangReleaseScl(handle);
    RafionBitbangDelayUs(RafionBitbangGetPeriodUs(handle));

    if (RafionBitbangWaitForSclHigh(handle, RafionBitbangGetStretchTimeoutMs(handle)) != HAL_OK)
    {
        return HAL_TIMEOUT;
    }

    return HAL_OK;
}

static HAL_StatusTypeDef RafionBitbangWriteByte(RafionBitbangHandle* handle, uint8_t byte, bool* ack_received)
{
    bool nack = false;

    for (uint32_t bit = 0U; bit < 8U; bit++)
    {
        if (RafionBitbangWriteBit(handle, (byte & 0x80U) != 0U) != RAFION_BITBANG_OK)
        {
            return HAL_ERROR;
        }
        byte <<= 1U;
    }

    if (RafionBitbangReadBit(handle, &nack) != RAFION_BITBANG_OK)
    {
        return HAL_ERROR;
    }

    *ack_received = (nack == false);
    if (*ack_received == false)
    {
        handle->last_error = HAL_I2C_ERROR_AF;
    }

    return HAL_OK;
}

static HAL_StatusTypeDef RafionBitbangReadByte(RafionBitbangHandle* handle, uint8_t* byte, bool send_ack)
{
    bool bit_value = false;
    uint8_t result = 0U;

    for (uint32_t bit = 0U; bit < 8U; bit++)
    {
        result <<= 1U;
        if (RafionBitbangReadBit(handle, &bit_value) != RAFION_BITBANG_OK)
        {
            return HAL_ERROR;
        }
        if (bit_value)
        {
            result |= 1U;
        }
    }

    if (RafionBitbangWriteBit(handle, (send_ack == false)) != RAFION_BITBANG_OK)
    {
        return HAL_ERROR;
    }

    *byte = result;
    return HAL_OK;
}

HAL_StatusTypeDef RafionBitbangI2C_Init(RafionBitbangHandle* handle)
{
    if ((handle == NULL) || (handle->sda_port == NULL) || (handle->scl_port == NULL))
    {
        return HAL_ERROR;
    }

    RafionBitbangConfigurePin(handle->sda_port, handle->sda_pin);
    RafionBitbangConfigurePin(handle->scl_port, handle->scl_pin);
    RafionBitbangReleaseSda(handle);
    RafionBitbangReleaseScl(handle);
    handle->last_error = HAL_I2C_ERROR_NONE;

    return HAL_OK;
}

HAL_StatusTypeDef RafionBitbangI2C_MASTER_Transmit(RafionBitbangHandle* handle, uint16_t dev_address, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    bool ack_received = false;

    if ((handle == NULL) || ((data == NULL) && (size > 0U)))
    {
        return HAL_ERROR;
    }

    handle->last_error = HAL_I2C_ERROR_NONE;

    if (RafionBitbangPrepareBus(handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (RafionBitbangDoStartCondition(handle) != RAFION_BITBANG_OK)
    {
        return HAL_ERROR;
    }

    if (RafionBitbangWriteByte(handle, (uint8_t)(dev_address & 0xFEU), &ack_received) != HAL_OK)
    {
        RafionBitbangDoStopCondition(handle);
        return HAL_ERROR;
    }

    if (!ack_received)
    {
        RafionBitbangDoStopCondition(handle);
        return HAL_ERROR;
    }

    for (uint16_t index = 0U; index < size; index++)
    {
        if (RafionBitbangWriteByte(handle, data[index], &ack_received) != HAL_OK)
        {
            RafionBitbangDoStopCondition(handle);
            return HAL_ERROR;
        }

        if (!ack_received)
        {
            RafionBitbangDoStopCondition(handle);
            return HAL_ERROR;
        }

        if ((timeout != HAL_MAX_DELAY) && (timeout != 0U))
        {
            // Timeout is consumed only by clock stretching; keep the API shape aligned with HAL.
        }
    }

    if (RafionBitbangDoStopCondition(handle) != RAFION_BITBANG_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef RafionBitbangI2C_MASTER_Receive(RafionBitbangHandle* handle, uint16_t dev_address, uint8_t* data, uint16_t size, uint32_t timeout)
{
    bool ack_received = false;

    if ((handle == NULL) || ((data == NULL) && (size > 0U)))
    {
        return HAL_ERROR;
    }

    handle->last_error = HAL_I2C_ERROR_NONE;

    if (RafionBitbangPrepareBus(handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (RafionBitbangDoStartCondition(handle) != RAFION_BITBANG_OK)
    {
        return HAL_ERROR;
    }

    if (RafionBitbangWriteByte(handle, (uint8_t)(dev_address | 0x01U), &ack_received) != HAL_OK)
    {
        RafionBitbangDoStopCondition(handle);
        return HAL_ERROR;
    }

    if (!ack_received)
    {
        RafionBitbangDoStopCondition(handle);
        return HAL_ERROR;
    }

    for (uint16_t index = 0U; index < size; index++)
    {
        if (RafionBitbangReadByte(handle, &data[index], index < (size - 1U)) != HAL_OK)
        {
            RafionBitbangDoStopCondition(handle);
            return HAL_ERROR;
        }

        if ((timeout != HAL_MAX_DELAY) && (timeout != 0U))
        {
            // Timeout is consumed only by clock stretching; keep the API shape aligned with HAL.
        }
    }

    if (RafionBitbangDoStopCondition(handle) != RAFION_BITBANG_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef RafionBitbangI2C_MASTER_IsDeviceReady(RafionBitbangHandle* handle, uint16_t dev_address, uint32_t trials, uint32_t timeout)
{
    bool ack_received = false;

    if (handle == NULL)
    {
        return HAL_ERROR;
    }

    handle->last_error = HAL_I2C_ERROR_AF;

    for (uint32_t trial = 0U; trial < trials; trial++)
    {
        if (RafionBitbangPrepareBus(handle) != HAL_OK)
        {
            return HAL_ERROR;
        }

        if (RafionBitbangDoStartCondition(handle) != RAFION_BITBANG_OK)
        {
            return HAL_ERROR;
        }

        if (RafionBitbangWriteByte(handle, (uint8_t)(dev_address & 0xFEU), &ack_received) != HAL_OK)
        {
            RafionBitbangDoStopCondition(handle);
        }
        else
        {
            RafionBitbangDoStopCondition(handle);
            if (ack_received)
            {
                handle->last_error = HAL_I2C_ERROR_NONE;
                return HAL_OK;
            }
        }

        if ((timeout != HAL_MAX_DELAY) && (timeout > 0U))
        {
            RafionBitbangDelayUs(1000U);
        }
    }

    return HAL_ERROR;
}

int32_t RafionBitbangDoStartCondition(RafionBitbangHandle* handle)
{
    uint32_t low_phase_us = RafionBitbangGetLowPhaseUs(handle);
    uint32_t high_phase_us = RafionBitbangGetHighPhaseUs(handle);

    RafionBitbangReleaseSda(handle);
    RafionBitbangReleaseScl(handle);
    RafionBitbangDelayUs(high_phase_us);

    if (RafionBitbangWaitForSclHigh(handle, RafionBitbangGetStretchTimeoutMs(handle)) != HAL_OK)
    {
        return RAFION_BITBANG_ERROR;
    }

    RafionBitbangPullSdaLow(handle);
    RafionBitbangDelayUs(high_phase_us);
    RafionBitbangPullSclLow(handle);
    RafionBitbangDelayUs(low_phase_us);

    return RAFION_BITBANG_OK;
}

int32_t RafionBitbangDoStopCondition(RafionBitbangHandle* handle)
{
    uint32_t low_phase_us = RafionBitbangGetLowPhaseUs(handle);
    uint32_t high_phase_us = RafionBitbangGetHighPhaseUs(handle);

    RafionBitbangPullSdaLow(handle);
    RafionBitbangDelayUs(low_phase_us);
    RafionBitbangReleaseScl(handle);
    if (RafionBitbangWaitForSclHigh(handle, RafionBitbangGetStretchTimeoutMs(handle)) != HAL_OK)
    {
        return RAFION_BITBANG_ERROR;
    }
    RafionBitbangDelayUs(high_phase_us);
    RafionBitbangReleaseSda(handle);
    RafionBitbangDelayUs(high_phase_us);

    return RAFION_BITBANG_OK;
}

int32_t RafionBitbangDoClockPulse(RafionBitbangHandle* handle)
{
    uint32_t low_phase_us = RafionBitbangGetLowPhaseUs(handle);
    uint32_t high_phase_us = RafionBitbangGetHighPhaseUs(handle);

    RafionBitbangPullSclLow(handle);
    RafionBitbangDelayUs(low_phase_us);
    RafionBitbangReleaseScl(handle);
    if (RafionBitbangWaitForSclHigh(handle, RafionBitbangGetStretchTimeoutMs(handle)) != HAL_OK)
    {
        return RAFION_BITBANG_ERROR;
    }
    RafionBitbangDelayUs(high_phase_us);
    RafionBitbangPullSclLow(handle);

    return RAFION_BITBANG_OK;
}

int32_t RafionBitbangWriteBit(RafionBitbangHandle* handle, bool bit_value)
{
    uint32_t low_phase_us = RafionBitbangGetLowPhaseUs(handle);
    uint32_t high_phase_us = RafionBitbangGetHighPhaseUs(handle);

    RafionBitbangPullSclLow(handle);
    if (bit_value)
    {
        RafionBitbangReleaseSda(handle);
    }
    else
    {
        RafionBitbangPullSdaLow(handle);
    }

    RafionBitbangDelayUs(low_phase_us);
    RafionBitbangReleaseScl(handle);

    if (RafionBitbangWaitForSclHigh(handle, RafionBitbangGetStretchTimeoutMs(handle)) != HAL_OK)
    {
        return RAFION_BITBANG_ERROR;
    }

    RafionBitbangDelayUs(high_phase_us);
    RafionBitbangPullSclLow(handle);

    return RAFION_BITBANG_OK;
}

int32_t RafionBitbangReadBit(RafionBitbangHandle* handle, bool* bit_value)
{
    uint32_t low_phase_us = RafionBitbangGetLowPhaseUs(handle);
    uint32_t high_phase_us = RafionBitbangGetHighPhaseUs(handle);

    if (bit_value == NULL)
    {
        handle->last_error = HAL_I2C_ERROR_BERR;
        return RAFION_BITBANG_ERROR;
    }

    RafionBitbangPullSclLow(handle);
    RafionBitbangReleaseSda(handle);
    RafionBitbangDelayUs(low_phase_us);
    RafionBitbangReleaseScl(handle);

    if (RafionBitbangWaitForSclHigh(handle, RafionBitbangGetStretchTimeoutMs(handle)) != HAL_OK)
    {
        return RAFION_BITBANG_ERROR;
    }

    RafionBitbangDelayUs(high_phase_us);
    *bit_value = (RafionBitbangReadSda(handle) == GPIO_PIN_SET);
    RafionBitbangPullSclLow(handle);

    return RAFION_BITBANG_OK;
}
