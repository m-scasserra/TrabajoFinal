#include "SPI.h"

spi_device_handle_t SPI::SPIHandle = NULL;

bool SPI::Begin(spi_bus_config_t *SPIBusCfg)
{
    // Initialize the SPI bus
    ESP_LOGI(SPITAG, "Inicio el bus SPI.");
    if (spi_bus_initialize(SPI2_HOST, SPIBusCfg, SPI_DMA_DISABLED) != ESP_OK)
    {
        ESP_LOGE(SPITAG, "Error al inicializar el SPI Bus.");
        return false;
    }
    ESP_LOGI(SPITAG, "SPI Bus inicializado correctamente.");

    return true;
}

bool SPI::AddDevice(spi_device_interface_config_t *SPISlaveCfg)
{
    // Attach the E22 to the SPI bus
    ESP_LOGI(SPITAG, "Agrego el dispositivo SPI");

    if (spi_bus_add_device(SPI2_HOST, SPISlaveCfg, &SPIHandle) != ESP_OK)
    {
        ESP_LOGE(SPITAG, "Error al agregar el dispositivo esclavo al SPI Bus");
        return false;
    }
    ESP_LOGI(SPITAG, "El dispositivo esclavo fue agregado correctamente.");

    return true;
}

bool SPI::SendMessage(uint8_t *tx_msg, uint8_t tx_len, uint8_t *rx_msg, uint8_t rx_len)
{
    spi_transaction_t message;
    memset(&message, 0, sizeof(spi_transaction_t));

    //message.cmd = E22_CMD_GetStatus;
    message.length = 8 * tx_len;
    message.rxlength = 8 * rx_len;
    message.user = NULL;
    message.tx_buffer = NULL;
    message.tx_buffer = tx_msg;
    message.rx_buffer = rx_msg;

    if (spi_device_polling_transmit(SPIHandle, &message) != ESP_OK)
    {
        ESP_LOGE("SPI", "Error en spi_device_polling_transmit()");
        return false;
    }
    return true;
}

bool SPI::SendMessage(uint8_t *tx_msg, uint8_t tx_len)
{
    spi_transaction_t message;
    memset(&message, 0, sizeof(spi_transaction_t));

    message.length = 8 * tx_len;
    message.user = NULL;
    message.tx_buffer = tx_msg;
    message.rx_buffer = NULL;

    if (spi_device_polling_transmit(SPIHandle, &message) != ESP_OK)
    {
        ESP_LOGE("SPI", "Error en spi_device_polling_transmit()");
        return false;
    }
    return true;

}