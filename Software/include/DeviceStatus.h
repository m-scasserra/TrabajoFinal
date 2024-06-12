/**
 * @file DeviceStatus.h
 * @brief Contains the declaration of the DEVICESTATUS class and related structs.
 */

#ifndef DEVICESTATUS_H
#define DEVICESTATUS_H

#include "CommonIncludes.h"
#include "E22Driver.h"

#define STATUSTAG "DEVICESTATUS"

/**
 * @class DEVICESTATUS
 * @brief Represents the device status and provides methods for initializing, printing, and loading the status.
 *
 * The `DEVICESTATUS` class is a singleton class that ensures only one instance of the device status exists.
 * It provides a static method `getInstance()` to access the instance of the `DEVICESTATUS` class.
 * The class also defines several nested structs, including `E22Status_t` and `DeviceStatus_t`, which represent
 * different aspects of the device status.
 *
 * The `DEVICESTATUS` class provides the following public methods:
 * - `bool Begin()`: Initializes the device status.
 * - `bool printStatus()`: Prints the device status.
 * - `bool loadStatusFromFs()`: Loads the status of the device from the file system.
 *
 * The `DEVICESTATUS` class also provides several private methods for loading specific aspects of the device status
 * from the file system.
 *
 * The `DEVICESTATUS` class follows the singleton design pattern and prohibits copying and assignment of instances.
 */
class DEVICESTATUS
{

public:
    // Eliminar las funciones de copia y asignación
    DEVICESTATUS(const DEVICESTATUS &) = delete;
    DEVICESTATUS &operator=(const DEVICESTATUS &) = delete;

    /**
     * @brief Gets the instance of the `DEVICESTATUS` class.
     *
     * @return The instance of the `DEVICESTATUS` class.
     */
    static DEVICESTATUS &getInstance()
    {
        static DEVICESTATUS instance; // Única instancia
        return instance;
    }

    /**
     * @struct XTalConfig_t
     * @brief Represents the crystal oscillator configuration.
     */
    typedef struct
    {
        bool use;      ///< Indicates whether the crystal oscillator is used.
        uint8_t XTALA; ///< XTAL A configuration value.
        uint8_t XTALB; ///< XTAL B configuration value.
    } XTalConfig_t;

    /**
     * @enum DeviceMode_t
     * @brief Represents the device mode.
     */
    typedef enum
    {
        NONE = 0, ///< No mode.
        TX = 1,   ///< Transmit mode.
        RX = 2    ///< Receive mode.
    } DeviceMode_t;

    /**
     * @struct E22Status_t
     * @brief Represents the status of the E22 device.
     */
    typedef struct
    {
        E22::tcxoVoltage_t tcxoVoltage;                   ///< TCXO voltage.
        uint32_t TXCOdio3Delay;                           ///< TXCO dio3 delay.
        E22::LoraPacketParams_t packetParams;             ///< Lora packet parameters.
        E22::ModulationParameters_t modulationParams;     ///< Modulation parameters.
        E22::RxGain_t rxGain;                             ///< RX gain.
        E22::ImageCalibrationFreq_t imageCalibrationFreq; ///< Image calibration frequency.
        E22::PacketType_t packetType;                     ///< Packet type.
        E22::Calibrate_t calibrations;                    ///< Calibrations.
        uint32_t frequency;                               ///< Frequency.
        XTalConfig_t xtalConfig;                          ///< Crystal oscillator configuration.
        E22::PaConfig_t paConfig;                         ///< PA configuration.
        E22::RampTime_t rampTime;                         ///< Ramp time.
        E22::SyncWordType_t syncWord;                     ///< Sync word type.
        uint32_t transmitTimeout;                         ///< Transmit timeout.
        uint32_t recieveTimeout;                          ///< Receive timeout.
    } E22Status_t;

    /**
     * @struct DeviceStatus_t
     * @brief Represents the status of the device.
     */
    typedef struct
    {
        E22Status_t E22Status; ///< Status of the E22 device.
        bool automatic;        ///< Indicates whether the status is automatic.
        DeviceMode_t mode;     ///< Device mode.
    } DeviceStatus_t;

