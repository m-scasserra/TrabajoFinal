#include "E22Driver.h"

bool E22::Begin(void)
{
    SPI &spi = SPI::getInstance();
    SPIHandle = *spi.GetSPIHandle();

    // Configuracion del dispositivo esclavo
    spi_device_interface_config_t SPISlaveCfg;
    memset(&SPISlaveCfg, 0, sizeof(spi_device_interface_config_t)); // Seteo el SPISlaveCfg en 0 para asegurarme que ningun parametro esta pre iniciado
    SPISlaveCfg.command_bits = SPI_COMMAND_LEN;                     // Largo en bits de loa comandos
    SPISlaveCfg.address_bits = SPI_ADDR_LEN;                        // Largo en bits de las direcciones de los registros a leer/escribir
    SPISlaveCfg.dummy_bits = SPI_DUMMY_BITS;                        // Cantidad en bits de DummyBits enviar si la comunicacion fuera HalfDuplex
    SPISlaveCfg.mode = SPI_MODE;                                    // SPI mode 0
    SPISlaveCfg.clock_source = SPI_CLK_SRC_XTAL;                    // Eligo el XTAL como clock source para no tener problemas si cambio la CPU frec
    SPISlaveCfg.duty_cycle_pos = SPI_DUTY_CYCLE;                    // Duty Cycle de la señal SPI
    SPISlaveCfg.cs_ena_pretrans = CYCLES_CS_BEFORE_TRANS;           // Ciclos de clock a esperar entre que cambio el CS de valor y empiezo una transaccion
    SPISlaveCfg.cs_ena_posttrans = CYCLES_CS_AFTER_TRANS;           // Ciclos de clock a esperar entre que termino una transaccion y cambio el CS de valor
    SPISlaveCfg.clock_speed_hz = SPI_CLOCK;                         // Clock out at 10 MHz
    SPISlaveCfg.input_delay_ns = SPI_INPUT_DELAY;                   // Delay en nS de la transaccion
    SPISlaveCfg.spics_io_num = SPI_CS_E22_PIN;                      // CS pin
    SPISlaveCfg.flags = SPI_DEVICE_CONFIG_FLAGS;                    // Flags de configuracion para el Device
    SPISlaveCfg.queue_size = SPI_QUEUE_SIZE;                        // We want to be able to queue 1 transaction at a time
    SPISlaveCfg.pre_cb = NULL;                                      // Callback pre transaccion, no se implementa
    SPISlaveCfg.post_cb = NULL;                                     // Callback post transaccion, no se implementa

    // Attach the E22 to the SPI bus
    ESP_LOGI(SPITAG, "Agrego el dispositivo E22");

    if (spi_bus_add_device(SPI2_HOST, &SPISlaveCfg, &SPIHandle) != ESP_OK)
    {
        ESP_LOGE(SPITAG, "Error al agregar al E22 al SPI Bus. Recordar iniciar primero el SPI Bus.");
        return false;
    }
    ESP_LOGI(SPITAG, "E22 agregado correctamente.");

    return true;
}

bool E22::GetStatus(void)
{
    uint8_t respBuffer[4] = {0, 0, 0, 0};

    spi_transaction_t message;
    memset(&message, 0, sizeof(spi_transaction_t));
    message.flags = SPI_TRANS_USE_TXDATA;
    //message.cmd = E22_CMD_GetStatus;
    message.cmd = 0;
    message.addr = 0;
    message.length = 8 * 3;
    message.rxlength = 8 * 3;
    message.user = NULL;
    message.tx_data[0] = 0x15;
    //message.tx_data[0] = E22_CMD_GetStatus;
    message.tx_data[1] = 0x00;
    message.tx_data[2] = 0x00;
    message.tx_data[3] = 0x00;
    //message.tx_buffer = NULL;
    message.rx_buffer = respBuffer;
    
    printf("Pregunto por el status del E22\r\n");
    if (spi_device_polling_transmit(SPIHandle, &message) != ESP_OK)
    {
        ESP_LOGE("SPI", "Error en spi_device_polling_transmit(SPIHandle, &t)");
        return false;
    }
    printf("Chip Mode: ");
    switch (respBuffer[1] & 0x70)
    {
    case 0x00:
        printf("Unused");
        break;

    case 0x20:
        printf("STBY_RC");
        break;

    case 0x30:
        printf("STBY_XOSC");
        break;

    case 0x40:
        printf("FS");
        break;

    case 0x50:
        printf("RX");
        break;

    case 0x60:
        printf("TX");
        break;

    default:
        printf("Undefined");
        break;
    }

    printf("    Command Status: ");
    switch (respBuffer[1] & 0x0E)
    {
    case 0x00:
        printf("Reserved");
        break;

    case 0x04:
        printf("Data is available to host");
        break;

    case 0x06:
        printf("Command timeout");
        break;

    case 0x08:
        printf("Command processing error");
        break;

    case 0x0A:
        printf("Failure to execute command");
        break;

    case 0x0C:
        printf("Command TX done");
        break;

    default:
        printf("Undefined");
        break;
    }
    printf("\r\n");
    printf("Valor de status: 0x%x\r\n", respBuffer[0] & 0xff);
    printf("Valor de status: 0x%x\r\n", respBuffer[1] & 0xff);
    printf("Valor de status: 0x%x\r\n", respBuffer[2] & 0xff);
    printf("Valor de status: 0x%x\r\n", respBuffer[3] & 0xff);
    return true;
}