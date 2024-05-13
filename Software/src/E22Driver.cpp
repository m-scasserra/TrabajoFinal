#include "E22Driver.h"
#include "DeviceStatus.h"

TaskHandle_t E22::E22TaskHandle = NULL;
SemaphoreHandle_t E22::xE22InterruptSempahore = NULL;
SemaphoreHandle_t E22::xE22ResponseWaitSempahore = NULL;
uint8_t E22::s_rssiInst = 0;
uint32_t E22::s_msgTimeoutms = DEFAULT_MSG_TIMEOUT_MS;
E22::LoraPacketParams_t E22::s_packetParams;
E22::ModulationParameters_t E22::s_modulationParams;
E22::PacketType_t E22::s_packetType;
uint8_t E22::s_PayloadLenghtRx = 0;
uint8_t E22::s_RxBufferAddr = 0;
uint8_t E22::s_TxBufferAddr = 0;
E22::IRQReg_t E22::IRQReg;
bool E22::processIRQ = false;
bool E22::PacketReceived = false;

void IRAM_ATTR E22::E22ISRHandler(void)
{
    gpio_set_intr_type((gpio_num_t)DIO1_E22_PIN, GPIO_INTR_DISABLE);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xE22InterruptSempahore, &xHigherPriorityTaskWoken);
}

void E22::E22Task(void *pvParameters)
{
    E22 *e22 = (E22 *)pvParameters;

    ESP_LOGI(E22TAG, "Apagando E22.");
    e22->resetOn();
    vTaskDelay(pdMS_TO_TICKS(1000));

    while (e22->isBusy())
    {
        ESP_LOGI(E22TAG, "Esperando a que el E22 se apague.");
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(E22TAG, "E22 apagado, lo prendo.");
    e22->resetOff();

    if (!e22->checkDeviceConnection())
    {
        ESP_LOGE(E22TAG, "E22 no conectado.");
        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    ESP_LOGI(E22TAG, "E22 conectado.");

    if (!e22->antennaMismatchCorrection())
    {
        ESP_LOGE(E22TAG, "E22 no se puede corregir la antena.");
    }

    ESP_LOGI(E22TAG, "E22 listo para enviar y recibir.");

    while (1)
    {
        if (!e22->isBusy())
        {
            e22->processInterrupt();

            e22->processCmd();
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Espero al siguiente CMD
    }
}

bool E22::Begin(void)
{
    SPI &spi = SPI::getInstance();

    // Configuro el IO necesario para el control del E22
    E22IOInit();

    InterruptInit();

    // Seteo todos los valores de las variables a 0
    memset(&IRQReg, 0, sizeof(IRQReg_t));
    memset(&s_packetParams, 0, sizeof(LoraPacketParams_t));
    memset(&s_modulationParams, 0, sizeof(ModulationParameters_t));

    // Creo la queue de mensajes hacia el E22
    xE22CmdQueue = xQueueCreate(MAX_E22_CMD_QUEUE, sizeof(E22Command_t));
    // Creo la queue de mensajes desde el E22
    xE22ResponseQueue = xQueueCreate(MAX_E22_CMD_QUEUE, sizeof(E22Response_t));

    // Creo el semaphore de interrupcciones
    xE22InterruptSempahore = xSemaphoreCreateBinary();
    xSemaphoreGive(xE22InterruptSempahore);
    xSemaphoreTake(xE22InterruptSempahore, 0);

    // Creo el semaphore de respuestas de los mensajes
    xE22ResponseWaitSempahore = xSemaphoreCreateBinary();
    xSemaphoreGive(xE22ResponseWaitSempahore);
    xSemaphoreTake(xE22ResponseWaitSempahore, 0);

    if (xE22CmdQueue == NULL)
    {
        /* Queue was not created and must not be used. */
    }

    // Configuracion del bus SPI
    spi_bus_config_t SPIBusCfg;
    memset(&SPIBusCfg, 0, sizeof(spi_bus_config_t));   // Seteo el SPIBus en 0 para asegurarme que ningun parametro esta pre iniciado
    SPIBusCfg.mosi_io_num = SPI_MOSI_PIN;              // Pin del MOSI
    SPIBusCfg.miso_io_num = SPI_MISO_PIN;              // Pin del MiSO
    SPIBusCfg.sclk_io_num = SPI_SCK_PIN;               // Pin del SCK
    SPIBusCfg.quadhd_io_num = -1;                      // No se usa el Write Protect
    SPIBusCfg.quadwp_io_num = -1;                      // No se usa el Hold
    SPIBusCfg.data4_io_num = -1;                       // No se usa el spi Data 4 signal porque no estamos en OSPI
    SPIBusCfg.data5_io_num = -1;                       // No se usa el spi Data 5 signal porque no estamos en OSPI
    SPIBusCfg.data6_io_num = -1;                       // No se usa el spi Data 6 signal porque no estamos en OSPI
    SPIBusCfg.data7_io_num = -1;                       // No se usa el spi Data 7 signal porque no estamos en OSPI
    SPIBusCfg.max_transfer_sz = SPI_MAX_TRANSFER;      // Maximo tamaño de transferencia
    SPIBusCfg.flags = SPICOMMON_BUSFLAG_MASTER;        // Flag para indicar que el µC es el master en SPI
    SPIBusCfg.isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO; // CPU que se encarga de las interrupciones, en el ESP32-C3 hay un solo CPU
    SPIBusCfg.intr_flags = SPI_INTR_BUS_FLAGS;         // Flags de configuracion para el bus SPI

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

bool E22::processInterrupt(void)
{
    IO &io = IO::getInstance();

    if (xSemaphoreTake(xE22InterruptSempahore, 0))
    {
        getIRQStatusForInterrupt();

        ESP_LOGI(E22TAG, "IRQReg.txDone: 0x%X", IRQReg.txDone);
        ESP_LOGI(E22TAG, "IRQReg.rxDone: 0x%X", IRQReg.rxDone);
        ESP_LOGI(E22TAG, "IRQReg.preambleDetected: 0x%X", IRQReg.preambleDetected);
        ESP_LOGI(E22TAG, "IRQReg.syncWordValid: 0x%X", IRQReg.syncWordValid);
        ESP_LOGI(E22TAG, "IRQReg.headerValid: 0x%X", IRQReg.headerValid);
        ESP_LOGI(E22TAG, "IRQReg.headerErr: 0x%X", IRQReg.headerErr);
        ESP_LOGI(E22TAG, "IRQReg.crcErr: 0x%X", IRQReg.crcErr);
        ESP_LOGI(E22TAG, "IRQReg.cadDone: 0x%X", IRQReg.cadDone);
        ESP_LOGI(E22TAG, "IRQReg.cadDetected: 0x%X", IRQReg.cadDetected);
        ESP_LOGI(E22TAG, "IRQReg.timeout: 0x%X", IRQReg.timeout);
        ESP_LOGI(E22TAG, "IRQReg.lrFhssHop: 0x%X", IRQReg.lrFhssHop);

        if (IRQReg.rxDone)
        {
            io.SetLevel((gpio_num_t)RX_EN_E22_PIN, IO_LOW);
            getRxBufferStatusForInterrupt();
            PacketReceived = true;
        }
        if (IRQReg.txDone)
        {
            io.SetLevel((gpio_num_t)TX_EN_E22_PIN, IO_LOW);
        }
        if (IRQReg.preambleDetected)
        {
        }
        if (IRQReg.syncWordValid)
        {
        }
        if (IRQReg.headerValid)
        {
        }
        if (IRQReg.headerErr)
        {
            io.SetLevel((gpio_num_t)RX_EN_E22_PIN, IO_LOW);
        }
        if (IRQReg.crcErr)
        {
            io.SetLevel((gpio_num_t)RX_EN_E22_PIN, IO_LOW);
        }
        if (IRQReg.cadDone)
        {
        }
        if (IRQReg.cadDetected)
        {
        }
        if (IRQReg.timeout)
        {
            io.SetLevel((gpio_num_t)RX_EN_E22_PIN, IO_LOW);
            io.SetLevel((gpio_num_t)TX_EN_E22_PIN, IO_LOW);
        }
        if (IRQReg.lrFhssHop)
        {
        }

        clearIrqStatus(IRQREGFULL);

        return true;
    }
    return false;
}

bool E22::E22IOInit(void)
{
    IO &io = IO::getInstance();
    // Configuracion inicial y estado inicial del pin RXEN del E22
    if (!io.SetConfig(RX_EN_E22_PIN, GPIO_MODE_OUTPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_ENABLE, GPIO_INTR_DISABLE))
    {
        return false;
    };
    if (io.SetLevel((gpio_num_t)RX_EN_E22_PIN, IO_LOW))
    {
        return false;
    }

    // Configuracion inicial y estado inicial del pin TXEN del E22
    if (!io.SetConfig(TX_EN_E22_PIN, GPIO_MODE_OUTPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_ENABLE, GPIO_INTR_DISABLE))
    {
        return false;
    };
    if (io.SetLevel((gpio_num_t)TX_EN_E22_PIN, IO_LOW))
    {
        return false;
    }

    // Configuracion inicial y estado inicial del pin NRST del E22
    if (!io.SetConfig(NRST_E22_PIN, GPIO_MODE_OUTPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_DISABLE))
    {
        return false;
    };
    if (io.SetLevel((gpio_num_t)NRST_E22_PIN, IO_LOW))
    {
        return false;
    }

    // Configuracion inicial del pin BUSY del E22
    if (!io.SetConfig(BUSY_E22_PIN, GPIO_MODE_INPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_DISABLE))
    {
        return false;
    };

    return true;
}

bool E22::InterruptInit(void)
{
    gpio_config_t interrputConf;
    memset(&interrputConf, 0, sizeof(interrputConf));

    // Interrupt disable
    interrputConf.intr_type = GPIO_INTR_DISABLE;
    // Bit mask of the pin DIO1_E22_PIN
    interrputConf.pin_bit_mask = (1ULL << DIO1_E22_PIN);
    // Set as input mode
    interrputConf.mode = GPIO_MODE_INPUT;
    interrputConf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config(&interrputConf);

    // Install gpio isr service on the lowest priority
    if (gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1) != ESP_OK)
    {
        return false;
    }
    // Hook isr handler for the DIO1 pin of the E22
    if (gpio_isr_handler_add((gpio_num_t)DIO1_E22_PIN, (gpio_isr_t)E22ISRHandler, nullptr) != ESP_OK)
    {
        return false;
    }

    return true;
}

bool E22::getStatus(void)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_GetStatus;
    command.paramCount = 2;
    command.responsesCount = 2;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando getStatus a la queue.");
    return false;
}

bool E22::getRssiInst(void)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_GetRssiInst;
    command.paramCount = 3;
    command.params.paramsArray[0] = 0x00;
    command.params.paramsArray[1] = 0x00;
    command.responsesCount = 3;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando getRssiInst a la queue.");
    return false;
}

bool E22::setBufferBaseAddress(uint8_t TxBaseAddr, uint8_t RxBaseAddr)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_SetBufferBaseAddress;
    command.paramCount = 3;
    command.params.paramsArray[0] = TxBaseAddr;
    command.params.paramsArray[1] = RxBaseAddr;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando setBufferBaseAddress a la queue.");
    return false;
}

