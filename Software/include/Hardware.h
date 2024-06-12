/**
 * @file Hardware.h
 * @brief Defines the pin mappings for various hardware components.
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#define RX_EN_E22_PIN 0             /**< Pin number for RX enable of E22 module. */
#define TX_EN_E22_PIN 1             /**< Pin number for TX enable of E22 module. */
#define NRST_E22_PIN 2              /**< Pin number for reset of E22 module. */
#define ADC_PIN 3                   /**< Pin number for ADC. */
#define SPI_SCK_PIN 4               /**< Pin number for SPI clock. */
#define SPI_MISO_PIN 5              /**< Pin number for SPI MISO. */
#define SPI_MOSI_PIN 6              /**< Pin number for SPI MOSI. */
#define SPI_CS_E22_PIN 7            /**< Pin number for SPI chip select of E22 module. */
#define RGB_LED_PIN 8               /**< Pin number for RGB LED. */
#define SPI_CS_EXT_SENSOR_PIN 10    /**< Pin number for SPI chip select of external sensor. */
#define DIO1_E22_PIN 18             /**< Pin number for DIO1 of E22 module. */
#define BUSY_E22_PIN 19             /**< Pin number for BUSY of E22 module. */
#define UART_RX 20                  /**< Pin number for UART RX. */
#define UART_TX 21                  /**< Pin number for UART TX. */

#endif // HARDWARE_H