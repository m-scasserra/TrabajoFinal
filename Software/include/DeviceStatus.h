#ifndef DEVICESTATUS_H
#define DEVICESTATUS_H

#include "Includes.h"

class DEVICESTATUS
{

public:
    // Eliminar las funciones de copia y asignación
    DEVICESTATUS(const DEVICESTATUS &) = delete;
    DEVICESTATUS &operator=(const DEVICESTATUS &) = delete;


    static DEVICESTATUS &getInstance()
    {
        static DEVICESTATUS instance; // Única instancia
        return instance;
    }

    typedef struct
    {
        bool use;
        uint8_t XTALA;
        uint8_t XTALB;
    } XTalConfig_t;


    typedef struct
    {
        E22::tcxoVoltage_t tcxoVoltage;
        uint32_t TXCOdio3Delay;
        E22::LoraPacketParams_t packetParams;
        E22::ModulationParameters_t modulationParams;
        E22::RxGain_t rxGain;
        E22::ImageCalibrationFreq_t imageCalibrationFreq;
        E22::PacketType_t packetType;
        E22::Calibrate_t calibrations;
        uint32_t frequency;
        XTalConfig_t xtalConfig;
        E22::PaConfig_t paConfig;
        E22::RampTime_t rampTime;
        E22::SyncWordType_t syncWord;
        uint32_t transmitTimeout;
        uint32_t recieveTimeout;
    } E22Status_t;

    typedef struct
    {
        E22Status_t E22Status;
        bool automatic;
    } DeviceStatus_t;
    

    bool Begin(void);
    bool printStatus(void);
    bool loadStatusFromFs(void);

    static DeviceStatus_t deviceStatus;

private:
    // Constructor privado
    DEVICESTATUS() {}

    static bool loadDeviceFromFs(void);
    static bool loadPacketTypeFromFs(void);
    static bool loadTCXOFromFs(void);
    static bool loadCalibrationsFromFs(void);
    static bool loadXTALConfigFromFs(void);
    static bool loadFrequencyFromFs(void);
    static void calculateImageCalibrationFrequency(uint32_t frequency);
    static bool loadPAConfigFromFs(void);
    static bool loadTxParamsFromFs(void);
    static bool loadModulationParamsFromFs(void);
    static bool loadPacketParamsFromFs(void);
    static bool loadSyncWordFromFs(void);
    static bool loadRxGainFromFs(void);
    static bool loadTimeoutFromFs(void);

};

#endif // DEVICESTATUS_H