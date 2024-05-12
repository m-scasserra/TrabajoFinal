#include "Network.h"

bool NETWORK::connectedToNetwork = false;
bool NETWORK::wifiStarted = false;

void NETWORK::wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_CONNECTED:
    {
        ESP_LOGI(NETWORKTAG, "Event: Connected to AP");
        connectedToNetwork = true;
    }
    break;

    case WIFI_EVENT_STA_DISCONNECTED:
    {
        ESP_LOGI(NETWORKTAG, "Event: Disconnected from AP");
        connectedToNetwork = false;
    }
    break;

    case IP_EVENT_STA_GOT_IP:
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(NETWORKTAG, "Event: Got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        connectedToNetwork = true;
    }
    break;

    default:
    {
        ESP_LOGD(NETWORKTAG, "Unhandled event: %li", event_id);
    }
    break;
    }
}

bool NETWORK::Begin(void)
{
    esp_log_level_set("wifi", ESP_LOG_NONE);
    esp_log_level_set("wifi_init", ESP_LOG_NONE);
    esp_log_level_set("phy_init", ESP_LOG_NONE);

    ESP_LOGI(NETWORKTAG, "Initializing NVS");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGI(NETWORKTAG, "nvs_flash_init: Erasing");
        if (nvs_flash_erase() != ESP_OK)
        {
            ESP_LOGE(NETWORKTAG, "nvs_flash_erase failed");
            return false;
        }
        ESP_LOGI(NETWORKTAG, "nvs_flash_init: Re-initializing");
        if (nvs_flash_init() != ESP_OK)
        {
            ESP_LOGE(NETWORKTAG, "nvs_flash_init failed again");
            return false;
        }
    }

    if (esp_netif_init() != ESP_OK)
    {
        ESP_LOGE(NETWORKTAG, "esp_netif_init failed");
        return false;
    }

    ESP_LOGI(NETWORKTAG, "Starting event loop...");
    if (esp_event_loop_create_default() != ESP_OK)
    {
        ESP_LOGE(NETWORKTAG, "esp_event_loop_create_default failed");
        return false;
    }

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    ESP_LOGD(NETWORKTAG, "Initializing Wi-Fi driver...");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK)
    {
        ESP_LOGE(NETWORKTAG, "esp_wifi_init failed");
        return false;
    }

    ESP_LOGI(NETWORKTAG, "Setting Wi-Fi mode to station...");
    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK)
    {
        ESP_LOGE(NETWORKTAG, "esp_wifi_set_mode failed");
        return false;
    }

    return true;
}

bool NETWORK::startWifiStation(void)
{
    if (wifiStarted == true)
    {
        ESP_LOGE(NETWORKTAG, "Wi-Fi already started");
        return false;
    }
    ESP_LOGI(NETWORKTAG, "Starting Wi-Fi station...");
    if (esp_wifi_start() != ESP_OK)
    {
        ESP_LOGE(NETWORKTAG, "esp_wifi_start failed");
        return false;
    }
    ESP_LOGI(NETWORKTAG, "Wi-Fi station started");

    wifiStarted = true;
    return true;
}

bool NETWORK::stopWifiStation(void)
{
    ESP_LOGI(NETWORKTAG, "Stopping Wi-Fi station...");
    if (esp_wifi_stop() != ESP_OK)
    {
        ESP_LOGE(NETWORKTAG, "esp_wifi_stop failed");
        return false;
    }
    ESP_LOGI(NETWORKTAG, "Wi-Fi station stopped");

    wifiStarted = false;
    return true;
}

bool NETWORK::wifiScan(void)
{
    if (wifiStarted == false)
    {
        ESP_LOGE(NETWORKTAG, "Wi-Fi not started");
        return false;
    }

    uint16_t number = MAX_NUMBER_NETOWRK_SCAN;
    wifi_ap_record_t ap_info[MAX_NUMBER_NETOWRK_SCAN];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    ESP_LOGI(NETWORKTAG, "Starting Wi-Fi scan...");
    esp_wifi_scan_start(NULL, true);
    ESP_LOGI(NETWORKTAG, "Finished Wi-Fi scan");

    if (esp_wifi_scan_get_ap_records(&number, ap_info) != ESP_OK)
    {
        ESP_LOGE(NETWORKTAG, "esp_wifi_scan_get_ap_records failed");
        return false;
    }

    if (esp_wifi_scan_get_ap_num(&ap_count) != ESP_OK)
    {
        ESP_LOGE(NETWORKTAG, "esp_wifi_scan_get_ap_num failed");
        return false;
    }

    ESP_LOGD(NETWORKTAG, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);

    for (int i = 0; i < number; i++)
    {
        printf("SSID %s\t\t", ap_info[i].ssid);
        printf("RSSI %d\t\t", ap_info[i].rssi);
        printAuthMode(ap_info[i].authmode);
    }

    return true;
}

