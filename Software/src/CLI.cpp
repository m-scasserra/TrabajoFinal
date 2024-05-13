#include "argtable3/argtable3.h"
#include "includes.h"
#include "sys/queue.h"
#include "DeviceStatus.h"

CLI::timercmd_args_t CLI::timer_args;
CLI::configcmd_args_t CLI::config_args;
CLI::FScmd_args_t CLI::FS_args;
CLI::netcmd_args_t CLI::net_args;
CLI::timecmd_args_t CLI::time_args;
CLI::AJcmd_args_t CLI::AJ_args;

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
    DEVICESTATUS &ds = DEVICESTATUS::getInstance();
    ds.printStatus();
    return 0;
}

int CLI::recieveCMD(int argc, char **argv)
{
    E22 &e22 = E22::getInstance();

    e22.setUpForRx();
    printf("\n-- LORA RECEIVER --\n");

    e22.receivePacket(100000);

    while (e22.messageIsAvailable() == false)
    {
        printf("Waiting for packet...\n\r");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
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

    e22.setUpForTx();
    printf("\n-- LORA TRANSMITTER --\n\r");

    //-----------------------------------------------------------------------------------------------------------------

    // Transmit message and counter
    // write() method must be placed between beginPacket() and endPacket()
    static uint8_t counter = 0;
    // char message[] = "HeLoRa World!";
    char message[] = "Hola Cabrita! ";
    e22.beginTxPacket();
    e22.writeMessageTxLength((uint8_t *)message, sizeof(message) - 1);
    e22.writeMessageTxByte(counter);
    e22.transmitPacket(30000);

    // Print message and counter in serial
    printf("%s %u", message, counter);
    counter++;

    // Wait until modulation process for transmitting packet finish
    // LoRa.wait();

    // Print transmit time
    // Serial.print("Transmit time: ");
    // Serial.print(LoRa.transmitTime());
    // Serial.println(" ms");
    // Serial.println();

    // Don't load RF module with continous transmit
    // delay(5000);

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
    FS &fs = FS::getInstance();
    DEVICESTATUS &deviceStatus = DEVICESTATUS::getInstance();
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
            ESP_LOGI(CLITAG, "El comando config show necesita un argumento <section>.");
            return 0;
        }
        if (config_args.key->count == 0)
        {
            ESP_LOGI(CLITAG, "El comando config show necesita un argumento <key>.");
            return 0;
        }
        const char *section = config_args.section->sval[0];
        const char *key = config_args.key->sval[0];
        char value[INI_MAX_LEN];
        memset(value, 0, sizeof(value));
        if (fs.Ini_gets(section, key, value, DEVICE_CONFIG_FILE_PATH))
        {
            ESP_LOGI(CLITAG, "CONFIG [%s] %s=%s", section, key, value);
        }
        else
        {
            ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
        }

        return 0;
    }
    else if (!strcmp(command, "set"))
    {
        if (config_args.section->count == 0)
        {
            ESP_LOGI(CLITAG, "El comando config set necesita un argumento <section>.");
            return 0;
        }
        if (config_args.key->count == 0)
        {
            ESP_LOGI(CLITAG, "El comando config set necesita un argumento <key>.");
            return 0;
        }
        if (config_args.value->count == 0)
        {
            ESP_LOGI(CLITAG, "El comando config set necesita un argumento <value>.");
            return 0;
        }
        const char *section = config_args.section->sval[0];
        const char *key = config_args.key->sval[0];
        const char *value = config_args.value->sval[0];
        if (fs.Ini_puts(section, key, value, DEVICE_CONFIG_FILE_PATH))
        {
            ESP_LOGI(CLITAG, "CONFIG SET [%s] %s=%s", section, key, value);
        }
        else
        {
            ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
        }
        return 0;
    }
    else if (!strcmp(command, "delete"))
    {
        if (config_args.section->count == 0)
        {
            ESP_LOGI(CLITAG, "El comando config delete necesita un argumento <section>.");
            return 0;
        }
        if (config_args.key->count == 0)
        {
            ESP_LOGI(CLITAG, "El comando config delete necesita un argumento <key>.");
            return 0;
        }
        const char *section = config_args.section->sval[0];
        const char *key = config_args.key->sval[0];
        if (fs.Ini_puts(section, key, NULL, DEVICE_CONFIG_FILE_PATH))
        {
            ESP_LOGI(CLITAG, "CONFIG DELETE [%s] %s", section, key);
        }
        else
        {
            ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
        }
        return 0;
    }
    else if (!strcmp(command, "reset"))
    {
        ESP_LOGI(CLITAG, "CONFIG RESET");
        if (!fs.populateDeviceConfigIni())
        {
            ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
            return 0;
        }
        ESP_LOGI(CLITAG, "CONFIG RESET OK");
        
        return 0;
    }
    else if (!strcmp(command, "load"))
    {
        ESP_LOGI(CLITAG, "Config load");
        if (!deviceStatus.loadStatusFromFs())
        {
            ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
            return 0;
        }
        ESP_LOGI(CLITAG, "CONFIG LOAD OK");
        
        return 0;
    }
    else
    {
        ESP_LOGI(CLITAG, "Comando no valido");
        return 0;
    }

    return 0;
}

