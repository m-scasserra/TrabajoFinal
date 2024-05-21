#include <string.h>

#include "DeviceStatus.h"
#include "FileSystem.h"

#include "esp_log.h"


DEVICESTATUS::DeviceStatus_t DEVICESTATUS::deviceStatus;

bool DEVICESTATUS::Begin(void)
{
    memset(&deviceStatus, 0, sizeof(deviceStatus));
    return true;
}

bool DEVICESTATUS::printStatus(void)
{
    printf("E22Status\r\n");
    printf("\ttcxoVoltage: 0x%02X\r\n", deviceStatus.E22Status.tcxoVoltage);
    printf("\tTXCOdio3Delay: %lu\r\n", deviceStatus.E22Status.TXCOdio3Delay);
    printf("\tpacketParams: preambleLength: %d\r\n", deviceStatus.E22Status.packetParams.preambleLength);
    printf("\tpacketParams: headerType: 0x%02X\r\n", deviceStatus.E22Status.packetParams.headerType);
    printf("\tpacketParams: payloadLength: %d\r\n", deviceStatus.E22Status.packetParams.payloadLength);
    printf("\tpacketParams: crcType: 0x%02X\r\n", deviceStatus.E22Status.packetParams.crcType);
    printf("\tpacketParams: iqType: 0x%02X\r\n", deviceStatus.E22Status.packetParams.iqType);
    printf("\tmodulationParams: spreadingFactor: 0x%02X\r\n", deviceStatus.E22Status.modulationParams.spreadingFactor);
    printf("\tmodulationParams: bandwidth: 0x%02X\r\n", deviceStatus.E22Status.modulationParams.bandwidth);
    printf("\tmodulationParams: codingRate: 0x%02X\r\n", deviceStatus.E22Status.modulationParams.codingRate);
    printf("\trxGain: 0x%02X\r\n", deviceStatus.E22Status.rxGain);
    printf("\timageCalibrationFreq: 0x%02X\r\n", deviceStatus.E22Status.imageCalibrationFreq);
    printf("\tpacketType: 0x%02X\r\n", deviceStatus.E22Status.packetType);
    printf("\tcalibrations: RC64kCalibration: 0x%02X\r\n", deviceStatus.E22Status.calibrations.RC64kCalibration);
    printf("\tcalibrations: RC13MCalibration: 0x%02X\r\n", deviceStatus.E22Status.calibrations.RC13MCalibration);
    printf("\tcalibrations: PLLCalibration: 0x%02X\r\n", deviceStatus.E22Status.calibrations.PLLCalibration);
    printf("\tcalibrations: ADCPulseCalibration: 0x%02X\r\n", deviceStatus.E22Status.calibrations.ADCPulseCalibration);
    printf("\tcalibrations: ADCBulkNCalibration: 0x%02X\r\n", deviceStatus.E22Status.calibrations.ADCBulkNCalibration);
    printf("\tcalibrations: ADCBulkPCalibration: 0x%02X\r\n", deviceStatus.E22Status.calibrations.ADCBulkPCalibration);
    printf("\tcalibrations: ImageCalibration: 0x%02X\r\n", deviceStatus.E22Status.calibrations.ImageCalibration);
    printf("\tfrequency: %lu\r\n", deviceStatus.E22Status.frequency);
    printf("\txtalConfig: USE: 0x%02X\r\n", deviceStatus.E22Status.xtalConfig.use);
    printf("\txtalConfig: XTALA: 0x%02X\r\n", deviceStatus.E22Status.xtalConfig.XTALA);
    printf("\txtalConfig: XTALB: 0x%02X\r\n", deviceStatus.E22Status.xtalConfig.XTALB);
    printf("\tpaConfig: 0x%02X\r\n", deviceStatus.E22Status.paConfig);
    printf("\trampTime: 0x%02X\r\n", deviceStatus.E22Status.rampTime);
    printf("\tsyncWord: 0x%02X\r\n", deviceStatus.E22Status.syncWord);
    printf("\ttransmitTimeout: %lu\r\n", deviceStatus.E22Status.transmitTimeout);
    printf("\trecieveTimeout: %lu\r\n", deviceStatus.E22Status.recieveTimeout);

    printf("DEVICE: AUTOMATIC: %s\r\n", deviceStatus.automatic ? "ON" : "OFF");
    printf("DEVICE: FUNCTION: %02X\r\n", deviceStatus.mode);
    return true;
}

