#include "mock_spi.h"

// Include the original ESP-IDF SPI header (if applicable)
// #include <driver/spi.h>  // Uncomment if needed

// Define mock behavior for spi_device_polling_transmit
esp_err_t mock_spi_device_polling_transmit(spi_device_handle_t handle, spi_transaction_t *trans) {
  // Implement your mocking logic here
  // You can control the return value (ESP_OK, error code) and potentially modify transaction parameters
  // based on your testing requirements

  // Example: Simulate successful transmission
  return ESP_OK;

  // Example: Simulate an error
  // return ESP_FAIL;

  // Example: Modify transaction length (be cautious, might not be suitable for all scenarios)
  // trans->length = 16; // Adjust as needed
  // return ESP_OK;
}