bool E22::setRx(uint32_t Timeout)
{
    if (Timeout > 0xFFFFFF)
    {
        ESP_LOGE(E22TAG, "Timeout invalido, tiene que ser un valor entre 0x000000 y 0xFFFFFF.");
        return false;
    }
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    uint32_t TimeoutBits = Timeout << 6;
    command.commandCode = E22_CMD_SetRx;
    command.paramCount = 4;
    command.params.paramsArray[0] = (uint8_t)(TimeoutBits >> 16);
    command.params.paramsArray[1] = (uint8_t)(TimeoutBits >> 8);
    command.params.paramsArray[2] = (uint8_t)(TimeoutBits >> 0);

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando setRx a la queue.");
    return false;
}

bool E22::setStandBy(StdByMode_t mode)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_SetStandBy;
    command.paramCount = 2;
    command.params.paramsArray[0] = (uint8_t)mode;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando setStandBy a la queue.");
    return false;
}

void E22::processStatus(uint8_t msg)
{
    // Create a buffer to store the formatted string
    char chipMode[10];      // Allocate enough space for hex, spaces and null terminator
    char commandStatus[27]; // Allocate enough space for hex, spaces and null terminator
    memset(chipMode, 0, sizeof(chipMode));

    switch (msg & 0x70)
    {
    case 0x00:
    {
        sprintf(chipMode, "Unused");
        break;
    }

    case 0x20:
    {
        sprintf(chipMode, "STBY_RC");
        break;
    }

    case 0x30:
    {
        sprintf(chipMode, "STBY_XOSC");
        break;
    }

    case 0x40:
    {
        sprintf(chipMode, "FS");
        break;
    }

    case 0x50:
    {
        sprintf(chipMode, "RX");
        break;
    }

    case 0x60:
    {
        sprintf(chipMode, "TX");
        break;
    }

    default:
    {
        sprintf(chipMode, "Undefined");
        break;
    }
    }

    switch (msg & 0x0E)
    {
    case 0x00:
    {
        sprintf(commandStatus, "Reserved");
        break;
    }

    case 0x04:
    {
        sprintf(commandStatus, "Data is available to host");
        break;
    }

    case 0x06:
    {
        sprintf(commandStatus, "Command timeout");
        break;
    }

    case 0x08:
    {
        sprintf(commandStatus, "Command processing error");
        break;
    }

    case 0x0A:
    {
        sprintf(commandStatus, "Failure to execute command");
        break;
    }

    case 0x0C:
    {
        sprintf(commandStatus, "Command TX done");
        break;
    }

    default:
    {
        sprintf(commandStatus, "Undefined");
        break;
    }
    }
    ESP_LOGD(E22TAG, "Chip Mode: %s Command Status: %s Status Value: 0x%02X\r\n", chipMode, commandStatus, msg);
}

bool E22::isBusy(void)
{
    IO &io = IO::getInstance();

    if (io.GetLevel((gpio_num_t)BUSY_E22_PIN))
    {
        return true;
    }
    return false;
}

bool E22::resetOff(void)
{
    IO &io = IO::getInstance();

    if (io.SetLevel((gpio_num_t)NRST_E22_PIN, IO_HIGH))
    {
        return true;
    }
    return false;
}

bool E22::resetOn(void)
{
    IO &io = IO::getInstance();

    if (io.SetLevel((gpio_num_t)NRST_E22_PIN, IO_LOW))
    {
        return true;
    }
    return false;
}

