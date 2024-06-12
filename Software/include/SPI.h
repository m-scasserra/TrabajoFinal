#ifndef SPI_H
#define SPI_H

#include "CommonIncludes.h"

#include "driver/spi_master.h"

#define SPITAG "SPI"

// SPI2_HOST
#define SPI_MAX_TRANSFER 1024
#define SPI_INTR_BUS_FLAGS 0

class SPI
{
public:
    SPI() { SPIInitialized = false; };

    /**
     * @brief Initializes the SPI bus
     * 
     * @param SPIBusCfg SPI bus configuration
     * @param host_id SPI host device
     * @param dma_chan SPI DMA channel
     * @return true If the SPI bus was initialized correctly
     * @return false If there was an error initializing the SPI bus
     */
    bool Begin(spi_bus_config_t *SPIBusCfg, spi_host_device_t host_id = SPI2_HOST, spi_dma_chan_t dma_chan = SPI_DMA_DISABLED);

    /**
     * @brief Adds a device to the SPI bus
     * 
     * @param SPISlaveCfg SPI slave configuration
     * @return true If the slave device was added correctly
     * @return false If there was an error adding the slave device
     */
    bool AddDevice(spi_device_interface_config_t *SPISlaveCfg);

    /**
     * @brief Sends a message to the slave device
     * 
     * @param tx_msg Message to be sent
     * @param tx_len Length of the message to be sent
     * @param rx_msg Buffer to store the received message
     * @param rx_len Length of the buffer to store the received message
     * @return true If the message was sent and received correctly
     * @return false If there was an error sending or receiving the message
     */
    bool SendMessage(uint8_t *tx_msg, uint8_t tx_len, uint8_t *rx_msg, uint8_t rx_len);

    /**
     * @brief Sends a message to the slave device
     * 
     * @param tx_msg Message to be sent
     * @param tx_len Length of the message to be sent
     * @return true If the message was sent correctly
     * @return false If there was an error sending the message
     */
    bool SendMessage(uint8_t *tx_msg, uint8_t tx_len);

private:

    /**
     * @brief Debug function to print the message sent
     * 
     * @param tx_msg Message to be printed
     * @param tx_len Length of the message to be printed
     */
    void debugTXmessage(uint8_t *tx_msg, uint8_t tx_len);

    /**
     * @brief Debug function to print the message received
     * 
     * @param rx_msg Message to be printed
     * @param rx_len Length of the message to be printed
     */
    void debugRXmessage(uint8_t *rx_msg, uint8_t rx_len);

    /**
     * @brief Flag to check if the SPI bus was initialized
     */
    bool SPIInitialized;

    /**
     * @brief Handle to the SPI device
     */
    spi_device_handle_t SPIHandle;
};

#endif // SPI_H