#include "DeviceStatus.h"

DEVICESTATUS::DeviceStatus_t DEVICESTATUS::deviceStatus;

bool DEVICESTATUS::Begin(void)
{
    memset(&deviceStatus, 0, sizeof(deviceStatus));
}

bool DEVICESTATUS::loadStatusFromFs(void)
{
    loadDeviceFromFs();
    loadPacketTypeFromFs();
    loadTCXOFromFs();
    loadCalibrationsFromFs();
    loadXTALConfigFromFs();
    loadFrequencyFromFs();
    loadPAConfigFromFs();
    loadTxParamsFromFs();
    loadModulationParamsFromFs();
    loadPacketParamsFromFs();
    loadSyncWordFromFs();
    loadRxGainFromFs();
    loadTimeoutFromFs();
    return true;
}

bool DEVICESTATUS::loadDeviceFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("DEVICE", "AUTOMATIC", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "ON"))
        {
            deviceStatus.automatic = true;
            return true;
        }
        else if (!strcmp(value, "OFF"))
        {
            deviceStatus.automatic = false;
            return true;
        }
        else
        {
            ESP_LOGE(CLITAG, "Valor de DEVICE AUTOMATIC no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
        return false;
    }
}

bool DEVICESTATUS::loadPacketTypeFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("PACKETTYPE", "TYPE", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "LORA"))
        {
            deviceStatus.E22Status.packetType = E22::PACKET_TYPE_LORA;
            return true;
        }
        else if (!strcmp(value, "GFSK"))
        {
            deviceStatus.E22Status.packetType = E22::PACKET_TYPE_GFSK;
            return true;
        }
        else if (!strcmp(value, "LRFHSS"))
        {
            deviceStatus.E22Status.packetType = E22::PACKET_TYPE_LR_FHSS;
            return true;
        }
        else
        {
            ESP_LOGE(CLITAG, "Valor de  PACKETTYPE TYPE no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}

bool DEVICESTATUS::loadTCXOFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("TCXO", "DIO3VOLTAGE", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "1_6"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_1_6;
            return true;
        }
        else if (!strcmp(value, "1_7"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_1_7;
            return true;
        }
        else if (!strcmp(value, "1_8"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_1_8;
            return true;
        }
        else if (!strcmp(value, "2_2"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_2_2;
            return true;
        }
        else if (!strcmp(value, "2_4"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_2_4;
            return true;
        }
        else if (!strcmp(value, "2_7"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_2_7;
            return true;
        }
        else if (!strcmp(value, "3_0"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_3_0;
            return true;
        }
        else if (!strcmp(value, "3_3"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_3_3;
            return true;
        }
        else
        {
            ESP_LOGE(CLITAG, "Valor de TCXO DIO3VOLTAGE no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }

    if (fs.Ini_gets("TCXO", "DIO3DELAY", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.TXCOdio3Delay = atoi(value);
        return true;
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}

bool DEVICESTATUS::loadCalibrationsFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("CALIBRATIONS", "DO", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.calibrations = E22::calibrationsFromMask((int)strtol(value, NULL, 0));
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}

bool DEVICESTATUS::loadXTALConfigFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("XTAL", "USE", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "Y"))
        {
            deviceStatus.E22Status.xtalConfig.use = true;
            return true;
        }
        else if (!strcmp(value, "N"))
        {
            deviceStatus.E22Status.xtalConfig.use = false;
            return true;
        }
        else
        {
            ESP_LOGE(CLITAG, "Valor de XTAL USE no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }

    if (fs.Ini_gets("XTAL", "A", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.xtalConfig.XTALA = (int)strtol(value, NULL, 0);
        return true;
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }

    if (fs.Ini_gets("XTAL", "B", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.xtalConfig.XTALB = (int)strtol(value, NULL, 0);
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}

bool DEVICESTATUS::loadFrequencyFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("FREQUENCY", "FREQUENCY", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.frequency = atoi(value);
        calculateImageCalibrationFrequency(deviceStatus.E22Status.frequency);
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}

void DEVICESTATUS::calculateImageCalibrationFrequency(uint32_t frequency)
{
    if (frequency < 446000000) // 430 - 440 Mhz
    {
        deviceStatus.E22Status.imageCalibrationFreq = E22::FREQ_430_440;
    }
    else if (frequency < 734000000) // 470 - 510 Mhz
    {
        deviceStatus.E22Status.imageCalibrationFreq = E22::FREQ_470_510;
    }
    else if (frequency < 828000000) // 779 - 787 Mhz
    {
        deviceStatus.E22Status.imageCalibrationFreq = E22::FREQ_779_787;
    }
    else if (frequency < 877000000) // 863 - 870 Mhz
    {
        deviceStatus.E22Status.imageCalibrationFreq = E22::FREQ_863_870;
    }
    else if (frequency < 1100000000) // 902 - 928 Mhz
    {
        deviceStatus.E22Status.imageCalibrationFreq = E22::FREQ_902_928;
    }
}

bool DEVICESTATUS::loadPAConfigFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("PACONFIG", "DBM", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "14"))
        {
            deviceStatus.E22Status.paConfig = E22::PA_14_DBM;
            return true;
        }
        else if (!strcmp(value, "17"))
        {
            deviceStatus.E22Status.paConfig = E22::PA_17_DBM;
            return true;
        }
        else if (!strcmp(value, "20"))
        {
            deviceStatus.E22Status.paConfig = E22::PA_20_DBM;
            return true;
        }
        else if (!strcmp(value, "22"))
        {
            deviceStatus.E22Status.paConfig = E22::PA_22_DBM;
            return true;
        }
        else
        {
            ESP_LOGE(CLITAG, "Valor de PACONFIG DBM no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}

bool DEVICESTATUS::loadTxParamsFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("TXPARAMS", "RAMPTIME", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "10"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_10U;
            return true;
        }
        else if (!strcmp(value, "20"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_20U;
            return true;
        }
        else if (!strcmp(value, "40"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_40U;
            return true;
        }
        else if (!strcmp(value, "80"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_80U;
            return true;
        }
        else if (!strcmp(value, "200"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_200U;
            return true;
        }
        else if (!strcmp(value, "800"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_800U;
            return true;
        }
        else if (!strcmp(value, "1700"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_1700U;
            return true;
        }
        else if (!strcmp(value, "3400"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_3400U;
            return true;
        }
        else
        {
            ESP_LOGE(CLITAG, "Valor de TXPARAMS RAMPTIME no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}

bool DEVICESTATUS::loadModulationParamsFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("MODULATION", "SF", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "5"))
        {
            deviceStatus.E22Status.modulationParams.spredingFactor = E22::SF_5;
            return true;
        }
        else if (!strcmp(value, "6"))
        {
            deviceStatus.E22Status.modulationParams.spredingFactor = E22::SF_6;
            return true;
        }
        else if (!strcmp(value, "7"))
        {
            deviceStatus.E22Status.modulationParams.spredingFactor = E22::SF_7;
            return true;
        }
        else if (!strcmp(value, "8"))
        {
            deviceStatus.E22Status.modulationParams.spredingFactor = E22::SF_8;
            return true;
        }
        else if (!strcmp(value, "9"))
        {
            deviceStatus.E22Status.modulationParams.spredingFactor = E22::SF_9;
            return true;
        }
        else if (!strcmp(value, "10"))
        {
            deviceStatus.E22Status.modulationParams.spredingFactor = E22::SF_10;
            return true;
        }
        else if (!strcmp(value, "11"))
        {
            deviceStatus.E22Status.modulationParams.spredingFactor = E22::SF_11;
            return true;
        }
        else if (!strcmp(value, "12"))
        {
            deviceStatus.E22Status.modulationParams.spredingFactor = E22::SF_12;
            return true;
        }
        else
        {
            ESP_LOGE(CLITAG, "Valor de MODULATION SF no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}

bool DEVICESTATUS::loadPacketParamsFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("DEVICE", "AUTOMATIC", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "ON"))
        {
            deviceStatus.automatic = true;
            return true;
        }
        else if (!strcmp(value, "OFF"))
        {
            deviceStatus.automatic = false;
            return true;
        }
        else
        {
            ESP_LOGE(CLITAG, "Valor de DEVICE AUTOMATIC no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}

bool DEVICESTATUS::loadSyncWordFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("DEVICE", "AUTOMATIC", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "ON"))
        {
            deviceStatus.automatic = true;
            return true;
        }
        else if (!strcmp(value, "OFF"))
        {
            deviceStatus.automatic = false;
            return true;
        }
        else
        {
            ESP_LOGE(CLITAG, "Valor de DEVICE AUTOMATIC no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}

bool DEVICESTATUS::loadRxGainFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("DEVICE", "AUTOMATIC", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "ON"))
        {
            deviceStatus.automatic = true;
            return true;
        }
        else if (!strcmp(value, "OFF"))
        {
            deviceStatus.automatic = false;
            return true;
        }
        else
        {
            ESP_LOGE(CLITAG, "Valor de DEVICE AUTOMATIC no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}

bool DEVICESTATUS::loadTimeoutFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("DEVICE", "AUTOMATIC", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "ON"))
        {
            deviceStatus.automatic = true;
            return true;
        }
        else if (!strcmp(value, "OFF"))
        {
            deviceStatus.automatic = false;
            return true;
        }
        else
        {
            ESP_LOGE(CLITAG, "Valor de DEVICE AUTOMATIC no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(CLITAG, "Error al escribir/leer el config.");
    }
    return false;
}
