#include "argtable3/argtable3.h"
#include "includes.h"
#include "sys/queue.h"

CLI::timercmd_args_t CLI::timer_args;
CLI::configcmd_args_t CLI::config_args;
CLI::FScmd_args_t CLI::FS_args;

static int tasks_infocmd()
{
    const size_t bytes_per_task = 40; /* see vTaskList description */
    char *task_list_buffer = (char *)malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_list_buffer == NULL)
    {
        return 1;
    }
    fputs("Task Name\tStatus\tPrio\tHWM\tTask#", stdout);
#ifdef CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
    fputs("\tAffinity", stdout);
#endif
    fputs("\n", stdout);
    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);
    free(task_list_buffer);
    return 0;
}

esp_err_t CLI::esp_console_register_simple_command(const char *commandtext, const char *help, esp_console_cmd_func_t func)
{
    esp_console_cmd_t command = {
        .command = commandtext,
        .help = help,
        .hint = NULL,
        .func = func,
        .argtable = NULL};
    return esp_console_cmd_register(&command);
}

int CLI::showStatusCMD(int argc, char **argv)
{
    // TODO: Llenar el show status
    return 0;
}

int CLI::recieveCMD(int argc, char **argv)
{
    E22 &e22 = E22::getInstance();
    //printf("Set RF module to use TCXO as clock reference\n\r");
    //e22.setStandBy(E22::STDBY_RC);
//
    //e22.setPacketType(E22::PACKET_TYPE_LORA);
//
    //// Set frequency to 915 Mhz
    //printf("Set frequency to 915 Mhz\n\r");
    //e22.setFrequency(915000000);
    e22.setStandBy(E22::STDBY_RC);
    e22.setPacketType(E22::PACKET_TYPE_LORA);
//
    //e22.setBufferBaseAddress(SX126X_TX_BASE_BUFFER_ADDR, SX126X_RX_BASE_BUFFER_ADDR);
    //E22::tcxoVoltage_t dio3Voltage = E22::TCXOVOLTAGE_1_8;
    e22.setDIO3asTCXOCtrl(E22::TCXOVOLTAGE_1_8, 10);

    e22.setStandBy(E22::STDBY_RC);

    E22::Calibrate_t calibrationsToDo;
    memset(&calibrationsToDo, 0, sizeof(E22::Calibrate_t));
    calibrationsToDo.RC64kCalibration = true;
    calibrationsToDo.RC13MCalibration = true;
    calibrationsToDo.PLLCalibration = true;
    calibrationsToDo.ADCPulseCalibration = true;
    calibrationsToDo.ADCBulkNCalibration = true;
    calibrationsToDo.ADCBulkPCalibration = true;
    calibrationsToDo.ImageCalibration = true;
    e22.calibrate(calibrationsToDo);

    //uint8_t xtalA = 0x12;
    //uint8_t xtalB = 0x12;
    //printf("Set RF module to use XTAL as clock reference\n\r");
    //e22.setXtalCap(xtalA, xtalB);

    // Set frequency to 915 Mhz
    printf("Set frequency to 915 Mhz\n\r");
    e22.calibrateImage(E22::FREQ_902_928);
    e22.setFrequency(915000000);

    // Set RX gain. RX gain option are power saving gain or boosted gain
    printf("Set RX gain to power saving gain\n\r");
    //E22::RxGain_t gain = E22::RX_POWER_SAVE;
    e22.setRxGain(E22::RX_BOOST); // Power saving gain

    // Configure modulation parameter including spreading factor (SF), bandwidth (BW), and coding rate (CR)
    // Transmitter must have same SF and BW setting so receiver can receive LoRa packet
    printf("Set modulation parameters:\n\tSpreading factor = 7\n\tBandwidth = 125 kHz\n\tCoding rate = 4/5\r\n");
    E22::ModulationParameters_t modulation;
    modulation.spredingFactor = E22::SF_7;    // LoRa spreading factor: 7
    modulation.bandwidth = E22::LORA_BW_125;  // LoRa bandwidth: 125 kHz
    modulation.codingRate = E22::LORA_CR_4_5; // Coding rate: 4/5
    e22.setModulationParams(modulation);

    // Configure packet parameter including header type, preamble length, payload length, and CRC type
    // The explicit packet includes header contain CR, number of byte, and CRC type
    // Packet with explicit header can't be received by receiver with implicit header mode
    printf("Set packet parameters:\n\tExplicit header type\n\tPreamble length = 12\n\tPayload Length = 15\n\tCRC on\n\r");
    E22::LoraPacketParams_t packetParams;
    packetParams.headerType = E22::EXPLICIT_HEADER; // Explicit header mode
    packetParams.preambleLength = 12;               // Set preamble length to 12
    packetParams.payloadLength = 15;                // Initialize payloadLength to 15
    packetParams.crcType = true;                    // Set CRC enable
    packetParams.iqType = E22::STANDARD_IQ;
    e22.setPacketParams(packetParams);

    // Set syncronize word for public network (0x3444)
    printf("Set syncronize word to 0x3444\n\r");
    //E22::SyncWordType_t syncWord = E22::PUBLIC_SYNCWORD;
    e22.setSyncWord(E22::PUBLIC_SYNCWORD);

    printf("\n-- LORA RECEIVER --\n");

    e22.receivePacket(100000);

    while(e22.messageIsAvailable() == false){
        printf("Waiting for packet...\n\r");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    printf("Received packet size %u\n\r", e22.getMessageLenght());
    char messageOut[100];
    memset(messageOut, 0, sizeof(messageOut));
    e22.getMessageRxLenght((uint8_t *)messageOut, 15);
    printf("Message: %s\n\r", messageOut);

    return 0;
}

int CLI::transmitCMD(int argc, char **argv)
{
    E22 &e22 = E22::getInstance();
    //printf("Set RF module to use TCXO as clock reference\r\n");
//
    //e22.setStandBy(E22::STDBY_RC);
    //e22.setPacketType(E22::PACKET_TYPE_LORA);
    //e22.setBufferBaseAddress(SX126X_TX_BASE_BUFFER_ADDR, SX126X_RX_BASE_BUFFER_ADDR);
    e22.setStandBy(E22::STDBY_RC);
    e22.setPacketType(E22::PACKET_TYPE_LORA);

    //E22::tcxoVoltage_t dio3Voltage = E22::TCXOVOLTAGE_1_8;
    e22.setDIO3asTCXOCtrl(E22::TCXOVOLTAGE_1_8, 10);
    e22.setStandBy(E22::STDBY_RC);
    E22::Calibrate_t calibrationsToDo;
    memset(&calibrationsToDo, 0, sizeof(E22::Calibrate_t));
    calibrationsToDo.RC64kCalibration = true;
    calibrationsToDo.RC13MCalibration = true;
    calibrationsToDo.PLLCalibration = true;
    calibrationsToDo.ADCPulseCalibration = true;
    calibrationsToDo.ADCBulkNCalibration = true;
    calibrationsToDo.ADCBulkPCalibration = true;
    calibrationsToDo.ImageCalibration = true;
    e22.calibrate(calibrationsToDo);

    // uncomment code below to use XTAL
    //int8_t xtalA = 0x12;
    //int8_t xtalB = 0x12;
    //rintf("Set RF module to use XTAL as clock reference\n\r");
    //22.setXtalCap(xtalA, xtalB);

    // Set frequency to 915 Mhz
    printf("Set frequency to 915 Mhz\n\r");
    e22.calibrateImage(E22::FREQ_902_928);
    e22.setFrequency(915000000);

    // Set TX power, default power for SX1262 and SX1268 are +22 dBm and for SX1261 is +14 dBm
    // This function will set PA config with optimal setting for requested TX power
    printf("Set TX power to +17 dBm\n\r");
    //E22::PaConfig_t paConfig = E22::PA_17_DBM; // TX power +17 dBm for SX1262
    e22.setPaConfig(E22::PA_17_DBM);
    //E22::RampTime_t rampTime = ; // 800 microsecond Ramp Time
    e22.setTxParams(E22::SET_RAMP_800U);

    // Configure modulation parameter including spreading factor (SF), bandwidth (BW), and coding rate (CR)
    // Transmitter must have same SF and BW setting so receiver can receive LoRa packet
    printf("Set modulation parameters:\n\tSpreading factor = 7\n\tBandwidth = 125 kHz\n\tCoding rate = 4/5\r\n");
    E22::ModulationParameters_t modulation;
    modulation.spredingFactor = E22::SF_7;    // LoRa spreading factor: 7
    modulation.bandwidth = E22::LORA_BW_125;  // LoRa bandwidth: 125 kHz
    modulation.codingRate = E22::LORA_CR_4_5; // Coding rate: 4/5
    e22.setModulationParams(modulation);

    // Configure packet parameter including header type, preamble length, payload length, and CRC type
    // The explicit packet includes header contain CR, number of byte, and CRC type
    // Packet with explicit header can't be received by receiver with implicit header mode
    printf("Set packet parameters:\n\tExplicit header type\n\tPreamble length = 12\n\tPayload Length = 15\n\tCRC on\n\r");
    E22::LoraPacketParams_t packetParams;
    packetParams.headerType = E22::EXPLICIT_HEADER; // Explicit header mode
    packetParams.preambleLength = 12;               // Set preamble length to 12
    packetParams.payloadLength = 15;                // Initialize payloadLength to 15
    packetParams.crcType = true;                    // Set CRC enable
    packetParams.iqType = E22::STANDARD_IQ;
    e22.setPacketParams(packetParams);
    e22.fixInvertedIq(packetParams.iqType);

    // Set syncronize word for public network (0x3444)
    printf("Set syncronize word to 0x3444\n\r");
    //E22::SyncWordType_t syncWord = E22::PUBLIC_SYNCWORD;
    e22.setSyncWord(E22::PUBLIC_SYNCWORD);

    printf("\n-- LORA TRANSMITTER --\n\r");

    //-----------------------------------------------------------------------------------------------------------------

    // Transmit message and counter
    // write() method must be placed between beginPacket() and endPacket()
    static uint8_t counter = 0;
    //char message[] = "HeLoRa World!";
    char message[] = "Hola Cabrita!";
    e22.beginTxPacket();
    e22.writeMessageTxLength((uint8_t *)message, sizeof(message));
    e22.writeMessageTxByte(counter);
    e22.transmitPacket(30000);

    // Print message and counter in serial
    printf("%s %u", message, counter);
    counter++;

    // Wait until modulation process for transmitting packet finish
    //LoRa.wait();

    // Print transmit time
    //Serial.print("Transmit time: ");
    //Serial.print(LoRa.transmitTime());
    //Serial.println(" ms");
    //Serial.println();

    // Don't load RF module with continous transmit
    //delay(5000);

    return 0;
}

esp_err_t CLI::esp_console_register_config_command(void)
{

    config_args.command = arg_str1(NULL, NULL, "<funcion>", "Funcion del comando config.");
    config_args.section = arg_str1(NULL, NULL, "<section>", "Nombre de la seccion.");
    config_args.key = arg_str1(NULL, NULL, "<key>", "Nombre de la key.");
    config_args.value = arg_str0(NULL, NULL, "[value]", "Valor de la key a modificar.");
    config_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "config",
        .help = "Control del config.ini",
        .hint = "<show|set|delete> <section> <key> [value]",
        .func = &configCmdFunc,
        .argtable = &config_args};
    return esp_console_cmd_register(&cmd);
}