bool E22::processCmd(void)
{
    SPI &spi = SPI::getInstance();
    E22Command_t cmdToProcess;
    E22Response_t response;
    uint8_t TxBuffer[MAX_CMD_PARAMS + 1];
    uint8_t RxBuffer[MAX_CMD_PARAMS + 1];

    memset(&cmdToProcess, 0, sizeof(E22Command_t));
    memset(&response, 0, sizeof(E22Response_t));
    memset(&TxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));
    memset(&RxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));

    if (xQueueReceive(xE22CmdQueue, &(cmdToProcess), 0) == pdPASS)
    {
        /* Proceso el mensaje de la tarea que lo solicite */
        ESP_LOGE(E22TAG, "Procesando comando %d", cmdToProcess.commandCode);

        switch (cmdToProcess.commandCode)
        {
        case E22_CMD_SetSleep:
        {
        }
        break;

        case E22_CMD_SetStandBy:
        {
            TxBuffer[0] = E22_OpCode_SetStandby;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando SetStandby");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_SetFs:
        {
        }
        break;

        case E22_CMD_SetTx:
        {
            TxBuffer[0] = E22_OpCode_SetTX;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];
            TxBuffer[3] = cmdToProcess.params.paramsArray[2];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando SetTx");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_SetRx:
        {
            TxBuffer[0] = E22_OpCode_SetRX;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];
            TxBuffer[3] = cmdToProcess.params.paramsArray[2];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando SetRx");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_StopTimerOnPreamble:
        {
        }
        break;

        case E22_CMD_SetRxDutyCycle:
        {
        }
        break;

        case E22_CMD_SetCad:
        {
        }
        break;

        case E22_CMD_SetTxContinousWave:
        {
        }
        break;

        case E22_CMD_SetTxInfinitePreamble:
        {
        }
        break;

        case E22_CMD_SetRegulatorMode:
        {
        }
        break;

        case E22_CMD_Calibrate:
        {
            TxBuffer[0] = E22_OpCode_Calibrate;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando Calibrate al spi.");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_CalibrateImage:
        {
            TxBuffer[0] = E22_OpCode_CalibrateImage;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando CalibrateImage al spi.");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_SetPaConfig:
        {
            TxBuffer[0] = E22_OpCode_SetPaConfig;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];
            TxBuffer[3] = cmdToProcess.params.paramsArray[2];
            TxBuffer[4] = cmdToProcess.params.paramsArray[3];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando SetPaConfig al spi.");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_SetRxTxFallbackMode:
        {
        }
        break;

        case E22_CMD_WriteRegister:
        {
            TxBuffer[0] = E22_OpCode_WriteRegister;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];
            TxBuffer[3] = cmdToProcess.params.paramsArray[2];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando WriteRegister al spi.");
                return false;
            }
            ESP_LOGE(E22TAG, "WriteRegister: %X", TxBuffer[3]);
            return true;
        }
        break;

        case E22_CMD_ReadRegister:
        {
            TxBuffer[0] = E22_OpCode_ReadRegister;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount, RxBuffer, cmdToProcess.responsesCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando ReadRegister al spi.");
                return false;
            }
            processStatus(RxBuffer[1]);
            processStatus(RxBuffer[2]);
            processStatus(RxBuffer[3]);
            response.commandCode = E22_CMD_ReadRegister;
            response.responsesCount = 1;
            response.responses.responsesArray[0] = RxBuffer[4];

            if (xQueueSend(xE22ResponseQueue, (void *)&response, 0) == pdPASS)
            {
                return true;
            }

            ESP_LOGE(E22TAG, "Error al enviar el comando ReadRegister a la queue de respuestas.");

            return false;
        }
        break;

        case E22_CMD_WriteBuffer:
        {
            TxBuffer[0] = E22_OpCode_WriteBuffer;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando WriteBuffer al spi.");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_ReadBuffer:
        {
            TxBuffer[0] = E22_OpCode_ReadBuffer;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount, RxBuffer, cmdToProcess.responsesCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando ReadBuffer al spi.");
                return false;
            }
            processStatus(RxBuffer[1]);
            processStatus(RxBuffer[2]);
            response.commandCode = E22_CMD_ReadBuffer;
            response.responsesCount = 1;
            response.responses.responsesArray[0] = RxBuffer[3];

            if (xQueueSend(xE22ResponseQueue, (void *)&response, 0) == pdPASS)
            {
                return true;
            }

            ESP_LOGE(E22TAG, "Error al enviar el comando ReadBuffer a la queue de respuestas.");

            return false;
        }
        break;

        case E22_CMD_SetDioIrqParams:
        {
            TxBuffer[0] = E22_OpCode_SetDioIrqParams;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];
            TxBuffer[3] = cmdToProcess.params.paramsArray[2];
            TxBuffer[4] = cmdToProcess.params.paramsArray[3];
            TxBuffer[5] = cmdToProcess.params.paramsArray[4];
            TxBuffer[6] = cmdToProcess.params.paramsArray[5];
            TxBuffer[7] = cmdToProcess.params.paramsArray[6];
            TxBuffer[8] = cmdToProcess.params.paramsArray[7];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando SetDioIrqParams al spi.");
                return false;
            }
        }
        break;

        case E22_CMD_GetIrqStatus:
        {
            TxBuffer[0] = E22_OpCode_GetIrqStatus;

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount, RxBuffer, cmdToProcess.responsesCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando GetIrqStatus al spi.");
                return false;
            }
            processStatus(RxBuffer[1]);
            response.commandCode = E22_CMD_GetIrqStatus;
            response.responsesCount = 2;
            response.responses.responsesArray[0] = RxBuffer[2];
            response.responses.responsesArray[1] = RxBuffer[3];

            if (xQueueSend(xE22ResponseQueue, (void *)&response, 0) == pdPASS)
            {
                return true;
            }

            ESP_LOGE(E22TAG, "Error al enviar el comando GetIrqStatus a la queue de respuestas.");

            return false;
        }
        break;

        case E22_CMD_ClearIrqStatus:
        {
            TxBuffer[0] = E22_OpCode_ClearIrqStatus;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando ClearIrqStatus al spi.");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_SetDIO2AsRfSwitchCtrl:
        {
        }
        break;

        case E22_CMD_SetDIO3asTcxoCtrl:
        {
            TxBuffer[0] = E22_OpCode_SetDIO3AsTcxoCtrl;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];
            TxBuffer[3] = cmdToProcess.params.paramsArray[2];
            TxBuffer[4] = cmdToProcess.params.paramsArray[3];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando SetDIO3AsTcxoCtrl al spi.");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_SetRfFrequency:
        {
            TxBuffer[0] = E22_OpCode_SetRfFrequency;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];
            TxBuffer[3] = cmdToProcess.params.paramsArray[2];
            TxBuffer[4] = cmdToProcess.params.paramsArray[3];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando SetRfFrequency al spi.");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_SetPacketType:
        {
            TxBuffer[0] = E22_OpCode_SetPacketType;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando setPacketType al spi.");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_GetPacketType:
        {
            TxBuffer[0] = E22_OpCode_GetPacketType;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount, RxBuffer, cmdToProcess.responsesCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando getPacketType al spi.");
                return false;
            }
            processStatus(RxBuffer[1]);
            response.commandCode = E22_CMD_GetPacketType;
            response.responsesCount = 1;
            response.responses.responsesArray[0] = RxBuffer[2];

            if (xQueueSend(xE22ResponseQueue, (void *)&response, 0) == pdPASS)
            {
                return true;
            }

            ESP_LOGE(E22TAG, "Error al enviar el comando getPacketType a la queue de respuestas.");

            return false;
        }
        break;

        case E22_CMD_SetTxParams:
        {
            TxBuffer[0] = E22_OpCode_SetTxParams;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando SetTxParams al spi.");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_SetModulationParams:
        {
            TxBuffer[0] = E22_OpCode_SetModulationParams;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];
            TxBuffer[3] = cmdToProcess.params.paramsArray[2];
            TxBuffer[4] = cmdToProcess.params.paramsArray[3];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando SetModulationParams al spi.");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_SetPacketParams:
        {
            TxBuffer[0] = E22_OpCode_SetPacketParams;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];
            TxBuffer[3] = cmdToProcess.params.paramsArray[2];
            TxBuffer[4] = cmdToProcess.params.paramsArray[3];
            TxBuffer[5] = cmdToProcess.params.paramsArray[4];
            TxBuffer[6] = cmdToProcess.params.paramsArray[5];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando SetPacketParams al spi.");
                return false;
            }

            s_packetParams.preambleLength = (uint16_t)((cmdToProcess.params.paramsArray[0] << 8) | cmdToProcess.params.paramsArray[1]);
            s_packetParams.headerType = (E22::PacketHeaderType_t)cmdToProcess.params.paramsArray[2];
            s_packetParams.payloadLength = cmdToProcess.params.paramsArray[3];
            s_packetParams.crcType = cmdToProcess.params.paramsArray[4];
            s_packetParams.iqType = (E22::PacketIQType_t)cmdToProcess.params.paramsArray[5];

            return true;
        }
        break;

        case E22_CMD_SetCadParams:
        {
        }
        break;

        case E22_CMD_SetBufferBaseAddress:
        {
            TxBuffer[0] = E22_OpCode_SetBufferBaseAddress;
            TxBuffer[1] = cmdToProcess.params.paramsArray[0];
            TxBuffer[2] = cmdToProcess.params.paramsArray[1];

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando setBufferBaseAddress al spi.");
                return false;
            }

            return true;
        }
        break;

        case E22_CMD_SetLoRaSymbNumTimeout:
        {
        }
        break;

        case E22_CMD_GetStatus:
        {
            TxBuffer[0] = E22_OpCode_GetStatus;
            TxBuffer[1] = 0x00;

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount, RxBuffer, cmdToProcess.responsesCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando getStatus");
                return false;
            }
            processStatus(RxBuffer[1]);

            return true;
        }
        break;

        case E22_CMD_GetRssiInst:
        {
            TxBuffer[0] = E22_OpCode_GetRssiInst;
            TxBuffer[1] = 0x00;
            TxBuffer[2] = 0x00;

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount, RxBuffer, cmdToProcess.responsesCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando getRssiInst");
                return false;
            }

            processStatus(RxBuffer[1]);
            s_rssiInst = RxBuffer[2] / 2;
            return true;
        }
        break;

        case E22_CMD_GetRxBufferStatus:
        {
            TxBuffer[0] = E22_OpCode_GetRxBufferStatus;
            TxBuffer[1] = 0x00;
            TxBuffer[2] = 0x00;
            TxBuffer[3] = 0x00;

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount, RxBuffer, cmdToProcess.responsesCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando GetRxBufferStatus");
                return false;
            }

            processStatus(RxBuffer[1]);
            response.commandCode = E22_CMD_GetRxBufferStatus;
            response.responsesCount = 2;
            response.responses.responsesArray[0] = RxBuffer[2];
            response.responses.responsesArray[1] = RxBuffer[3];

            if (xQueueSend(xE22ResponseQueue, (void *)&response, 0) == pdPASS)
            {
                return true;
            }

            ESP_LOGE(E22TAG, "Error al enviar el comando GetRxBufferStatus a la queue de respuestas.");

            return false;
        }
        break;

        case E22_CMD_GetPacketStatus:
        {
            TxBuffer[0] = E22_OpCode_GetPacketStatus;
            TxBuffer[1] = 0x00;
            TxBuffer[2] = 0x00;
            TxBuffer[3] = 0x00;
            TxBuffer[4] = 0x00;

            if (!spi.SendMessage(TxBuffer, cmdToProcess.paramCount, RxBuffer, cmdToProcess.responsesCount))
            {
                ESP_LOGE(E22TAG, "Error al enviar el comando GetPacketStatus");
                return false;
            }

            processStatus(RxBuffer[1]);
            response.commandCode = E22_CMD_GetPacketStatus;
            response.responsesCount = 3;
            response.responses.responsesArray[0] = RxBuffer[2];
            response.responses.responsesArray[1] = RxBuffer[3];
            response.responses.responsesArray[2] = RxBuffer[4];

            if (xQueueSend(xE22ResponseQueue, (void *)&response, 0) == pdPASS)
            {
                return true;
            }

            ESP_LOGE(E22TAG, "Error al enviar el comando GetPacketStatus a la queue de respuestas.");

            return false;
        }
        break;

        case E22_CMD_GetDeviceErrors:
        {
        }
        break;

        case E22_CMD_ClearDeviceErrors:
        {
        }
        break;

        case E22_CMD_GetStats:
        {
        }
        break;

        case E22_CMD_ResetStats:
        {
        }
        break;

        default:
        {
        }
        break;
        }
    }
    return true; // TODO
}

