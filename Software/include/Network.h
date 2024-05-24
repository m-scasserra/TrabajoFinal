#ifndef NETWORK_H
#define NETWORK_H

#include "CommonIncludes.h"

#include "esp_event.h"

#define NETWORKTAG "NET"
#define MAX_NUMBER_NETOWRK_SCAN 10

class NETWORK
{

public:
    // Eliminar las funciones de copia y asignación
    NETWORK(const NETWORK &) = delete;
    NETWORK &operator=(const NETWORK &) = delete;


    static NETWORK &getInstance()
    {
        static NETWORK instance; // Única instancia
        return instance;
    }

    static bool Begin(void);
    static bool startWifiStation(void);
    static bool stopWifiStation(void);
    static bool wifiScan(void);
    static bool connectToNetwork(const char *ssid, uint32_t ssidLen, const char *password, uint32_t passwordLen);
    static bool disconnectFromNetwork(void);
    static bool isConnectedToNetwork(void);

private:
    // Constructor privado
    NETWORK() {}

    static void printAuthMode(int authmode);
    static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    static bool connectedToNetwork;
    static bool wifiStarted;
    
};

#endif // NETWORK_H