bool DEVICESTATUS::loadStatusFromFs(void)
{
    if (!loadDeviceFromFs())
        return false;
    if (!loadPacketTypeFromFs())
        return false;
    if (!loadTCXOFromFs())
        return false;
    if (!loadCalibrationsFromFs())
        return false;
    if (!loadXTALConfigFromFs())
        return false;
    if (!loadFrequencyFromFs())
        return false;
    if (!loadPAConfigFromFs())
        return false;
    if (!loadTxParamsFromFs())
        return false;
    if (!loadModulationParamsFromFs())
        return false;
    if (!loadPacketParamsFromFs())
        return false;
    if (!loadSyncWordFromFs())
        return false;
    if (!loadRxGainFromFs())
        return false;
    if (!loadTimeoutFromFs())
        return false;
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
        }
        else if (!strcmp(value, "OFF"))
        {
            deviceStatus.automatic = false;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de DEVICE AUTOMATIC no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }

    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("DEVICE", "FUNCTION", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "NONE"))
        {
            deviceStatus.mode = NONE;
        }
        else if (!strcmp(value, "TX"))
        {
            deviceStatus.mode = TX;
        }
        else if (!strcmp(value, "RX"))
        {
            deviceStatus.mode = RX;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de DEVICE FUNCTION no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
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
        }
        else if (!strcmp(value, "GFSK"))
        {
            deviceStatus.E22Status.packetType = E22::PACKET_TYPE_GFSK;
        }
        else if (!strcmp(value, "LRFHSS"))
        {
            deviceStatus.E22Status.packetType = E22::PACKET_TYPE_LR_FHSS;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de  PACKETTYPE TYPE no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
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
        }
        else if (!strcmp(value, "1_7"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_1_7;
        }
        else if (!strcmp(value, "1_8"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_1_8;
        }
        else if (!strcmp(value, "2_2"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_2_2;
        }
        else if (!strcmp(value, "2_4"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_2_4;
        }
        else if (!strcmp(value, "2_7"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_2_7;
        }
        else if (!strcmp(value, "3_0"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_3_0;
        }
        else if (!strcmp(value, "3_3"))
        {
            deviceStatus.E22Status.tcxoVoltage = E22::TCXOVOLTAGE_3_3;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de TCXO DIO3VOLTAGE no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }

    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("TCXO", "DIO3DELAY", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.TXCOdio3Delay = atoi(value);
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
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
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
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
        }
        else if (!strcmp(value, "N"))
        {
            deviceStatus.E22Status.xtalConfig.use = false;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de XTAL USE no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }

    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("XTAL", "A", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.xtalConfig.XTALA = (int)strtol(value, NULL, 0);
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }

    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("XTAL", "B", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.xtalConfig.XTALB = (int)strtol(value, NULL, 0);
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
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
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
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
        }
        else if (!strcmp(value, "17"))
        {
            deviceStatus.E22Status.paConfig = E22::PA_17_DBM;
        }
        else if (!strcmp(value, "20"))
        {
            deviceStatus.E22Status.paConfig = E22::PA_20_DBM;
        }
        else if (!strcmp(value, "22"))
        {
            deviceStatus.E22Status.paConfig = E22::PA_22_DBM;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de PACONFIG DBM no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
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
        }
        else if (!strcmp(value, "20"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_20U;
        }
        else if (!strcmp(value, "40"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_40U;
        }
        else if (!strcmp(value, "80"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_80U;
        }
        else if (!strcmp(value, "200"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_200U;
        }
        else if (!strcmp(value, "800"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_800U;
        }
        else if (!strcmp(value, "1700"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_1700U;
        }
        else if (!strcmp(value, "3400"))
        {
            deviceStatus.E22Status.rampTime = E22::SET_RAMP_3400U;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de TXPARAMS RAMPTIME no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
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
            deviceStatus.E22Status.modulationParams.spreadingFactor = E22::SF_5;
        }
        else if (!strcmp(value, "6"))
        {
            deviceStatus.E22Status.modulationParams.spreadingFactor = E22::SF_6;
        }
        else if (!strcmp(value, "7"))
        {
            deviceStatus.E22Status.modulationParams.spreadingFactor = E22::SF_7;
        }
        else if (!strcmp(value, "8"))
        {
            deviceStatus.E22Status.modulationParams.spreadingFactor = E22::SF_8;
        }
        else if (!strcmp(value, "9"))
        {
            deviceStatus.E22Status.modulationParams.spreadingFactor = E22::SF_9;
        }
        else if (!strcmp(value, "10"))
        {
            deviceStatus.E22Status.modulationParams.spreadingFactor = E22::SF_10;
        }
        else if (!strcmp(value, "11"))
        {
            deviceStatus.E22Status.modulationParams.spreadingFactor = E22::SF_11;
        }
        else if (!strcmp(value, "12"))
        {
            deviceStatus.E22Status.modulationParams.spreadingFactor = E22::SF_12;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de MODULATION SF no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }

    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("MODULATION", "BW", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "7"))
        {
            deviceStatus.E22Status.modulationParams.bandwidth = E22::LORA_BW_7;
        }
        else if (!strcmp(value, "10"))
        {
            deviceStatus.E22Status.modulationParams.bandwidth = E22::LORA_BW_10;
        }
        else if (!strcmp(value, "15"))
        {
            deviceStatus.E22Status.modulationParams.bandwidth = E22::LORA_BW_15;
        }
        else if (!strcmp(value, "20"))
        {
            deviceStatus.E22Status.modulationParams.bandwidth = E22::LORA_BW_20;
        }
        else if (!strcmp(value, "31"))
        {
            deviceStatus.E22Status.modulationParams.bandwidth = E22::LORA_BW_31;
        }
        else if (!strcmp(value, "41"))
        {
            deviceStatus.E22Status.modulationParams.bandwidth = E22::LORA_BW_41;
        }
        else if (!strcmp(value, "62"))
        {
            deviceStatus.E22Status.modulationParams.bandwidth = E22::LORA_BW_62;
        }
        else if (!strcmp(value, "125"))
        {
            deviceStatus.E22Status.modulationParams.bandwidth = E22::LORA_BW_125;
        }
        else if (!strcmp(value, "250"))
        {
            deviceStatus.E22Status.modulationParams.bandwidth = E22::LORA_BW_250;
        }
        else if (!strcmp(value, "500"))
        {
            deviceStatus.E22Status.modulationParams.bandwidth = E22::LORA_BW_500;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de MODULATION BW no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }

    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("MODULATION", "CR", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "4/5"))
        {
            deviceStatus.E22Status.modulationParams.codingRate = E22::LORA_CR_4_5;
        }
        else if (!strcmp(value, "4/6"))
        {
            deviceStatus.E22Status.modulationParams.codingRate = E22::LORA_CR_4_6;
        }
        else if (!strcmp(value, "4/7"))
        {
            deviceStatus.E22Status.modulationParams.codingRate = E22::LORA_CR_4_7;
        }
        else if (!strcmp(value, "4/8"))
        {
            deviceStatus.E22Status.modulationParams.codingRate = E22::LORA_CR_4_8;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de MODULATION CR no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
}

bool DEVICESTATUS::loadPacketParamsFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("PACKETPARAMS", "HEADER", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "EXPLICIT"))
        {
            deviceStatus.E22Status.packetParams.headerType = E22::EXPLICIT_HEADER;
        }
        else if (!strcmp(value, "IMPLICIT"))
        {
            deviceStatus.E22Status.packetParams.headerType = E22::IMPLICIT_HEADER;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de PACKETPARAMS HEADER no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }

    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("PACKETPARAMS", "PREAMBLELENGTH", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.packetParams.preambleLength = atoi(value);
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }

    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("PACKETPARAMS", "PAYLOADLENGTH", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.packetParams.payloadLength = atoi(value);
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }

    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("PACKETPARAMS", "CRC", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "ON"))
        {
            deviceStatus.E22Status.packetParams.crcType = true;
        }
        else if (!strcmp(value, "OFF"))
        {
            deviceStatus.E22Status.packetParams.crcType = false;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de PACKETPARAMS CRC no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
}

bool DEVICESTATUS::loadSyncWordFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("SYNCWORD", "SYNCWORD", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "0x3444"))
        {
            deviceStatus.E22Status.syncWord = E22::PUBLIC_SYNCWORD;
        }
        else if (!strcmp(value, "0x1424"))
        {
            deviceStatus.E22Status.syncWord = E22::PRIVATE_SYNCWORD;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de SYNCWORD SYNCWORD no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
}

bool DEVICESTATUS::loadRxGainFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("RXGAIN", "RXGAIN", value, DEVICE_CONFIG_FILE_PATH))
    {
        if (!strcmp(value, "BOOST"))
        {
            deviceStatus.E22Status.rxGain = E22::RX_BOOST;
        }
        else if (!strcmp(value, "POWERSAVE"))
        {
            deviceStatus.E22Status.rxGain = E22::RX_POWER_SAVE;
        }
        else
        {
            ESP_LOGE(STATUSTAG, "Valor de RXGAIN RXGAIN no reconocido.");
            return false;
        }
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
}

bool DEVICESTATUS::loadTimeoutFromFs(void)
{
    FS &fs = FS::getInstance();
    char value[INI_MAX_LEN];
    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("TIMEOUT", "TRANSMIT", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.transmitTimeout = atoi(value);
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }

    memset(value, 0, sizeof(value));
    if (fs.Ini_gets("TIMEOUT", "RECIEVE", value, DEVICE_CONFIG_FILE_PATH))
    {
        deviceStatus.E22Status.recieveTimeout = atoi(value);
    }
    else
    {
        ESP_LOGE(STATUSTAG, "Error al escribir/leer el config.");
        return false;
    }
    return true;
}
