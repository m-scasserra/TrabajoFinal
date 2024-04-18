#include "IO.h"

bool IO::Begin(void)
{
    if (true)
    {
        ESP_LOGI(IOTAG, "IO inicializado correctamente.");
        return true;
    }

    return false;
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