esp_err_t CLI::esp_console_register_FS_command(void)
{
    memset(&FS_args, 0, sizeof(FScmd_args_t));

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
        if (!strcmp(path, "storage"))
        {
            fs.formatPartition(STORAGE_PARTITION_NAME);
        }
        else
        {
            ESP_LOGI(CLITAG, "Comando no valido.");
        }
    }
    else if (!strcmp(command, "ls"))
    {
        if (FS_args.path->count == 0)
        {
            ESP_LOGI(CLITAG, "El comando fs ls necesita un argumento <path>.");
            return 0;
        }

        fs.ls(FS_args.path->sval[0]);
        return 0;
    }
    else if (!strcmp(command, "cat"))
    {
        fs.cat(FS_args.path->sval[0]);
    }
    else if (!strcmp(command, "catb"))
    {
        fs.catb(FS_args.path->sval[0]);
    }
    else
    {
        ESP_LOGI(CLITAG, "Comando no valido");
    }

    return 0;
}

esp_err_t CLI::esp_console_register_net_command(void)
{
    memset(&net_args, 0, sizeof(netcmd_args_t));

    net_args.command = arg_str1(NULL, NULL, "<funcion>", "Funcion del comando net.");
    net_args.ssid = arg_str0(NULL, NULL, "[ssid]", "Nombre de la red a conectar");
    net_args.pass = arg_str0(NULL, NULL, "[password]", "Password de la red a conectar");
    net_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "net",
        .help = "Comandos de control de la red",
        .hint = "<on|off|scan|connect|dc> [ssid] [password]",
        .func = &netCmdFunc,
        .argtable = &net_args};
    return esp_console_cmd_register(&cmd);
}

int CLI::netCmdFunc(int argc, char **argv)
{

    NETWORK &net = NETWORK::getInstance();
    int nerrors = arg_parse(argc, argv, (void **)&net_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, net_args.end, argv[0]);
        return 1;
    }
    const char *command = net_args.command->sval[0];
    if (!strcmp(command, "on"))
    {
        if (!net.startWifiStation())
        {
            ESP_LOGE(CLITAG, "Error al iniciar la red");
        }
        return 0;
    }
    else if (!strcmp(command, "off"))
    {
        if (!net.stopWifiStation())
        {
            ESP_LOGE(CLITAG, "Error al apagar la red");
        }
        return 0;
    }
    else if (!strcmp(command, "scan"))
    {
        if (!net.wifiScan())
        {
            ESP_LOGE(CLITAG, "Error al escanear la red");
        }
        return 0;
    }
    else if (!strcmp(command, "connect"))
    {
        if (net_args.ssid->count == 0)
        {
            ESP_LOGI(CLITAG, "El comando net connect necesita un argumento <ssid>.");
            return 0;
        }
        if (net_args.pass->count == 0)
        {
            ESP_LOGI(CLITAG, "El comando net connect necesita un argumento <pass>.");
            return 0;
        }
        if (!net.connectToNetwork(net_args.ssid->sval[0], strlen(net_args.ssid->sval[0]), net_args.pass->sval[0], strlen(net_args.pass->sval[0])))
        {
            ESP_LOGE(CLITAG, "Error al conectar a la red");
        }
        return 0;
    }
    else if (!strcmp(command, "dc"))
    {
        if (!net.disconnectFromNetwork())
        {
            ESP_LOGE(CLITAG, "Error al desconectar de la red");
        }
        return 0;
    }
    else
    {
        ESP_LOGI(CLITAG, "Comando no valido");
    }

    return 0;
}