void NETWORK::printAuthMode(int authmode)
{
    switch (authmode)
    {
    case WIFI_AUTH_OPEN:
        printf("Authmode WIFI_AUTH_OPEN\n\r");
        break;
    case WIFI_AUTH_OWE:
        printf("Authmode WIFI_AUTH_OWE\n\r");
        break;
    case WIFI_AUTH_WEP:
        printf("Authmode WIFI_AUTH_WEP\n\r");
        break;
    case WIFI_AUTH_WPA_PSK:
        printf("Authmode WIFI_AUTH_WPA_PSK\n\r");
        break;
    case WIFI_AUTH_WPA2_PSK:
        printf("Authmode WIFI_AUTH_WPA2_PSK\n\r");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        printf("Authmode WIFI_AUTH_WPA_WPA2_PSK\n\r");
        break;
    case WIFI_AUTH_ENTERPRISE:
        printf("Authmode WIFI_AUTH_ENTERPRISE\n\r");
        break;
    case WIFI_AUTH_WPA3_PSK:
        printf("Authmode WIFI_AUTH_WPA3_PSK\n\r");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        printf("Authmode WIFI_AUTH_WPA2_WPA3_PSK\n\r");
        break;
    case WIFI_AUTH_WPA3_ENT_192:
        printf("Authmode WIFI_AUTH_WPA3_ENT_192\n\r");
        break;
    default:
        printf("Authmode WIFI_AUTH_UNKNOWN\n\r");
        break;
    }
}

bool NETWORK::connectToNetwork(const char *ssid, uint32_t ssidLen, const char *password, uint32_t passwordLen)
{
    ESP_LOGI(NETWORKTAG, "Connecting to %s", ssid);

    if (wifiStarted == false)
    {
        ESP_LOGE(NETWORKTAG, "Wi-Fi not started");
        return false;
    }

    if (connectedToNetwork == true)
    {
        ESP_LOGE(NETWORKTAG, "Already connected to network");
        return false;
    }

    wifi_config_t wifiConfig;
    memset(&wifiConfig, 0, sizeof(wifi_config_t));

    if (ssidLen > sizeof(wifiConfig.sta.ssid) || passwordLen > sizeof(wifiConfig.sta.password))
    {
        ESP_LOGE(NETWORKTAG, "SSID or password too long");
        return false;
    }

    strncpy((char *)wifiConfig.sta.ssid, ssid, ssidLen);
    strncpy((char *)wifiConfig.sta.password, password, passwordLen);

    if (esp_wifi_set_config(WIFI_IF_STA, &wifiConfig) != ESP_OK)
    {
        ESP_LOGE(NETWORKTAG, "esp_wifi_set_config failed");
        return false;
    }

    if (esp_wifi_connect() != ESP_OK)
    {
        ESP_LOGE(NETWORKTAG, "esp_wifi_connect failed");
        return false;
    }

    return true;
}

bool NETWORK::disconnectFromNetwork(void)
{
    if (wifiStarted == false)
    {
        ESP_LOGE(NETWORKTAG, "Wi-Fi not started");
        return false;
    }

    if (connectedToNetwork == false)
    {
        ESP_LOGE(NETWORKTAG, "Not connected to network");
        return false;
    }

    ESP_LOGI(NETWORKTAG, "Disconnecting from network");

    if (esp_wifi_disconnect() != ESP_OK)
    {
        ESP_LOGE(NETWORKTAG, "esp_wifi_disconnect failed");
        return false;
    }

    connectedToNetwork = false;
    return true;
}

bool NETWORK::isConnectedToNetwork(void)
{
    return connectedToNetwork;
}