bool E22::processResponse(void)
{
    return true; // TODO
}

bool E22::writeRegister(E22_Reg_Addr addr, uint8_t dataIn)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_WriteRegister;
    command.paramCount = 4;
    command.params.paramsArray[0] = (uint8_t)(addr >> 8);
    command.params.paramsArray[1] = (uint8_t)(addr >> 0);
    command.params.paramsArray[2] = (uint8_t)dataIn;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando writeRegister a la queue.");
    return false;
}

bool E22::readRegister(E22_Reg_Addr addr, uint8_t *dataOut)
{
    E22Command_t command;
    E22Response_t response;
    memset(&command, 0, sizeof(E22Command_t));
    memset(&response, 0, sizeof(E22Response_t));
    command.commandCode = E22_CMD_ReadRegister;
    command.paramCount = 5;
    command.params.paramsArray[0] = (uint8_t)(addr >> 8);
    command.params.paramsArray[1] = (uint8_t)(addr >> 0);
    command.hasResponse = true;
    command.responsesCount = 5;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        if (xQueueReceive(xE22ResponseQueue, &(response), pdMS_TO_TICKS(10000)) == pdPASS)
        {
            *dataOut = response.responses.responsesArray[0];
            return true;
        }
        ESP_LOGE(E22TAG, "Error al leer el comando readRegister a la queue.");
        return false;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando readRegister a la queue.");
    return false;
}

bool E22::writeBuffer(uint8_t offset, uint8_t dataIn)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_WriteBuffer;
    command.paramCount = 3;
    command.params.paramsArray[0] = offset;
    command.params.paramsArray[1] = dataIn;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando writeBuffer a la queue.");
    return false;
}

bool E22::readBuffer(uint8_t offset, uint8_t *dataOut)
{
    E22Command_t command;
    E22Response_t response;
    memset(&command, 0, sizeof(E22Command_t));
    memset(&response, 0, sizeof(E22Response_t));
    command.commandCode = E22_CMD_ReadBuffer;
    command.paramCount = 4;
    command.params.paramsArray[0] = offset;
    command.hasResponse = true;
    command.responsesCount = 4;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        if (xQueueReceive(xE22ResponseQueue, &(response), pdMS_TO_TICKS(10000)) == pdPASS)
        {
            *dataOut = response.responses.responsesArray[0];
            return true;
        }
        ESP_LOGE(E22TAG, "Error al leer el comando readBuffer a la queue.");
        return false;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando readBuffer a la queue.");
    return false;
}

bool E22::setPacketType(PacketType_t _packetType)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_SetPacketType;
    command.paramCount = 2;
    command.params.paramsArray[0] = _packetType;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        s_packetType = _packetType;
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando setPacketType a la queue.");
    return false;
}

bool E22::getPacketType(PacketType_t *_packetType)
{
    E22Command_t command;
    E22Response_t response;
    memset(&command, 0, sizeof(E22Command_t));
    memset(&response, 0, sizeof(E22Response_t));
    command.commandCode = E22_CMD_GetPacketType;
    command.paramCount = 3;
    command.hasResponse = true;
    command.responsesCount = 3;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        if (xQueueReceive(xE22ResponseQueue, &(response), pdMS_TO_TICKS(10000)) == pdPASS)
        {
            *_packetType = (PacketType_t)response.responses.responsesArray[0];
            s_packetType = (PacketType_t)response.responses.responsesArray[0];
            return true;
        }
        ESP_LOGE(E22TAG, "Error al leer el comando readRegister a la queue.");
        return false;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando getPacketType a la queue.");
    return false;
}

bool E22::setDIO3asTCXOCtrl(E22::tcxoVoltage_t voltage, uint32_t delayms)
{
    if (delayms > 0xFFFFFF)
    {
        ESP_LOGE(E22TAG, "Timeout invalido, tiene que ser un valor entre 0x000000 y 0xFFFFFF.");
        return false;
    }
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    uint32_t delayBits = delayms << 6;
    command.commandCode = E22_CMD_SetDIO3asTcxoCtrl;
    command.paramCount = 5;
    command.params.paramsArray[0] = (uint8_t)voltage;
    command.params.paramsArray[1] = (uint8_t)(delayBits >> 16);
    command.params.paramsArray[2] = (uint8_t)(delayBits >> 8);
    command.params.paramsArray[3] = (uint8_t)(delayBits >> 0);

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando SetDIO3asTcxoCtrl a la queue.");
    return false;
}

bool E22::setXtalCap(uint8_t XTA, uint8_t XTB)
{
    Calibrate_t calibrationsToDo;
    memset(&calibrationsToDo, 0, sizeof(Calibrate_t));
    calibrationsToDo.RC64kCalibration = true;
    calibrationsToDo.RC13MCalibration = true;
    calibrationsToDo.PLLCalibration = true;
    calibrationsToDo.ADCPulseCalibration = true;
    calibrationsToDo.ADCBulkNCalibration = true;
    calibrationsToDo.ADCBulkPCalibration = true;
    calibrationsToDo.ImageCalibration = true;

    if (!setStandBy(STDBY_XOSC))
    {
        return false;
    }

    if (!writeRegister(E22_Reg_XTATrim, XTA))
    {
        return false;
    }

    if (!writeRegister(E22_Reg_XTBTrim, XTB))
    {
        return false;
    }

    if (!setStandBy(STDBY_RC))
    {
        return false;
    }

    if (!calibrate(calibrationsToDo))
    {
        return false;
    }

    return true;
}

bool E22::calibrate(Calibrate_t calibrationsToDo)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_Calibrate;
    command.paramCount = 2;
    command.params.paramsArray[0] = calibrationsToMask(calibrationsToDo);

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando Calibrate a la queue.");
    return false;
}

uint8_t E22::calibrationsToMask(Calibrate_t calibrations)
{
    uint8_t aux = 0;
    if (calibrations.RC64kCalibration)
    {
        aux = aux | 0x01;
    }
    if (calibrations.RC13MCalibration)
    {
        aux = aux | 0x02;
    }
    if (calibrations.PLLCalibration)
    {
        aux = aux | 0x04;
    }
    if (calibrations.ADCPulseCalibration)
    {
        aux = aux | 0x08;
    }
    if (calibrations.ADCBulkNCalibration)
    {
        aux = aux | 0x10;
    }
    if (calibrations.ADCBulkPCalibration)
    {
        aux = aux | 0x20;
    }
    if (calibrations.ImageCalibration)
    {
        aux = aux | 0x40;
    }
    return aux;
}

