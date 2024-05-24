#include "Message.h"
#include "FileSystem.h"

Message_t MESSAGE::processMessageSent(uint8_t arr[])
{
    Message_t aux;
    memset(&aux, 0, sizeof(Message_t));

    time_t timeAction = 0;
    time(&timeAction);
    aux.timeAction = timeAction;

    aux.ADCValue = (int32_t)arr[0] << 24 | (uint32_t)arr[1] << 16 | (uint32_t)arr[2] << 8 | (uint32_t)arr[3];

    aux.flags = arr[4];
    aux.messageId = (uint32_t)arr[5] << 24 | (uint32_t)arr[6] << 16 | (uint32_t)arr[7] << 8 | (uint32_t)arr[8];
    //memcpy(&aux.ADCValue, arr, sizeof(int32_t));
    //memcpy(&aux.flags, arr + 4, sizeof(uint8_t));
    //memcpy(&aux.messageId, arr + 5, sizeof(uint32_t));

    return aux;
}

Message_t MESSAGE::processMessageRecieved(uint8_t arr[])
{
    Message_t aux;
    memset(&aux, 0, sizeof(Message_t));

    time_t timeAction = 0;
    time(&timeAction);
    aux.timeAction = timeAction;

    memcpy(&aux.ADCValue, arr, sizeof(int32_t));
    memcpy(&aux.flags, arr + 4, sizeof(uint8_t));
    memcpy(&aux.messageId, arr + 4, sizeof(uint32_t));
    memcpy(&aux.RssiPkt, arr + 9, sizeof(uint8_t));
    memcpy(&aux.SnrPkt, arr + 10, sizeof(uint8_t));
    memcpy(&aux.SignalRssiPkt, arr + 11, sizeof(uint8_t));

    return aux;
}

bool MESSAGE::saveMessage(Message_t message)
{
    FS &fs = FS::getInstance();
    time_t auxTime = 0;
    time(&auxTime);
    message.timeAction = auxTime;

    if (!fs.WriteFile(&message, sizeof(Message_t), 1, PACKETS_BIN_PATH, "a+"))
    {
        ESP_LOGE(MESSAGETAG, "Error al guardar el valor del paquete");
        return false;
    }

    ESP_LOGI(MESSAGETAG, "Se guardó el paquete");
    return true;
}

bool MESSAGE::readAllPackets(void)
{
    FS &fs = FS::getInstance();
    Message_t aux;
    long sizeFile;
    fs.getFileSize(PACKETS_BIN_PATH, &sizeFile);

    if (sizeFile > 0)
    {
        uint32_t numPackets = sizeFile / sizeof(Message_t);
        for (uint32_t i = 0; i < numPackets; i++)
        {
            memset(&aux, 0, sizeof(Message_t));

            if (!fs.seekAndReadFile(PACKETS_BIN_PATH, &aux, sizeof(Message_t), i, SEEK_SET))
            {
                ESP_LOGE(MESSAGETAG, "Error al leer el paquete %ld", i);
                return false;
            }

            printf("Paquete guardado %lu: \r\n", i);
            printf("Tiempo: %llu\r\n", aux.timeAction);
            printf("ADC: %ld\r\n", aux.ADCValue);
            printf("Flags: %X\r\n", aux.flags);
            printf("ID: %lu\r\n", aux.messageId);
            printf("Rssi: %d\r\n", aux.RssiPkt);
            printf("Snr: %d\r\n", aux.SnrPkt);
            printf("SignalRssi: %d\r\n", aux.SignalRssiPkt);
        }
    }

    return true;
}