int CLI::configCmdFunc(int argc, char **argv)
{
    // FS &fs = FS::getInstance();
    int nerrors = arg_parse(argc, argv, (void **)&config_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, config_args.end, argv[0]);
        return 1;
    }
    const char *command = config_args.command->sval[0];
    if (!strcmp(command, "show"))
    {
        if (config_args.section->count == 0)
        {
            printf("El comando config show necesita un argumento <section>.");
            return 0;
        }
        if (config_args.key->count == 0)
        {
            printf("El comando config show necesita un argumento <key>.");
            return 0;
        }
        const char *section = config_args.section->sval[0];
        const char *key = config_args.key->sval[0];
        /*char value[INI_MAX_LEN];
        memset(value, 0, sizeof(value));
        if (fs.Ini_gets(section, key, value))
        {
            printf("CONFIG [%s] %s=%s\r\n", section, key, value);
        }
        else
        {
            ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
        }*/

        return 0;
    }
    else if (!strcmp(command, "set"))
    {
        if (config_args.section->count == 0)
        {
            printf("El comando config set necesita un argumento <section>.");
            return 0;
        }
        if (config_args.key->count == 0)
        {
            printf("El comando config set necesita un argumento <key>.");
            return 0;
        }
        if (config_args.value->count == 0)
        {
            printf("El comando config set necesita un argumento <value>.");
            return 0;
        }
        const char *section = config_args.section->sval[0];
        const char *key = config_args.key->sval[0];
        const char *value = config_args.value->sval[0];
        /*if (fs.Ini_puts(section, key, value))
        {
            printf("CONFIG SET [%s] %s=%s\r\n", section, key, value);
        }
        else
        {
            ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
        }*/
        return 0;
    }
    else if (!strcmp(command, "delete"))
    {
        if (config_args.section->count == 0)
        {
            printf("El comando config delete necesita un argumento <section>.");
            return 0;
        }
        if (config_args.key->count == 0)
        {
            printf("El comando config delete necesita un argumento <key>.");
            return 0;
        }
        const char *section = config_args.section->sval[0];
        const char *key = config_args.key->sval[0];
        /*if (fs.Ini_puts(section, key, NULL))
        {
            printf("CONFIG DELETE [%s] %s\r\n", section, key);
        }
        else
        {
            ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
        }*/
        return 0;
    }
    else
    {
        printf("Comando no valido\r\n");
        return 0;
    }

    return 0;
}

