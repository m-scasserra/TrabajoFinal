#include "IO.h"

bool IO::Begin(void)
{
    gpio_config_t ioConf;
    memset(&ioConf, 0, sizeof(gpio_config_t));

    // Configuracion del pin RXEN del E22
    ioConf.pin_bit_mask = (1ULL << RX_EN_E22_PIN);
    ioConf.mode = GPIO_MODE_OUTPUT;
    ioConf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    ioConf.pull_up_en = GPIO_PULLUP_DISABLE;
    ioConf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&ioConf);

    // Configuracion del pin TXEN del E22
    ioConf.pin_bit_mask = (1ULL << TX_EN_E22_PIN);
    ioConf.mode = GPIO_MODE_OUTPUT;
    ioConf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    ioConf.pull_up_en = GPIO_PULLUP_DISABLE;
    ioConf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&ioConf);

    return true;
}

bool IO::SetLevel(gpio_num_t gpio_num, uint32_t level)
{
    gpio_set_level(gpio_num, level);
    return true;
}