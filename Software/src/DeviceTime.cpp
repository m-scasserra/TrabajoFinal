#include <sys/time.h>
#include <string.h>

#include "DeviceTime.h"
#include "FileSystem.h"
#include "Network.h"

#include "esp_log.h"
#include "esp_netif_sntp.h"

bool DEVICETIME::Begin(void)
{
    FS &fs = FS::getInstance();
    
    // Set timezone
    setenv("TZ", (const char *)DEFAULT_TIMEZONE, 1);
    tzset();

    if (!fs.CheckFileExists(TIME_BIN_PATH))
    {
        ESP_LOGE(TIMETAG, "No se ha encontrado el archivo %s.", TIME_BIN_PATH);
        if (!saveTimeToFs())
        {
            ESP_LOGE(TIMETAG, "Error al guardar el tiempo");
        }
    }
    else
    {
        if (!loadTimeFromFs())
        {
            ESP_LOGE(TIMETAG, "Error al cargar el tiempo");
        }
    }
    return true;
}

bool DEVICETIME::updateTimeFromNet(const char *server, uint8_t serverLen)
{
    if (NETWORK::isConnectedToNetwork())
    {
        esp_sntp_config_t config;
        if (strncmp(server, "", serverLen) == 0)
        {
            ESP_LOGI(TIMETAG, "No se ha especificado el servidor NTP. Usando el servidor por defecto %s", DEFAULT_NTP_SERVER);
            config = ESP_NETIF_SNTP_DEFAULT_CONFIG((const char *)DEFAULT_NTP_SERVER);
        }
        else
        {
            config = ESP_NETIF_SNTP_DEFAULT_CONFIG(server);
        }
        esp_netif_sntp_init(&config);
        esp_netif_sntp_start();
        if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(WAIT_TIME_FOR_NTP_MS)) != ESP_OK)
        {
            ESP_LOGE(TIMETAG, "Error al sincronizar el tiempo");
            esp_netif_sntp_deinit();
            return false;
        }

        ESP_LOGI(TIMETAG, "Tiempo sincronizado");
        esp_netif_sntp_deinit();
        return true;
    }
    ESP_LOGE(TIMETAG, "No hay una red conectada");
    return false;
}

bool DEVICETIME::printTime(void)
{
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    time(&now);

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    printf("The current date/time in %s is: %s\r\n", DEFAULT_TIMEZONE, strftime_buf);
    return true;
}

bool DEVICETIME::saveTimeToFs(void)
{
    FS &fs = FS::getInstance();

    time_t now;
    time(&now);
    if (fs.WriteFile(&now, sizeof(now), 1, TIME_BIN_PATH, "w"))
    {
        ESP_LOGI(TIMETAG, "Tiempo guardado");
        return true;
    }
    ESP_LOGE(TIMETAG, "Error al guardar el tiempo");
    return false;
}


bool DEVICETIME::loadTimeFromFs(void)
{
    FS &fs = FS::getInstance();

    time_t now;
    if (fs.seekAndReadFile(TIME_BIN_PATH, &now, sizeof(now), 0, SEEK_SET))
    {
        ESP_LOGI(TIMETAG, "Tiempo cargado");
        struct timeval tv;
        tv.tv_sec = now;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
        return true;
    }
    ESP_LOGE(TIMETAG, "Error al cargar el tiempo");
    return false;
}