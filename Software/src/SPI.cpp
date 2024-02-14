#include "SPI.h"

bool SPI::Begin(void)
{
    esp_err_t ret;

    // Configuracion del bus SPI
    spi_bus_config_t SPIBusCfg;
    memset(&SPIBusCfg, 0, sizeof(spi_bus_config_t));     // Seteo el SPIBus en 0 para asegurarme que ningun parametro esta pre iniciado
    SPIBusCfg.mosi_io_num = SPI_MOSI_PIN;         // Pin del MOSI
    SPIBusCfg.miso_io_num = SPI_MISO_PIN;         // Pin del MiSO
    SPIBusCfg.sclk_io_num = SPI_SCK_PIN;          // Pin del SCK
    SPIBusCfg.quadhd_io_num = -1;                 // No se usa el Write Protect
    SPIBusCfg.quadwp_io_num = -1;                 // No se usa el Hold
    SPIBusCfg.data4_io_num = -1;                  // No se usa el spi Data 4 signal porque no estamos en OSPI
    SPIBusCfg.data5_io_num = -1;                  // No se usa el spi Data 5 signal porque no estamos en OSPI
    SPIBusCfg.data6_io_num = -1;                  // No se usa el spi Data 6 signal porque no estamos en OSPI
    SPIBusCfg.data7_io_num = -1;                  // No se usa el spi Data 7 signal porque no estamos en OSPI
    SPIBusCfg.max_transfer_sz = SPI_MAX_TRANSFER; // Maximo tamaño de transferencia
    SPIBusCfg.flags = SPICOMMON_BUSFLAG_MASTER;   // Flag para indicar que el µC es el master en SPI
    SPIBusCfg.isr_cpu_id = INTR_CPU_ID_AUTO;      // CPU que se encarga de las interrupciones, en el ESP32-C3 hay un solo CPU
    SPIBusCfg.intr_flags = SPI_INTR_BUS_FLAGS;    // Flags de configuracion para el bus SPI

    // Initialize the SPI bus
    ESP_LOGI(SPITAG, "Iniciando el bus SPI.");
    if (spi_bus_initialize(SPI2_HOST, &SPIBusCfg, SPI_DMA_DISABLED) != ESP_OK)
    {
        ESP_LOGE(SPITAG, "Error al inicializar el SPI Bus.");
        return false;
    }
    ESP_LOGI(SPITAG, "SPI Bus inicializado correctamente.");

    return true;
}



spi_device_handle_t * SPI::GetSPIHandle(void)
{
    return &SPIHandle;
}