#ifndef E22DRIVER_H
#define E22DRIVER_H

#include "includes.h"

#define E22TAG "E22"

// Defines generales

#define MAX_CMD_PARAMS 10

// Defines para la tarea de FRTOS del E22

#define MAX_E22_CMD_QUEUE 10

// Configuracion del SPI Device E22
#define SPI_COMMAND_LEN 8
#define SPI_ADDR_LEN 16
#define SPI_DUMMY_BITS 0
#define SPI_MODE 0
#define SPI_DUTY_CYCLE 0
#define CYCLES_CS_BEFORE_TRANS 1
#define CYCLES_CS_AFTER_TRANS 1
#define SPI_CLOCK 5000000 // 10MHz SPI Clock
#define SPI_QUEUE_SIZE 10
#define SPI_INPUT_DELAY 0
#define SPI_DEVICE_CONFIG_FLAGS SPI_DEVICE_NO_DUMMY

class E22
{
public:
    // Eliminar las funciones de copia y asignación
    E22(const E22 &) = delete;
    E22 &operator=(const E22 &) = delete;

    // Función para acceder a la instancia única del E22
    static E22 &getInstance()
    {
        static E22 instance; // Única instancia
        return instance;
    }

    enum StdByMode_t
    {
        STDBY_RC,
        STDBY_XOSC
    };

    enum E22Cmd_t
    {
        E22_CMD_SetSleep,
        E22_CMD_SetStandBy,
        E22_CMD_SetFs,
        E22_CMD_SetTx,
        E22_CMD_SetRx,
        E22_CMD_StopTimerOnPreamble,
        E22_CMD_SetRxDutyCycle,
        E22_CMD_SetCad,
        E22_CMD_SetTxContinousWave,
        E22_CMD_SetTxInfinitePreamble,
        E22_CMD_SetRegulatorMode,
        E22_CMD_Calibrate,
        E22_CMD_CalibrateImage,
        E22_CMD_SetPaConfig,
        E22_CMD_SetRxTxFallbackMode,
        E22_CMD_WriteRegister,
        E22_CMD_ReadRegister,
        E22_CMD_WriteBuffer,
        E22_CMD_ReadBuffe,
        E22_CMD_SetDioIrqParams,
        E22_CMD_GetIrqStatus,
        E22_CMD_ClearIrqStatus,
        E22_CMD_SetDIO2AsRfSwitchCtrl,
        E22_CMD_SetDIO3asTcxoCtrl,
        E22_CMD_SetRfFrequency,
        E22_CMD_SetPacketType,
        E22_CMD_GetPacketType,
        E22_CMD_SetTxParams,
        E22_CMD_SetModulationParams,
        E22_CMD_SetPacketParams,
        E22_CMD_SetCadParams,
        E22_CMD_SetBufferBaseAddress,
        E22_CMD_SetLoRaSymbNumTimeout,
        E22_CMD_GetStatus,
        E22_CMD_GetRssiInst,
        E22_CMD_GetRxBufferStatus,
        E22_CMD_GetPacketStatus,
        E22_CMD_GetDeviceErrors,
        E22_CMD_ClearDeviceErrors,
        E22_CMD_GetStats,
        E22_CMD_ResetStats,
    };

    typedef struct
    {
        E22Cmd_t command_code;
        uint8_t param_count;            // Number of parameters present
        uint8_t params[MAX_CMD_PARAMS]; // Array to hold various parameter types
    } E22Command_t;

    bool Begin(void);

private:
    // Constructor privado
    E22() {}

    static void E22Task(void *pvParameters);
    bool processStatus(uint8_t msg);
    bool processCmd(void);
    bool getStatus(void);
    bool getRssiInst(void);
    bool setBufferBaseAddress(uint8_t TxBaseAddr, uint8_t RxBaseAddr);
    bool setRx(uint32_t Timeout);
    bool setStandBy(StdByMode_t mode);
    bool isBusy(void);
    bool resetOn(void);
    bool resetOff(void);

    spi_device_handle_t SPIHandle;

    static TaskHandle_t E22TaskHandle;
    QueueHandle_t xE22CmdQueue;

    uint8_t rssiInst;
};

#endif // E22DRIVER_H