E22::Calibrate_t E22::calibrationsFromMask(uint8_t calibrationsMask)
{
    E22::Calibrate_t calibrations;
    (calibrationsMask & 0x01) ? calibrations.RC64kCalibration = true : calibrations.RC64kCalibration = false;
    (calibrationsMask & 0x02) ? calibrations.RC13MCalibration = true : calibrations.RC13MCalibration = false;
    (calibrationsMask & 0x04) ? calibrations.PLLCalibration = true : calibrations.PLLCalibration = false;
    (calibrationsMask & 0x08) ? calibrations.ADCPulseCalibration = true : calibrations.ADCPulseCalibration = false;
    (calibrationsMask & 0x10) ? calibrations.ADCBulkNCalibration = true : calibrations.ADCBulkNCalibration = false;
    (calibrationsMask & 0x20) ? calibrations.ADCBulkPCalibration = true : calibrations.ADCBulkPCalibration = false;
    (calibrationsMask & 0x40) ? calibrations.ImageCalibration = true : calibrations.ImageCalibration = false;
    return calibrations;
}

bool E22::calibrateImage(ImageCalibrationFreq_t frequency)
{
    E22Command_t command;
    uint8_t freq1 = 0;
    uint8_t freq2 = 0;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_CalibrateImage;
    command.paramCount = 3;

    switch (frequency)
    {
    case FREQ_430_440:
        freq1 = 0x6B;
        freq2 = 0x6F;
        break;

    case FREQ_470_510:
        freq1 = 0x75;
        freq2 = 0x81;
        break;

    case FREQ_779_787:
        freq1 = 0xC1;
        freq2 = 0xC5;
        break;

    case FREQ_863_870:
        freq1 = 0xD7;
        freq2 = 0xD8;
        break;

    case FREQ_902_928:
        freq1 = 0xE1;
        freq2 = 0xE9;
        break;

    default:
        ESP_LOGE(E22TAG, "Error al enviar el comando CalibrateImage a la queue.");
        return false;
        break;
    }

    command.params.paramsArray[0] = freq1;
    command.params.paramsArray[1] = freq2;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando CalibrateImage a la queue.");
    return false;
}

bool E22::setFrequency(uint32_t frequency)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_SetRfFrequency;
    command.paramCount = 5;

    uint32_t RFfreq = ((uint64_t)frequency << SX126X_RF_FREQUENCY_SHIFT) / SX126X_RF_FREQUENCY_XTAL;
    command.params.paramsArray[0] = (uint8_t)(RFfreq >> 24);
    command.params.paramsArray[1] = (uint8_t)(RFfreq >> 16);
    command.params.paramsArray[2] = (uint8_t)(RFfreq >> 8);
    command.params.paramsArray[3] = (uint8_t)(RFfreq >> 0);

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando setFrequency a la queue.");
    return false;
}

bool E22::setFrequencyAndCalibrate(uint32_t frequency)
{
    // Perform image calibration BEFORE set frequency
    if (frequency < 446000000) // 430 - 440 Mhz
    {
        if (!calibrateImage(FREQ_430_440))
        {
            return false;
        }
    }
    else if (frequency < 734000000) // 470 - 510 Mhz
    {
        if (!calibrateImage(FREQ_470_510))
        {
            return false;
        }
    }
    else if (frequency < 828000000) // 779 - 787 Mhz
    {
        if (!calibrateImage(FREQ_779_787))
        {
            return false;
        }
    }
    else if (frequency < 877000000) // 863 - 870 Mhz
    {
        if (!calibrateImage(FREQ_863_870))
        {
            return false;
        }
    }
    else if (frequency < 1100000000) // 902 - 928 Mhz
    {
        if (!calibrateImage(FREQ_902_928))
        {
            return false;
        }
    }

    if (!setFrequency(frequency))
    {
        return false;
    }

    return true;
}

bool E22::setRxGain(RxGain_t gain)
{
    if (!writeRegister(E22_Reg_RxGain, gain))
    {
        return false;
    }
    return true;
}

bool E22::setModulationParams(ModulationParameters_t modulation)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_SetModulationParams;
    command.paramCount = 9;

    command.params.paramsArray[0] = (uint8_t)modulation.spreadingFactor;
    command.params.paramsArray[1] = (uint8_t)modulation.bandwidth;
    command.params.paramsArray[2] = (uint8_t)modulation.codingRate;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        s_modulationParams = modulation;
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando SetModulationParams a la queue.");
    return false;
}

bool E22::setPacketParams(LoraPacketParams_t packetParams)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_SetPacketParams;
    command.paramCount = 10;
    command.params.paramsArray[0] = (uint8_t)(packetParams.preambleLength >> 8);
    command.params.paramsArray[1] = (uint8_t)(packetParams.preambleLength >> 0);
    command.params.paramsArray[2] = (uint8_t)packetParams.headerType;
    command.params.paramsArray[3] = (uint8_t)packetParams.payloadLength;
    command.params.paramsArray[4] = (uint8_t)packetParams.crcType;
    command.params.paramsArray[5] = (uint8_t)packetParams.iqType;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando SetPacketParams a la queue.");
    return false;
}

bool E22::setSyncWord(SyncWordType_t syncWord)
{
    switch (syncWord)
    {
    case PUBLIC_SYNCWORD:
        if (!writeRegister(E22_Reg_LoRaSyncWordMSB, 0x34))
        {
            ESP_LOGE(E22TAG, "Error al enviar el comando SetSyncWord a la queue.");
            return false;
        }
        if (!writeRegister(E22_Reg_LoRaSyncWordLSB, 0x44))
        {
            ESP_LOGE(E22TAG, "Error al enviar el comando SetSyncWord a la queue.");
            return false;
        }
        break;

    case PRIVATE_SYNCWORD:
        if (!writeRegister(E22_Reg_LoRaSyncWordMSB, 0x14))
        {
            ESP_LOGE(E22TAG, "Error al enviar el comando SetSyncWord a la queue.");
            return false;
        }
        if (!writeRegister(E22_Reg_LoRaSyncWordLSB, 0x24))
        {
            ESP_LOGE(E22TAG, "Error al enviar el comando SetSyncWord a la queue.");
            return false;
        }
        break;

    default:
        ESP_LOGE(E22TAG, "Error al enviar el comando SetSyncWord a la queue.");
        return false;
        break;
    }

    return true; // TODO: FIX
}

bool E22::updateIrqStatus(void)
{
    E22Command_t command;
    E22Response_t response;
    memset(&command, 0, sizeof(E22Command_t));
    memset(&response, 0, sizeof(E22Response_t));
    command.commandCode = E22_CMD_GetIrqStatus;
    command.paramCount = 4;
    command.hasResponse = true;
    command.responsesCount = 4;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        if (xQueueReceive(xE22ResponseQueue, &(response), pdMS_TO_TICKS(10000)) == pdPASS)
        {
            uint16_t irqStatus = ((uint16_t)(response.responses.responsesArray[0] << 8)) | response.responses.responsesArray[1];
            updateIRQStatusFromMask(irqStatus);
            return true;
        }
        ESP_LOGE(E22TAG, "Error al leer el comando GetIrqStatus a la queue.");
        return false;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando GetIrqStatus a la queue.");
    return false;
}

bool E22::clearIrqStatus(IRQReg_t IRQRegClear)
{
    uint16_t IRQAux = 0;
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_ClearIrqStatus;
    command.paramCount = 3;
    IRQAux = processIRQMask(IRQRegClear);
    command.params.paramsArray[0] = (uint8_t)(IRQAux >> 8);
    command.params.paramsArray[1] = (uint8_t)(IRQAux >> 0);

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando ClearIrqStatus a la queue.");
    return false;
}

void E22::updateIRQStatusFromMask(uint16_t IRQRegValue)
{
    memset(&IRQReg, 0, sizeof(IRQReg_t));
    if (IRQRegValue & TX_DONE)
    {
        IRQReg.txDone = true;
    }
    if (IRQRegValue & RX_DONE)
    {
        IRQReg.rxDone = true;
    }
    if (IRQRegValue & PREAMBLE_DETECTED)
    {
        IRQReg.preambleDetected = true;
    }
    if (IRQRegValue & SYNCWORD_VALID)
    {
        IRQReg.syncWordValid = true;
    }
    if (IRQRegValue & HEADER_VALID)
    {
        IRQReg.headerValid = true;
    }
    if (IRQRegValue & HEADER_ERR)
    {
        IRQReg.headerErr = true;
    }
    if (IRQRegValue & CRC_ERR)
    {
        IRQReg.crcErr = true;
    }
    if (IRQRegValue & CAD_DONE)
    {
        IRQReg.cadDone = true;
    }
    if (IRQRegValue & CAD_DETECTED)
    {
        IRQReg.cadDetected = true;
    }
    if (IRQRegValue & TIMEOUT)
    {
        IRQReg.timeout = true;
    }
    if (IRQRegValue & LRFGSS_HOP)
    {
        IRQReg.lrFhssHop = true;
    }
}

