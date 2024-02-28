#include "E22Driver.h"

TaskHandle_t E22::E22TaskHandle = NULL;

void E22::E22Task(void *pvParameters)
{
    E22 *e22 = (E22 *)pvParameters;
    while (!e22->isBusy())
    {
        ESP_LOGI(E22TAG, "Esperando a que el E22 se apague.");
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    ESP_LOGI(E22TAG, "E22 apagado, lo prendo.");

    e22->resetOff();

    while (1)
    {
        e22->processCmd();
        vTaskDelay(pdMS_TO_TICKS(1000)); // Espero al siguiente CMD
    }
}

bool E22::Begin(void)
{
    SPI &spi = SPI::getInstance();

    // Seteo todos los valores de las variables a 0
    rssiInst = 0;

    // Creo la queue de mensajes

    xE22CmdQueue = xQueueCreate(MAX_E22_CMD_QUEUE, sizeof(uint8_t));

    if (xE22CmdQueue == NULL)
    {
        /* Queue was not created and must not be used. */
    }

    // Configuracion del bus SPI
    spi_bus_config_t SPIBusCfg;
    memset(&SPIBusCfg, 0, sizeof(spi_bus_config_t)); // Seteo el SPIBus en 0 para asegurarme que ningun parametro esta pre iniciado
    SPIBusCfg.mosi_io_num = SPI_MOSI_PIN;            // Pin del MOSI
    SPIBusCfg.miso_io_num = SPI_MISO_PIN;            // Pin del MiSO
    SPIBusCfg.sclk_io_num = SPI_SCK_PIN;             // Pin del SCK
    SPIBusCfg.quadhd_io_num = -1;                    // No se usa el Write Protect
    SPIBusCfg.quadwp_io_num = -1;                    // No se usa el Hold
    SPIBusCfg.data4_io_num = -1;                     // No se usa el spi Data 4 signal porque no estamos en OSPI
    SPIBusCfg.data5_io_num = -1;                     // No se usa el spi Data 5 signal porque no estamos en OSPI
    SPIBusCfg.data6_io_num = -1;                     // No se usa el spi Data 6 signal porque no estamos en OSPI
    SPIBusCfg.data7_io_num = -1;                     // No se usa el spi Data 7 signal porque no estamos en OSPI
    SPIBusCfg.max_transfer_sz = SPI_MAX_TRANSFER;    // Maximo tamaño de transferencia
    SPIBusCfg.flags = SPICOMMON_BUSFLAG_MASTER;      // Flag para indicar que el µC es el master en SPI
    SPIBusCfg.isr_cpu_id = INTR_CPU_ID_AUTO;         // CPU que se encarga de las interrupciones, en el ESP32-C3 hay un solo CPU
    SPIBusCfg.intr_flags = SPI_INTR_BUS_FLAGS;       // Flags de configuracion para el bus SPI

    // Configuracion SPI del E22
    spi_device_interface_config_t SPISlaveCfg;
    memset(&SPISlaveCfg, 0, sizeof(spi_device_interface_config_t)); // Seteo el SPISlaveCfg en 0 para asegurarme que ningun parametro esta pre iniciado
    SPISlaveCfg.command_bits = SPI_COMMAND_LEN;                     // Largo en bits de loa comandos
    SPISlaveCfg.address_bits = SPI_ADDR_LEN;                        // Largo en bits de las direcciones de los registros a leer/escribir
    SPISlaveCfg.dummy_bits = SPI_DUMMY_BITS;                        // Cantidad en bits de DummyBits enviar si la comunicacion fuera HalfDuplex
    SPISlaveCfg.mode = SPI_MODE;                                    // SPI mode 0
    SPISlaveCfg.clock_source = SPI_CLK_SRC_APB;                     // Eligo el XTAL como clock source para no tener problemas si cambio la CPU frec
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

    spi.Begin(&SPIBusCfg);
    spi.AddDevice(&SPISlaveCfg);

    ESP_LOGI(E22TAG, "Configuracion del SPI correcta, iniciando tarea del E22");

    xTaskCreatePinnedToCore(
        E22Task,            // Task function.
        "E22 Control Task", // Name of task.
        10000,              // Stack size of task
        (void *)this,       // Parameter of the task
        1,                  // Priority of the task
        &E22TaskHandle,     // Task handle to keep track of created task
        0);                 // Pin task to core 0

    return true;
}

bool E22::getStatus(void)
{
    SPI &spi = SPI::getInstance();
    uint8_t RxBuffer[2];
    uint8_t TxBuffer[2] = {E22_CMD_GetStatus, 0};
    memset(RxBuffer, 0, sizeof(RxBuffer));

    if (!spi.SendMessage(TxBuffer, 2, RxBuffer, 2))
    {
        ESP_LOGE(E22TAG, "Error al enviar el comando getStatus");
        return false;
    }

    processStatus(RxBuffer[1]);
    ESP_LOGD(E22TAG, "Valor de la respuesta de getStatus RxBuffer[0]:%u  RxBuffer[1]:%u", RxBuffer[0], RxBuffer[1]);
    return true;
}

bool E22::getRssiInst(void)
{
    SPI &spi = SPI::getInstance();
    uint8_t RxBuffer[3];
    uint8_t TxBuffer[3] = {E22_CMD_GetRssiInst, 0, 0};
    memset(RxBuffer, 0, sizeof(RxBuffer));

    if (!spi.SendMessage(TxBuffer, 3, RxBuffer, 3))
    {
        ESP_LOGE(E22TAG, "Error al enviar el comando getRssiInst");
        return false;
    }

    processStatus(RxBuffer[1]);
    ESP_LOGD(E22TAG, "Valor de la respuesta de GetRssiInt RxBuffer[0]:%u  RxBuffer[1]:%u  RxBuffer[2]:%u", RxBuffer[0], RxBuffer[1], RxBuffer[2]);
    rssiInst = RxBuffer[2] / 2;
    return true;
}

bool E22::setBufferBaseAddress(uint8_t TxBaseAddr, uint8_t RxBaseAddr)
{
    SPI &spi = SPI::getInstance();
    uint8_t TxBuffer[3] = {E22_CMD_SetBufferBaseAddress, TxBaseAddr, RxBaseAddr};

    if (!spi.SendMessage(TxBuffer, 3))
    {
        ESP_LOGE(E22TAG, "Error al enviar el comando setBufferBaseAddress");
        return false;
    }

    return true;
}

bool E22::setRx(uint32_t Timeout)
{
    SPI &spi = SPI::getInstance();
    if (Timeout > 0xFFFFFF)
    {
        ESP_LOGE(E22TAG, "Timeout invalido, tiene que ser un valor entre 0x000000 y 0xFFFFFF.");
        return false;
    }

    uint8_t TxBuffer[4] = {E22_CMD_SetRX, (uint8_t)(Timeout >> 16), (uint8_t)(Timeout >> 8), (uint8_t)(Timeout & 0xFF)};

    if (!spi.SendMessage(TxBuffer, 4))
    {
        ESP_LOGE(E22TAG, "Error al enviar el comando SetRx");
        return false;
    }

    return true;
}

bool E22::setStandBy(StdByMode_t mode)
{
    SPI &spi = SPI::getInstance();

    uint8_t TxBuffer[2] = {E22_CMD_SetStandby, (uint8_t)mode};

    if (!spi.SendMessage(TxBuffer, 2))
    {
        ESP_LOGE(E22TAG, "Error al enviar el comando SetStandby");
        return false;
    }

    return true;
}

bool E22::processStatus(uint8_t msg)
{
    bool rc = true;
    printf("Chip Mode: ");
    switch (msg & 0x70)
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
    switch (msg & 0x0E)
    {
    case 0x00:
        printf("Reserved");
        break;

    case 0x04:
        printf("Data is available to host");
        break;

    case 0x06:
        printf("Command timeout");
        rc = false;
        break;

    case 0x08:
        printf("Command processing error");
        rc = false;
        break;

    case 0x0A:
        printf("Failure to execute command");
        rc = false;
        break;

    case 0x0C:
        printf("Command TX done");
        break;

    default:
        printf("Undefined");
        break;
    }
    printf("\r\n");
    printf("Valor de la respuesta status: 0x%X\r\n", msg & 0xff);

    return rc;
}

bool E22::isBusy(void)
{
    IO &io = IO::getInstance();

    if (io.GetLevel((gpio_num_t)NRST_E22_PIN))
    {
        return true;
    }
    return false;
}

bool E22::resetOff(void)
{
    IO &io = IO::getInstance();

    if (io.SetLevel((gpio_num_t)NRST_E22_PIN, 1))
    {
        return true;
    }
    return false;
}

bool E22::resetOn(void)
{
    IO &io = IO::getInstance();

    if (io.SetLevel((gpio_num_t)NRST_E22_PIN, 0))
    {
        return true;
    }
    return false;
}

void E22::processCmd(void)
{
    uint8_t cmdToProcess = 0;
    if (xQueueReceive(xE22CmdQueue, &(cmdToProcess), 0) == pdPASS)
    {
        /* Proceso el mensaje de la tarea que lo solicite */
    }
}