esp_err_t CLI::esp_console_register_FS_command(void)
{

    FS_args.command = arg_str1(NULL, NULL, "<funcion>", "Funcion del comando FS.");
    FS_args.path = arg_str0(NULL, NULL, "[path]", "Camino del archivo o particion");
    FS_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "fs",
        .help = "Comandos de control del filesystem",
        .hint = "<format|ls|cat> [path]",
        .func = &FSCmdFunc,
        .argtable = &FS_args};
    return esp_console_cmd_register(&cmd);
}

int CLI::FSCmdFunc(int argc, char **argv)
{
    /*
    FS &fs = FS::getInstance();
    int nerrors = arg_parse(argc, argv, (void **)&FS_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, FS_args.end, argv[0]);
        return 1;
    }
    const char *command = FS_args.command->sval[0];
    if (!strcmp(command, "format"))
    {
        const char *path = FS_args.path->sval[0];
        if (!strcmp(path, "portal"))
        {
            ESP_LOGE(FSTAG, "La funcion no fuciona por ahora, ver comentarios en codigo.");
            //fs.formatPartition(PORTAL_PARTITION_NAME);
        }
        else if (!strcmp(path, "storage"))
        {
            ESP_LOGE(FSTAG, "La funcion no fuciona por ahora, ver comentarios en codigo.");
            //fs.formatPartition(STORAGE_PARTITION_NAME);
        }
        else {
            printf("Comando no valido\r\n");
        }
    }
    else if (!strcmp(command, "ls"))
    {
        const char *path = FS_args.path->sval[0];
        if (!strcmp(path, "portal"))
        {
            fs.ls(PORTAL_PARTITION_NAME);
        }
        else if (!strcmp(path, "storage"))
        {
            fs.ls(STORAGE_PARTITION_NAME);
        }
        else
        {
            printf("Comando no valido\r\n");
        }

    }
    else if (!strcmp(command, "cat"))
    {
        const char *path = FS_args.path->sval[0];
        fs.cat((char *)path);
    }
    else
    {
        printf("Comando no valido\r\n");
    }
    */
    return 0;
}