bool E22::setDioIrqParams(IRQReg_t IRQMask, IRQReg_t DIO1Mask, IRQReg_t DIO2Mask, IRQReg_t DIO3Mask)
{
    E22Command_t command;
    uint16_t IRQAux = 0;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_SetDioIrqParams;
    command.paramCount = 9;
    IRQAux = processIRQMask(IRQMask);
    command.params.paramsArray[0] = (uint8_t)(IRQAux >> 8);
    command.params.paramsArray[1] = (uint8_t)(IRQAux >> 0);
    IRQAux = processIRQMask(DIO1Mask);
    command.params.paramsArray[2] = (uint8_t)(IRQAux >> 8);
    command.params.paramsArray[3] = (uint8_t)(IRQAux >> 0);
    IRQAux = processIRQMask(DIO2Mask);
    command.params.paramsArray[4] = (uint8_t)(IRQAux >> 8);
    command.params.paramsArray[5] = (uint8_t)(IRQAux >> 0);
    IRQAux = processIRQMask(DIO3Mask);
    command.params.paramsArray[6] = (uint8_t)(IRQAux >> 8);
    command.params.paramsArray[7] = (uint8_t)(IRQAux >> 0);

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando setDioIrqParams a la queue.");
    return false;
}

uint16_t E22::processIRQMask(IRQReg_t IRQMask)
{
    uint16_t IRQAux = 0;
    if (IRQMask.txDone)
    {
        IRQAux = IRQAux | TX_DONE;
    }
    if (IRQMask.rxDone)
    {
        IRQAux = IRQAux | RX_DONE;
    }
    if (IRQMask.preambleDetected)
    {
        IRQAux = IRQAux | PREAMBLE_DETECTED;
    }
    if (IRQMask.syncWordValid)
    {
        IRQAux = IRQAux | SYNCWORD_VALID;
    }
    if (IRQMask.headerValid)
    {
        IRQAux = IRQAux | HEADER_VALID;
    }
    if (IRQMask.headerErr)
    {
        IRQAux = IRQAux | HEADER_ERR;
    }
    if (IRQMask.crcErr)
    {
        IRQAux = IRQAux | CRC_ERR;
    }
    if (IRQMask.cadDone)
    {
        IRQAux = IRQAux | CAD_DONE;
    }
    if (IRQMask.cadDetected)
    {
        IRQAux = IRQAux | CAD_DETECTED;
    }
    if (IRQMask.timeout)
    {
        IRQAux = IRQAux | TIMEOUT;
    }
    if (IRQMask.lrFhssHop)
    {
        IRQAux = IRQAux | LRFGSS_HOP;
    }

    return IRQAux;
}

bool E22::getRxBufferStatus(uint8_t *payLoadLenghtRx, uint8_t *RxBufferAddr)
{
    E22Command_t command;
    E22Response_t response;
    memset(&command, 0, sizeof(E22Command_t));
    memset(&response, 0, sizeof(E22Response_t));
    command.commandCode = E22_CMD_GetRxBufferStatus;
    command.paramCount = 4;
    command.hasResponse = true;
    command.responsesCount = 4;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        if (xQueueReceive(xE22ResponseQueue, &(response), pdMS_TO_TICKS(10000)) == pdPASS)
        {
            *payLoadLenghtRx = response.responses.responsesArray[0];
            *RxBufferAddr = response.responses.responsesArray[1];
            return true;
        }

        ESP_LOGE(E22TAG, "Error al leer el comando getRxBufferStatus de la queue.");
        return false;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando GetRxBufferStatus a la queue.");
    return false;
}

bool E22::getPacketStatus(uint8_t *RssiPkt, uint8_t *SnrPkt, uint8_t *SignalRssiPkt)
{
    E22Command_t command;
    E22Response_t response;
    memset(&command, 0, sizeof(E22Command_t));
    memset(&response, 0, sizeof(E22Response_t));
    command.commandCode = E22_CMD_GetPacketStatus;
    command.paramCount = 5;
    command.hasResponse = true;
    command.responsesCount = 5;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        if (xQueueReceive(xE22ResponseQueue, &(response), pdMS_TO_TICKS(10000)) == pdPASS)
        {
            *RssiPkt = response.responses.responsesArray[0];
            *SnrPkt = response.responses.responsesArray[1];
            *SignalRssiPkt = response.responses.responsesArray[2];
            return true;
        }

        ESP_LOGE(E22TAG, "Error al leer el comando GetPacketStatus a la queue.");
        return false;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando GetPacketStatus a la queue.");
    return false;
}

bool E22::messageIsAvailable(void)
{
    return PacketReceived;
}

uint8_t E22::getMessageLenght(void)
{
    return s_PayloadLenghtRx;
}

bool E22::getMessageRxByte(uint8_t *dataOut)
{
    if (SX126X_RX_BASE_BUFFER_ADDR + s_RxBufferAddr > s_PayloadLenghtRx) // TODO
    {
        return false;
    }
    if (!readBuffer(s_RxBufferAddr, dataOut))
    {
        return false;
    }
    s_RxBufferAddr++;
    return true;
}

bool E22::getMessageRxLenght(uint8_t *dataOut, uint8_t length)
{
    for (uint8_t i = 0; i < s_PayloadLenghtRx; i++)
    {
        if (SX126X_RX_BASE_BUFFER_ADDR + s_RxBufferAddr > s_PayloadLenghtRx) // TODO
        {
            return false;
        }
        if (!readBuffer(s_RxBufferAddr, &dataOut[i]))
        {
            return false;
        }
        s_RxBufferAddr++;
    }
    return true;
}

bool E22::beginTxPacket(void)
{
    IO &io = IO::getInstance();

    // Set buffer addresses to SX126X_TX_BASE_BUFFER_ADDR and SX126X_RX_BASE_BUFFER_ADDR
    setBufferBaseAddress(SX126X_TX_BASE_BUFFER_ADDR, SX126X_RX_BASE_BUFFER_ADDR);
    s_TxBufferAddr = SX126X_TX_BASE_BUFFER_ADDR;
    s_RxBufferAddr = SX126X_RX_BASE_BUFFER_ADDR;

    // Set RxEn and TxEn to the corresponding values
    io.SetLevel((gpio_num_t)RX_EN_E22_PIN, IO_LOW);
    io.SetLevel((gpio_num_t)TX_EN_E22_PIN, IO_HIGH);

    fixModulationQuality();

    return true;
}

bool E22::writeMessageTxByte(uint8_t data)
{
    if (!writeBuffer(s_TxBufferAddr, data))
    {
        return false;
    }
    s_TxBufferAddr++;
    return true;
}
bool E22::writeMessageTxLength(uint8_t *data, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        if (!writeBuffer(s_TxBufferAddr, data[i]))
        {
            return false;
        }
        s_TxBufferAddr++;
    }
    return true;
}

bool E22::transmitPacket(uint32_t Timeout)
{
    IRQReg_t irqStatus;
    memset(&irqStatus, 0, sizeof(IRQReg_t));

    // Clear all previous interrupts
    if (!clearIrqStatus(IRQREGFULL))
    {
        return false;
    }

    // Set DIO1 IRQ params for TX done and timeout
    irqStatus.txDone = true;
    irqStatus.timeout = true;
    if (!setDioIrqParams(irqStatus, irqStatus, IRQREGEMPTY, IRQREGEMPTY))
    {
        return false;
    }

    // Send the PacketParams in case it was changed
    if (!setPacketParams(s_packetParams))
    {
        return false;
    }

    // Set device to transmit mode with configured timeout
    if (!setTx(Timeout))
    {
        return false;
    }

    // Enable Interrupt
    if (gpio_set_intr_type((gpio_num_t)DIO1_E22_PIN, GPIO_INTR_POSEDGE) != ESP_OK)
    {
        return false;
    }

    return true;
}

