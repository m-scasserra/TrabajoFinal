#include "SPI.h"


bool SPI::Begin(spi_bus_config_t *SPIBusCfg, spi_host_device_t host_id, spi_dma_chan_t dma_chan)
{
    if (SPIInitialized)
    {
        ESP_LOGI(SPITAG, "The SPI has already been initialized.");
        return true;
    }

    // Initialize the SPI bus
    ESP_LOGI(SPITAG, "Starting the SPI bus.");
    esp_err_t err = spi_bus_initialize(host_id, SPIBusCfg, dma_chan);
    if (err == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(SPITAG, "The SPI bus has already been initialized.");
        SPIInitialized = true;
        return false;
    }
    else if (err != ESP_OK)
    {
        ESP_LOGE(SPITAG, "Error initializing the SPI bus.");
        return false;
    }

    ESP_LOGI(SPITAG, "SPI Bus initialized correctly.");
    SPIInitialized = true;
    return true;
}

bool SPI::AddDevice(spi_device_interface_config_t *SPISlaveCfg)
{
    if (!SPIInitialized)
    {
        ESP_LOGE(SPITAG, "The SPI has not been initialized.");
        return false;
    }

    // Attach the E22 to the SPI bus
    ESP_LOGI(SPITAG, "Adding the SPI device");

    if (spi_bus_add_device(SPI2_HOST, SPISlaveCfg, &SPIHandle) != ESP_OK)
    {
        ESP_LOGE(SPITAG, "Error adding the slave device to the SPI Bus");
        return false;
    }

    ESP_LOGI(SPITAG, "The slave device was added correctly.");

    return true;
}

bool SPI::SendMessage(uint8_t *tx_msg, uint8_t tx_len, uint8_t *rx_msg, uint8_t rx_len)
{
    if (!SPIInitialized)
    {
        ESP_LOGE(SPITAG, "The SPI has not been initialized.");
        return false;
    }

    if (tx_len == 0)
    {
        ESP_LOGE(SPITAG, "The message size must be greater than 0.");
        return false;
    }

    if (rx_msg == NULL && rx_len > 0)
    {
        ESP_LOGE(SPITAG, "The receive buffer cannot be null if the size is greater than 0.");
        return false;
    }

    if (tx_msg == NULL)
    {
        ESP_LOGE(SPITAG, "The transmission buffer cannot be null.");
        return false;
    }

    spi_transaction_t message;
    memset(&message, 0, sizeof(spi_transaction_t));

    message.length = 8 * tx_len;
    message.rxlength = 8 * rx_len;
    message.user = NULL;
    message.tx_buffer = tx_msg;
    message.rx_buffer = rx_msg;

    if (spi_device_polling_transmit(SPIHandle, &message) != ESP_OK)
    {
        ESP_LOGE(SPITAG, "Error sending the message.");
        return false;
    }
    debugTXmessage(tx_msg, tx_len);
    debugRXmessage(rx_msg, rx_len);
    return true;
}

bool SPI::SendMessage(uint8_t *tx_msg, uint8_t tx_len)
{
    return SendMessage(tx_msg, tx_len, NULL, 0);
}

void SPI::debugTXmessage(uint8_t *tx_msg, uint8_t tx_len)
{
    if (tx_len == 0 || tx_msg == NULL)
    {
        return;
    }

    // Create a buffer to store the formatted string
    char formatted_string[tx_len * 5 + 1]; // Allocate enough space for hex, spaces and null terminator
    memset(formatted_string, 0, sizeof(formatted_string));
    int index = 0;

    for (int i = 0; i < tx_len; i++)
    {
        // Format each byte as a two-digit hex string and store it in the buffer
        sprintf(formatted_string + index, "0x%02X ", tx_msg[i]);
        index += 5; // Increment index by 5 for the formatted byte + spaces
    }

    // Print the formatted string with a trailing newline
    ESP_LOGD(SPITAG, "TX message: %s\n", formatted_string);
}

void SPI::debugRXmessage(uint8_t *rx_msg, uint8_t rx_len)
{
    if (rx_len == 0 || rx_msg == NULL)
    {
        return;
    }

    // Create a buffer to store the formatted string
    char formatted_string[rx_len * 5 + 1]; // Allocate enough space for hex, spaces and null terminator
    memset(formatted_string, 0, sizeof(formatted_string));
    int index = 0;

    for (int i = 0; i < rx_len; i++)
    {
        // Format each byte as a two-digit hex string and store it in the buffer
        sprintf(formatted_string + index, "0x%02X ", rx_msg[i]);
        index += 5; // Increment index by 5 for the formatted byte + spaces
    }

    // Print the formatted string with a trailing newline
    ESP_LOGD(SPITAG, "RX message: %s\n", formatted_string);
}