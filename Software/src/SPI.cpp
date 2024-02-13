#include "SPI.h"

bool SPI::Begin(void)
{
    esp_err_t ret;

    // Configuracion del bus SPI
    spi_bus_config_t SPIBusCfg;
    memset(&SPIBusCfg, 0, sizeof(SPIBusCfg));
    SPIBusCfg.mosi_io_num = SPI_MOSI_PIN;       // Pin del MOSI
    SPIBusCfg.miso_io_num = SPI_MISO_PIN;       // Pin del MiSO
    SPIBusCfg.sclk_io_num = SPI_SCK_PIN;        // Pin del SCK
    SPIBusCfg.quadhd_io_num = -1;
    SPIBusCfg.quadwp_io_num = -1;
    SPIBusCfg.max_transfer_sz = 1024;           // Maximo tamaño de transferencia
    SPIBusCfg.flags = SPICOMMON_BUSFLAG_MASTER; // Flag para indicar que el µC es el master en SPI
    //SPIBusCfg.intr_flags = ESP_INTR_FLAG_INTRDISABLED;
    SPIBusCfg.intr_flags = 0;

    // Configuracion del dispositivo esclavo
    spi_device_interface_config_t SPISlaveCfg;
    memset(&SPISlaveCfg, 0, sizeof(SPISlaveCfg));
    SPISlaveCfg.command_bits = 8;
    SPISlaveCfg.address_bits = 16;
    SPISlaveCfg.dummy_bits = 0;
    SPISlaveCfg.mode = 0; // SPI mode 0
    SPISlaveCfg.clock_source = SPI_CLK_SRC_XTAL;
    SPISlaveCfg.duty_cycle_pos = 128;
    SPISlaveCfg.cs_ena_pretrans = 1;
    SPISlaveCfg.cs_ena_posttrans = 1;
    SPISlaveCfg.clock_speed_hz = 10 * 1000 * 1000; // Clock out at 10 MHz
    SPISlaveCfg.input_delay_ns = 0;
    SPISlaveCfg.spics_io_num = SPI_CS_E22_PIN; // CS pin
    SPISlaveCfg.flags = 0;
    SPISlaveCfg.queue_size = 1; // We want to be able to queue 7 transactions at a time
    SPISlaveCfg.pre_cb = NULL;
    SPISlaveCfg.post_cb = NULL;

    // Initialize the SPI bus
    ret = spi_bus_initialize(SPI2_HOST, &SPIBusCfg, SPI_DMA_DISABLED);
    ESP_ERROR_CHECK(ret);
    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(SPI2_HOST, &SPISlaveCfg, &SPIHandle);
    ESP_ERROR_CHECK(ret);

    return true;
}

bool SPI::GetStatus(void)
{
    uint8_t data = 0;
    uint8_t pBuffer2[2] = {0, 0};

    spi_transaction_t t;

    t.cmd = 0xC0;
    t.length = 8 * 2;
    t.rxlength = 8 * 2;
    t.rx_buffer = pBuffer2;
    t.flags = 0;

    spi_device_polling_transmit(SPIHandle, &t);

    data = ((uint8_t *)pBuffer2)[1];

    switch (data & 0x70)
    {
    case 0x00:
        ESP_LOGI("SPI", "Chip Mode Unused");
        break;

    case 0x20:
        ESP_LOGI("SPI", "Chip Mode STBY_RC");
        break;

    case 0x30:
        ESP_LOGI("SPI", "Chip Mode STBY_XOSC");
        break;

    case 0x40:
        ESP_LOGI("SPI", "Chip Mode FS");
        break;

    case 0x50:
        ESP_LOGI("SPI", "Chip Mode RX");
        break;

    case 0x60:
        ESP_LOGI("SPI", "Chip Mode TX");
        break;

    default:
        ESP_LOGI("SPI", "Chip Mode Undefined");
        break;
    }

    switch (data & 0x0E)
    {
    case 0x00:
        ESP_LOGI("SPI", "Command Status Reserved");
        break;

    case 0x04:
        ESP_LOGI("SPI", "Command Status Data is available to host");
        break;

    case 0x06:
        ESP_LOGI("SPI", "Command Status Command timeout");
        break;

    case 0x08:
        ESP_LOGI("SPI", "Command Status Command processing error");
        break;

    case 0x0A:
        ESP_LOGI("SPI", "Command Status Failure to execute command");
        break;

    case 0x0C:
        ESP_LOGI("SPI", "Command Status Command TX done");
        break;

    default:
        ESP_LOGI("SPI", "Command Status Undefined");
        break;
    }

    ESP_LOGI("SPI", "Valor de data:   %d", data & 0xff);
    return true;
}