bool E22::changePacketPreambleLenght(uint16_t preambleLenght)
{
    s_packetParams.preambleLength = preambleLenght;
    return setPacketParams(s_packetParams);
}
bool E22::changePacketHeaderType(PacketHeaderType_t headerType)
{
    s_packetParams.headerType = headerType;
    return setPacketParams(s_packetParams);
}
bool E22::changePacketPayloadLength(uint8_t payloadLength)
{
    s_packetParams.payloadLength = payloadLength;
    return setPacketParams(s_packetParams);
}
bool E22::changePacketCrcType(bool crcType)
{
    s_packetParams.crcType = crcType;
    return setPacketParams(s_packetParams);
}
bool E22::changePacketIQType(PacketIQType_t iqType)
{
    s_packetParams.iqType = iqType;
    return setPacketParams(s_packetParams);
}

bool E22::setTx(uint32_t Timeout)
{
    if (Timeout > 0xFFFFFF)
    {
        ESP_LOGE(E22TAG, "Timeout invalido, tiene que ser un valor entre 0x000000 y 0xFFFFFF.");
        return false;
    }
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    uint32_t TimeoutBits = Timeout << 6;
    command.commandCode = E22_CMD_SetTx;
    command.paramCount = 4;
    command.params.paramsArray[0] = (uint8_t)(TimeoutBits >> 16);
    command.params.paramsArray[1] = (uint8_t)(TimeoutBits >> 8);
    command.params.paramsArray[2] = (uint8_t)(TimeoutBits >> 0);

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando setTx a la queue.");
    return false;
}

bool E22::receivePacket(uint32_t Timeout)
{
    IO &io = IO::getInstance();
    s_PayloadLenghtRx = 0;
    s_RxBufferAddr = 0;
    PacketReceived = false;
    IRQReg_t irqStatus;
    memset(&irqStatus, 0, sizeof(IRQReg_t));

    // Clear all previous interrupts
    if (!clearIrqStatus(IRQREGFULL))
    {
        return false;
    }

    // Set DIO1 IRQ params for RX done, timeout, header error, and CRC error
    irqStatus.rxDone = true;
    irqStatus.timeout = true;
    irqStatus.headerErr = true;
    irqStatus.crcErr = true;
    if (!setDioIrqParams(irqStatus, irqStatus, IRQREGEMPTY, IRQREGEMPTY))
    {
        return false;
    }

    // Set RxEn and TxEn to the corresponding values
    io.SetLevel((gpio_num_t)RX_EN_E22_PIN, IO_HIGH);
    io.SetLevel((gpio_num_t)TX_EN_E22_PIN, IO_LOW);

    // Set device to receive mode with configured timeout
    if (!setRx(Timeout))
    {
        return false;
    }

    // Enable Interrupt
    if (gpio_set_intr_type((gpio_num_t)DIO1_E22_PIN, GPIO_INTR_POSEDGE) != ESP_OK)
    {
        return false;
    }

    return true;
}

void E22::setMsgTimeoutms(uint32_t newMsgTimeoutms)
{
    s_msgTimeoutms = newMsgTimeoutms;
}

bool E22::setPaConfig(PaConfig_t PaConfig)
{
    uint8_t paDutyCycle = 0;
    uint8_t hpMax = 0;
    uint8_t DeviceSel = 0x00;
    uint8_t paLut = 0x01;
    switch (PaConfig)
    {
    case PA_22_DBM:
    {
        paDutyCycle = 0x04;
        hpMax = 0x07;
    }
    break;

    case PA_20_DBM:
    {
        paDutyCycle = 0x03;
        hpMax = 0x05;
    }
    break;

    case PA_17_DBM:
    {
        paDutyCycle = 0x02;
        hpMax = 0x03;
    }
    break;

    case PA_14_DBM:
    {
        paDutyCycle = 0x02;
        hpMax = 0x02;
    }
    break;

    default:
    {
        return false;
    }
    break;
    }

    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_SetPaConfig;
    command.paramCount = 5;
    command.params.paramsArray[0] = paDutyCycle;
    command.params.paramsArray[1] = hpMax;
    command.params.paramsArray[2] = DeviceSel;
    command.params.paramsArray[3] = paLut;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando SetPaConfig a la queue.");
    return false;
}

bool E22::setTxParams(RampTime_t RampTime)
{
    E22Command_t command;
    memset(&command, 0, sizeof(E22Command_t));
    command.commandCode = E22_CMD_SetTxParams;
    command.paramCount = 3;
    command.params.paramsArray[0] = 0x16; // 0x16 = 22 dBm
    command.params.paramsArray[1] = (uint8_t)RampTime;

    if (xQueueSend(xE22CmdQueue, (void *)&command, 0) == pdPASS)
    {
        return true;
    }

    ESP_LOGE(E22TAG, "Error al enviar el comando SetTxParams a la queue.");
    return false;
}

bool E22::checkDeviceConnection(void)
{
    SPI &spi = SPI::getInstance();

    uint8_t TxBuffer[MAX_CMD_PARAMS + 1];
    uint8_t RxBuffer[MAX_CMD_PARAMS + 1];

    memset(&TxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));
    memset(&RxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));

    TxBuffer[0] = E22_OpCode_SetStandby;
    TxBuffer[1] = STDBY_RC;

    if (!spi.SendMessage(TxBuffer, 2))
    {
        ESP_LOGE(E22TAG, "Error al enviar el comando SetStandby");
        return false;
    }

    memset(&TxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));

    TxBuffer[0] = E22_OpCode_GetStatus;
    TxBuffer[1] = 0x00;

    if (!spi.SendMessage(TxBuffer, 2, RxBuffer, 2))
    {
        ESP_LOGE(E22TAG, "Error al enviar el comando getStatus");
        return false;
    }
    if (RxBuffer[1] & 0x20)
    {
        ESP_LOGI(E22TAG, "Dispositivo conectado");
        return true;
    }

    ESP_LOGE(E22TAG, "Dispositivo no conectado");
    return true;
}

bool E22::antennaMismatchCorrection(void)
{
    SPI &spi = SPI::getInstance();

    uint8_t TxBuffer[MAX_CMD_PARAMS + 1];
    uint8_t RxBuffer[MAX_CMD_PARAMS + 1];
    uint8_t value = 0;

    memset(&TxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));
    memset(&RxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));

    TxBuffer[0] = E22_OpCode_ReadRegister;
    TxBuffer[1] = (uint8_t)(E22_Reg_TxClampConfig >> 8);
    TxBuffer[2] = (uint8_t)(E22_Reg_TxClampConfig >> 0);

    if (!spi.SendMessage(TxBuffer, 5, RxBuffer, 5))
    {
        ESP_LOGE(E22TAG, "Error al enviar el comando ReadRegister al spi.");
        return false;
    }

    value = RxBuffer[4] | 0x1E;

    memset(&TxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));
    TxBuffer[0] = E22_OpCode_WriteRegister;
    TxBuffer[1] = (uint8_t)(E22_Reg_TxClampConfig >> 8);
    TxBuffer[2] = (uint8_t)(E22_Reg_TxClampConfig >> 0);
    TxBuffer[3] = value;

    if (!spi.SendMessage(TxBuffer, 4))
    {
        ESP_LOGE(E22TAG, "Error al enviar el comando WriteRegister al spi.");
        return false;
    }

    return true;
}

bool E22::getIRQStatusForInterrupt(void)
{
    SPI &spi = SPI::getInstance();

    uint8_t TxBuffer[MAX_CMD_PARAMS + 1];
    uint8_t RxBuffer[MAX_CMD_PARAMS + 1];

    memset(&TxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));
    memset(&RxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));
    TxBuffer[0] = E22_OpCode_GetIrqStatus;

    if (!spi.SendMessage(TxBuffer, 4, RxBuffer, 4))
    {
        ESP_LOGE(E22TAG, "Error al enviar el comando GetIrqStatus al spi.");
        return false;
    }
    uint16_t irqStatus = (RxBuffer[2] << 8) | RxBuffer[3];
    updateIRQStatusFromMask(irqStatus);

    return true;
}

bool E22::getRxBufferStatusForInterrupt(void)
{
    SPI &spi = SPI::getInstance();

    uint8_t TxBuffer[MAX_CMD_PARAMS + 1];
    uint8_t RxBuffer[MAX_CMD_PARAMS + 1];

    memset(&TxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));
    memset(&RxBuffer, 0, sizeof(uint8_t) * (MAX_CMD_PARAMS + 1));
    TxBuffer[0] = E22_OpCode_GetRxBufferStatus;
    TxBuffer[1] = 0x00;
    TxBuffer[2] = 0x00;
    TxBuffer[3] = 0x00;

    if (!spi.SendMessage(TxBuffer, 4, RxBuffer, 4))
    {
        ESP_LOGE(E22TAG, "Error al enviar el comando GetRxBufferStatus al spi.");
        return false;
    }
    s_PayloadLenghtRx = RxBuffer[2];
    s_RxBufferAddr = RxBuffer[3];

    return true;
}