esp_err_t CLI::esp_console_register_time_command(void)
{
    memset(&time_args, 0, sizeof(timecmd_args_t));

    time_args.command = arg_str1(NULL, NULL, "<funcion>", "Funcion del comando time.");
    time_args.ntpserver = arg_str0(NULL, NULL, "[ntpserver]", "Direccion del servidor NTP");
    time_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "time",
        .help = "Comandos de control del comando time",
        .hint = "<show|update|save|load> [ntpserver]",
        .func = &timeCmdFunc,
        .argtable = &time_args};
    return esp_console_cmd_register(&cmd);
}

int CLI::timeCmdFunc(int argc, char **argv)
{
    DEVICETIME &time = DEVICETIME::getInstance();
    int nerrors = arg_parse(argc, argv, (void **)&time_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, time_args.end, argv[0]);
        return 1;
    }
    const char *command = time_args.command->sval[0];
    if (!strcmp(command, "show"))
    {
        time.printTime();
        return 0;
    }
    else if (!strcmp(command, "update"))
    {
        time.updateTimeFromNet(time_args.ntpserver->sval[0], time_args.ntpserver->count);
        time.printTime();
        return 0;
    }
    else if (!strcmp(command, "save"))
    {
        time.saveTimeToFs();
        time.printTime();
        return 0;
    }
    else if (!strcmp(command, "load"))
    {
        time.loadTimeFromFs();
        time.printTime();
        return 0;
    }
    else
    {
        ESP_LOGI(CLITAG, "Comando no valido");
    }

    return 0;
}

esp_err_t CLI::esp_console_register_AutoJob_command(void)
{
    memset(&AJ_args, 0, sizeof(AJcmd_args_t));

    AJ_args.command = arg_str1(NULL, NULL, "<funcion>", "Funcion del comando aj.");
    AJ_args.job = arg_str0(NULL, NULL, "[jobname]", "Nombre del job");
    AJ_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "aj",
        .help = "Comandos de control del comando aj (AutomaticJob)",
        .hint = "<show|update|save|load> [ntpserver]",
        .func = &AutoJobCmdFunc,
        .argtable = &AJ_args};
    return esp_console_cmd_register(&cmd);
}

int CLI::AutoJobCmdFunc(int argc, char **argv)
{
    AUTOJOB &AJ = AUTOJOB::getInstance();
    int nerrors = arg_parse(argc, argv, (void **)&AJ_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, AJ_args.end, argv[0]);
        return 1;
    }
    const char *command = AJ_args.command->sval[0];
    if (!strcmp(command, "start"))
    {
        AJ.startJobs();
        return 0;
    }
    else if (!strcmp(command, "stop"))
    {
        AJ.stopJobs();
        return 0;
    }
    else if (!strcmp(command, "save"))
    {
        AJ.startJobs();
        return 0;
    }
    else if (!strcmp(command, "load"))
    {

        return 0;
    }
    else
    {
        ESP_LOGI(CLITAG, "Comando no valido");
    }

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
    esp_console_register_net_command();
    esp_console_register_time_command();
    esp_console_register_AutoJob_command();
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