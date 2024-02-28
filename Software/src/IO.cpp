#include "IO.h"

bool IO::Begin(void)
{
    if (Initial_configuration())
    {
        ESP_LOGI(IOTAG, "IO inicializado correctamente.");
        return true;
    }

    return false;
}

bool IO::Initial_configuration(void)
{
    // Configuracion inicial del pin RXEN del E22
    if (!SetConfig(RX_EN_E22_PIN, GPIO_MODE_OUTPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_ENABLE, GPIO_INTR_DISABLE))
    {
        return false;
    };

    // Configuracion inicial del pin TXEN del E22
    if (!SetConfig(TX_EN_E22_PIN, GPIO_MODE_OUTPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_ENABLE, GPIO_INTR_DISABLE))
    {
        return false;
    };

    // Configuracion inicial del pin NRST del E22
    if (!SetConfig(NRST_E22_PIN, GPIO_MODE_OUTPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_DISABLE))
    {
        return false;
    };

    // Configuracion inicial del pin BUSY del E22
    if (!SetConfig(BUSY_E22_PIN, GPIO_MODE_INPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_DISABLE))
    {
        return false;
    };

    // Estado inicial del pin RX_EN_E22_PIN del E22
    if (SetLevel((gpio_num_t)RX_EN_E22_PIN, 0))
    {
        return false;
    }

    // Estado inicial del pin TX_EN_E22_PIN del E22
    if (SetLevel((gpio_num_t)TX_EN_E22_PIN, 0))
    {
        return false;
    }

    // Estado inicial del pin NRST_E22_PIN del E22
    if (SetLevel((gpio_num_t)NRST_E22_PIN, 0))
    {
        return false;
    }
    
    return true;
}

bool IO::SetConfig(uint64_t pin_bit_mask, gpio_mode_t mode, gpio_pullup_t pull_up_en, gpio_pulldown_t pull_down_en, gpio_int_type_t intr_type)
{
    gpio_config_t ioConf;
    memset(&ioConf, 0, sizeof(gpio_config_t));

    ioConf.pin_bit_mask = (1ULL << pin_bit_mask);
    ioConf.mode = mode;
    ioConf.pull_down_en = pull_down_en;
    ioConf.pull_up_en = pull_up_en;
    ioConf.intr_type = intr_type;
    if (gpio_config(&ioConf) != ESP_OK)
    {
        return false;
    }
    return true;
}

bool IO::SetLevel(gpio_num_t gpio_num, uint32_t level)
{
    if (gpio_set_level(gpio_num, level) != ESP_OK)
    {
        return false;
    }
    return true;
}

bool IO::GetLevel(gpio_num_t gpio_num)
{
    if (gpio_get_level(gpio_num) == 1)
    {
        return true;
    }
    return false;
}