bool E22::fixInvertedIq(PacketIQType_t iqType)
{
    uint8_t fixInvertedIqValue;
    if (!readRegister(E22_Reg_IQPolaritySetup, &fixInvertedIqValue))
    {
        ESP_LOGE(E22TAG, "Error al leer el registro IQPolaritySetup");
        return false;
    }

    if (iqType == STANDARD_IQ)
    {
        fixInvertedIqValue |= (1 << 2);
    }
    else
    {
        fixInvertedIqValue &= ~(1 << 2);
    }

    if (!writeRegister(E22_Reg_IQPolaritySetup, fixInvertedIqValue))
    {
        ESP_LOGE(E22TAG, "Error al escribir el registro IQPolaritySetup");
        return false;
    }
    return true;
}

bool E22::fixModulationQuality(void)
{
    uint8_t fixModulationQuality;
    if (!readRegister(E22_Reg_TxModulation, &fixModulationQuality))
    {
        ESP_LOGE(E22TAG, "Error al leer el registro TxModulation");
        return false;
    }

    if (s_modulationParams.bandwidth == LORA_BW_500)
    {
        fixModulationQuality &= ~(1 << 2);
    }
    else
    {
        fixModulationQuality |= (1 << 2);
    }

    if (!writeRegister(E22_Reg_TxModulation, fixModulationQuality))
    {
        ESP_LOGE(E22TAG, "Error al escribir el registro TxModulation");
        return false;
    }
    return true;
}

bool E22::setUpForRx(void)
{
    E22 &e22 = E22::getInstance();
    DEVICESTATUS &ds = DEVICESTATUS::getInstance();

    ESP_LOGI(E22TAG, "Setting up for TX");
    ESP_LOGI(E22TAG, "Going to standby RC");
    e22.setStandBy(E22::STDBY_RC);

    ESP_LOGI(E22TAG, "Setting up packet type: %d", ds.deviceStatus.E22Status.packetType);
    e22.setPacketType(ds.deviceStatus.E22Status.packetType);

    ESP_LOGI(E22TAG, "Setting up TCXO voltage: %d, TCXO delay: %lu", ds.deviceStatus.E22Status.tcxoVoltage, ds.deviceStatus.E22Status.TXCOdio3Delay);
    e22.setDIO3asTCXOCtrl(ds.deviceStatus.E22Status.tcxoVoltage, ds.deviceStatus.E22Status.TXCOdio3Delay);

    ESP_LOGI(E22TAG, "Going to standby RC");
    e22.setStandBy(E22::STDBY_RC);

    ESP_LOGI(E22TAG, "Calibrating");
    e22.calibrate(ds.deviceStatus.E22Status.calibrations);

    if (ds.deviceStatus.E22Status.xtalConfig.use)
    {
        ESP_LOGI(E22TAG, "Using XTAL: A=%d, B=%d", ds.deviceStatus.E22Status.xtalConfig.XTALA, ds.deviceStatus.E22Status.xtalConfig.XTALB);
        e22.setXtalCap(ds.deviceStatus.E22Status.xtalConfig.XTALA, ds.deviceStatus.E22Status.xtalConfig.XTALB);
    }

    ESP_LOGI(E22TAG, "Calibrating image frequency: %d", ds.deviceStatus.E22Status.imageCalibrationFreq);
    e22.calibrateImage(ds.deviceStatus.E22Status.imageCalibrationFreq);
    ESP_LOGI(E22TAG, "Setting up frequency: %lu", ds.deviceStatus.E22Status.frequency);
    e22.setFrequency(ds.deviceStatus.E22Status.frequency);

    
    ESP_LOGI(E22TAG, "Setting up Rx gain: %d", ds.deviceStatus.E22Status.rxGain);
    e22.setRxGain(ds.deviceStatus.E22Status.rxGain);

    ESP_LOGI(E22TAG, "Setting up modulation parameters");
    e22.setModulationParams(ds.deviceStatus.E22Status.modulationParams);

    ESP_LOGI(E22TAG, "Setting up packet parameters");
    e22.setPacketParams(ds.deviceStatus.E22Status.packetParams);
    e22.fixInvertedIq(ds.deviceStatus.E22Status.packetParams.iqType);

    ESP_LOGI(E22TAG, "Setting up sync word: %d", ds.deviceStatus.E22Status.syncWord);
    e22.setSyncWord(ds.deviceStatus.E22Status.syncWord);

    ESP_LOGI(E22TAG, "Setting up buffer base address: %d, %d", SX126X_TX_BASE_BUFFER_ADDR, SX126X_RX_BASE_BUFFER_ADDR);
    e22.setBufferBaseAddress(SX126X_TX_BASE_BUFFER_ADDR, SX126X_RX_BASE_BUFFER_ADDR);

    return true;
}

bool E22::setUpForTx(void)
{
    E22 &e22 = E22::getInstance();
    DEVICESTATUS &ds = DEVICESTATUS::getInstance();

    ESP_LOGI(E22TAG, "Setting up for TX");
    ESP_LOGI(E22TAG, "Going to standby RC");
    e22.setStandBy(E22::STDBY_RC);

    ESP_LOGI(E22TAG, "Setting up packet type: %d", ds.deviceStatus.E22Status.packetType);
    e22.setPacketType(ds.deviceStatus.E22Status.packetType);

    ESP_LOGI(E22TAG, "Setting up TCXO voltage: %d, TCXO delay: %lu", ds.deviceStatus.E22Status.tcxoVoltage, ds.deviceStatus.E22Status.TXCOdio3Delay);
    e22.setDIO3asTCXOCtrl(ds.deviceStatus.E22Status.tcxoVoltage, ds.deviceStatus.E22Status.TXCOdio3Delay);

    ESP_LOGI(E22TAG, "Going to standby RC");
    e22.setStandBy(E22::STDBY_RC);

    ESP_LOGI(E22TAG, "Calibrating");
    e22.calibrate(ds.deviceStatus.E22Status.calibrations);

    if (ds.deviceStatus.E22Status.xtalConfig.use)
    {
        ESP_LOGI(E22TAG, "Using XTAL: A=%d, B=%d", ds.deviceStatus.E22Status.xtalConfig.XTALA, ds.deviceStatus.E22Status.xtalConfig.XTALB);
        e22.setXtalCap(ds.deviceStatus.E22Status.xtalConfig.XTALA, ds.deviceStatus.E22Status.xtalConfig.XTALB);
    }

    ESP_LOGI(E22TAG, "Calibrating image frequency: %d", ds.deviceStatus.E22Status.imageCalibrationFreq);
    e22.calibrateImage(ds.deviceStatus.E22Status.imageCalibrationFreq);
    ESP_LOGI(E22TAG, "Setting up frequency: %lu", ds.deviceStatus.E22Status.frequency);
    e22.setFrequency(ds.deviceStatus.E22Status.frequency);

    ESP_LOGI(E22TAG, "Setting up TX power: %d", ds.deviceStatus.E22Status.paConfig);
    e22.setPaConfig(ds.deviceStatus.E22Status.paConfig);

    ESP_LOGI(E22TAG, "Setting up ramp time: %d", ds.deviceStatus.E22Status.rampTime);
    e22.setTxParams(ds.deviceStatus.E22Status.rampTime);

    ESP_LOGI(E22TAG, "Setting up modulation parameters");
    e22.setModulationParams(ds.deviceStatus.E22Status.modulationParams);

    ESP_LOGI(E22TAG, "Setting up packet parameters");
    e22.setPacketParams(ds.deviceStatus.E22Status.packetParams);
    e22.fixInvertedIq(ds.deviceStatus.E22Status.packetParams.iqType);

    ESP_LOGI(E22TAG, "Setting up sync word: %d", ds.deviceStatus.E22Status.syncWord);
    e22.setSyncWord(ds.deviceStatus.E22Status.syncWord);

    ESP_LOGI(E22TAG, "Setting up buffer base address: %d, %d", SX126X_TX_BASE_BUFFER_ADDR, SX126X_RX_BASE_BUFFER_ADDR);
    e22.setBufferBaseAddress(SX126X_TX_BASE_BUFFER_ADDR, SX126X_RX_BASE_BUFFER_ADDR);

    return true;
}