void CLI::esp_console_register_all_commands(void)
{
    esp_console_register_help_command();
    // esp_console_register_timer_command();
    esp_console_register_config_command();
    esp_console_register_FS_command();
    esp_console_register_simple_command("?", "Muestra el estado general del dispositivo", showStatusCMD);
    esp_console_register_simple_command("tx", "Transmit", transmitCMD);
    esp_console_register_simple_command("rx", "Recieve", recieveCMD);
    register_system();
}

void CLI::Begin(void)
{

    memset(&timer_args, 0, sizeof(timer_args));
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    repl_config.prompt = CONSOLE_PROMPT;
    repl_config.max_cmdline_length = MAX_CMDLINE_LEN;

    /* Register commands */
    esp_console_register_all_commands();
    // configuracion para la consola por la UART
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

    // configuracion para la consola por el USB SERIAL
    // esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    // esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl);

    PrintLogo();
    esp_console_start_repl(repl);
    // La consola reaciona bien a EOL de LF solo, CRLF genera una linea vacia despues del comando
    vTaskDelay(pdMS_TO_TICKS(1000));
    return;
}

void CLI::PrintLogo(void)
{

    printf("\r\n");
    printf("   _____              _______   ______   _         ____     _____   _____    _____ \r\n");
    printf("  / ____|     /\\     |__   __| |  ____| | |       / __ \\   / ____| |_   _|  / ____|\r\n");
    printf(" | (___      /  \\       | |    | |__    | |      | |  | | | |  __    | |   | |     \r\n");
    printf("  \\___ \\    / /\\ \\      | |    |  __|   | |      | |  | | | | |_ |   | |   | |     \r\n");
    printf("  ____) |  / ____ \\     | |    | |____  | |____  | |__| | | |__| |  _| |_  | |____ \r\n");
    printf(" |_____/  /_/    \\_\\    |_|    |______| |______|  \\____/   \\_____| |_____|  \\_____|\r\n");
    printf("\r\n");
    fflush(stdout);

    return;
}

void CLI::PrintSeparator(void)
{
    printf("---------------------------------------------------------------------\r\n");
}