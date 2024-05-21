#include <string.h>

#include "IO.h"
#include "FileSystem.h"

#include "esp_log.h"
#include "adc_cali_interface.h"
#include "adc_cali_schemes.h"
#include "esp_adc/adc_cali_scheme.h"

adc_oneshot_unit_handle_t IO::adc1_handle;
adc_cali_handle_t IO::adc1_cali_chan3_handle = NULL;
bool IO::do_calibADC1_chan3 = true;
bool IO::calibratedADC1_chan3 = false;

bool IO::Begin(void)
{

    if (ConfigADC())
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

bool IO::ConfigADC(void)
{
    // ADC initialization

    // Inicializa el ADC1
    adc_oneshot_unit_init_cfg_t init_configADC = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE};
    if (adc_oneshot_new_unit(&init_configADC, &adc1_handle) != ESP_OK)
    {
        ESP_LOGE(IOTAG, "adc_oneshot_new_unit failed");
        return false;
    }

    // ADC calibration
    if (do_calibADC1_chan3)
    {
        ESP_LOGI(IOTAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = ADC_UNIT_1,
            .chan = ADC_CHANNEL_3,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };

        esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &adc1_cali_chan3_handle);
        if (ret == ESP_OK)
        {
            calibratedADC1_chan3 = true;
        }

        if (ret == ESP_OK)
        {
            ESP_LOGI(IOTAG, "Calibration Success");
        }
        else if (ret == ESP_ERR_NOT_SUPPORTED || !calibratedADC1_chan3)
        {
            ESP_LOGW(IOTAG, "eFuse not burnt, skip software calibration");
        }
        else
        {
            ESP_LOGE(IOTAG, "Invalid arg or no memory");
        }
    }

    // ADC configuration for ADC_CHANNEL_0
    adc_oneshot_chan_cfg_t configADC = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12};
    // Configuro el ADC 1 CHAN 3 ADC Channel 0 corresponde al IO 3
    if (adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &configADC) != ESP_OK)
    {
        ESP_LOGE(IOTAG, "Error al configurar el canal ADC 1 CHAN 3 ADC Channel 0");
        return false;
    }

    return true;
}

bool IO::getADCOneShotRaw(int32_t *value)
{
    if (adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, (int *)value) != ESP_OK)
    {
        ESP_LOGE(IOTAG, "Error al leer el canal ADC 1 CHAN 3 ADC Channel 0");
        return false;
    }

    ESP_LOGI(IOTAG, "ADC1 CHAN 3 ADC Channel 0: %ld", *value);
    return true;
}

bool IO::getADCOneShotConverted(int32_t *value)
{
    if (!calibratedADC1_chan3)
    {
        ESP_LOGE(IOTAG, "ADC1 CHAN 3 no calibrado");
        return false;
    }

    int32_t aux = 0;
    if (!getADCOneShotRaw(&aux))
    {
        return false;
    }
    
    if (adc_cali_raw_to_voltage(adc1_cali_chan3_handle, (int)&aux, (int *)value) != ESP_OK)
    {
        ESP_LOGE(IOTAG, "Error al convertir el canal ADC 1 CHAN 3 ADC Channel 0");
        return false;
    }

    ESP_LOGI(IOTAG, "ADC1 CHAN 3 ADC Calibrated Channel 0: %ld", *value);
    return true;
}

bool IO::getAndSaveADCOneShotRaw(void)
{
    FS &fs = FS::getInstance();
    int32_t value = 0;
    if (!getADCOneShotRaw(&value))
    {
        return false;
    }
    if (!fs.WriteFile(&value, sizeof(int32_t), 1, ADC_BIN_PATH, "w"))
    {
        ESP_LOGE(IOTAG, "Error al guardar el valor de ADC1 CHAN 3 ADC Channel 0");
        return false;
    }
    
    return true;
}

bool IO::getAndSaveADCOneConverted(void)
{
    FS &fs = FS::getInstance();
    int32_t value = 0;
    if (!getADCOneShotConverted(&value))
    {
        return false;
    }
    if (!fs.WriteFile(&value, sizeof(int32_t), 1, ADC_BIN_PATH, "w"))
    {
        ESP_LOGE(IOTAG, "Error al guardar el valor de ADC1 CHAN 3 ADC Channel 0");
        return false;
    }
    
    return true;
}