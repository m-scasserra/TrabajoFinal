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

    /**
     * @brief Initialize the network module
     * 
     * @return true If the network module was initialized correctly
     * @return false If there was an error initializing the network module
     */
    static bool Begin(void);

    /**
     * @brief Start the WiFi station
     * 
     * @return true If the WiFi station was started correctly
     * @return false If there was an error starting the WiFi station
     */
    static bool startWifiStation(void);

    /**
     * @brief Stop the WiFi station
     * 
     * @return true If the WiFi station was stopped correctly
     * @return false If there was an error stopping the WiFi station
     */
    static bool stopWifiStation(void);

    /**
     * @brief Scan for available networks
     * 
     * @return true If the scan was successful
     * @return false If there was an error scanning for networks
     */
    static bool wifiScan(void);

    /**
     * @brief Connect to a network
     * 
     * @param ssid SSID of the network
     * @param ssidLen Length of the SSID
     * @param password Password of the network
     * @param passwordLen Length of the password
     * @return true If the connection was successful
     * @return false If there was an error connecting to the network
     */
    static bool connectToNetwork(const char *ssid, uint32_t ssidLen, const char *password, uint32_t passwordLen);

    /**
     * @brief Disconnect from the network
     * 
     * @return true If the disconnection was successful
     * @return false If there was an error disconnecting from the network
     */
    static bool disconnectFromNetwork(void);

    /**
     * @brief Check if the device is connected to a network
     * 
     * @return true If the device is connected to a network
     * @return false If the device is not connected to a network
     */
    static bool isConnectedToNetwork(void);

private:
    // Constructor privado
    NETWORK() {}

    // Doxygen english documentation
    /**
     * @brief Print the SSID and the signal strength of the networks found
     * 
     * @param ssid SSID of the network
     * @param rssi Signal strength of the network
     */
    static void printAuthMode(int authmode);

    /**
     * @brief Print the SSID and the signal strength of the networks found
     * 
     * @param ssid SSID of the network
     * @param rssi Signal strength of the network
     */
    static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    /**
     * @brief Variable to check if the device is connected to a network
     */
    static bool connectedToNetwork;
    
    /**
     * @brief Variable to check if the WiFi has been started
     */
    static bool wifiStarted;
    
};

#endif // NETWORK_H