    /**
     * @brief Initializes the device status.
     *
     * @return true If the device status was initialized correctly.
     * @return false If there was an error initializing the device status.
     */
    bool Begin(void);

    /**
     * @brief Prints the device status.
     *
     * @return true If the device status was printed correctly.
     * @return false If there was an error printing the device status.
     */
    bool printStatus(void);

    /**
     * @brief Loads the status of the device from the file system.
     *
     * @return true If the status was loaded correctly.
     * @return false If there was an error loading the status.
     */
    bool loadStatusFromFs(void);

    /**
     * @brief Represents the device status.
     */
    static DeviceStatus_t deviceStatus;

private:
    /**
     * @brief Private constructor to prevent instantiation.
     */
    DEVICESTATUS() {}

    /**
     * @brief Loads the device status from the file system.
     *
     * @return true If the device status was loaded correctly.
     * @return false If there was an error loading the device status.
     */
    static bool loadDeviceFromFs(void);

    /**
     * @brief Loads the packet type from the file system.
     *
     * @return true If the packet type was loaded correctly.
     * @return false If there was an error loading the packet type.
     */
    static bool loadPacketTypeFromFs(void);

    /**
     * @brief Loads the TCXO voltage from the file system.
     *
     * @return true If the TCXO voltage was loaded correctly.
     * @return false If there was an error loading the TCXO voltage.
     */
    static bool loadTCXOFromFs(void);

    /**
     * @brief Loads the calibrations from the file system.
     *
     * @return true If the calibrations were loaded correctly.
     * @return false If there was an error loading the calibrations.
     */
    static bool loadCalibrationsFromFs(void);

    /**
     * @brief Loads the crystal oscillator configuration from the file system.
     *
     * @return true If the crystal oscillator configuration was loaded correctly.
     * @return false If there was an error loading the crystal oscillator configuration.
     */
    static bool loadXTALConfigFromFs(void);

    /**
     * @brief Loads the frequency from the file system.
     *
     * @return true If the frequency was loaded correctly.
     * @return false If there was an error loading the frequency.
     */
    static bool loadFrequencyFromFs(void);

    /**
     * @brief Calculates the image calibration frequency based on the given frequency.
     *
     * @param frequency The frequency to calculate the image calibration frequency for.
     */
    static void calculateImageCalibrationFrequency(uint32_t frequency);

    /**
     * @brief Loads the PA configuration from the file system.
     *
     * @return true If the PA configuration was loaded correctly.
     * @return false If there was an error loading the PA configuration.
     */
    static bool loadPAConfigFromFs(void);

    /**
     * @brief Loads the transmit parameters from the file system.
     *
     * @return true If the transmit parameters were loaded correctly.
     * @return false If there was an error loading the transmit parameters.
     */
    static bool loadTxParamsFromFs(void);

    /**
     * @brief Loads the modulation parameters from the file system.
     *
     * @return true If the modulation parameters were loaded correctly.
     * @return false If there was an error loading the modulation parameters.
     */
    static bool loadModulationParamsFromFs(void);

    /**
     * @brief Loads the packet parameters from the file system.
     *
     * @return true If the packet parameters were loaded correctly.
     * @return false If there was an error loading the packet parameters.
     */
    static bool loadPacketParamsFromFs(void);

    /**
     * @brief Loads the sync word from the file system.
     *
     * @return true If the sync word was loaded correctly.
     * @return false If there was an error loading the sync word.
     */
    static bool loadSyncWordFromFs(void);

    /**
     * @brief Loads the RX gain from the file system.
     *
     * @return true If the RX gain was loaded correctly.
     * @return false If there was an error loading the RX gain.
     */
    static bool loadRxGainFromFs(void);

    /**
     * @brief Loads the timeout from the file system.
     *
     * @return true If the timeout was loaded correctly.
     * @return false If there was an error loading the timeout.
     */
    static bool loadTimeoutFromFs(void);
};

#endif // DEVICESTATUS_H