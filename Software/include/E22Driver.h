#ifndef E22DRIVER_H
#define E22DRIVER_H

#include "Includes.h"

#define E22TAG "E22"

// Defines generales

#define MAX_CMD_PARAMS 10
#define MAX_RESPONSES 10

#define DEFAULT_MSG_TIMEOUT_MS 1000

// Defines para la tarea de FRTOS del E22

#define MAX_E22_CMD_QUEUE 200

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

// Configuraciones del E22
#define SX126X_RF_FREQUENCY_XTAL 32000000 // XTAL frequency used for RF frequency calculation
#define SX126X_RF_FREQUENCY_SHIFT 25      // RF frequency = RF Freq * F XTAL / 2^25
#define SX126X_RX_BASE_BUFFER_ADDR 0x00   // Buffer Address where the received packet will be stored
#define SX126X_TX_BASE_BUFFER_ADDR 0x80   // Buffer Address where the transmitted packet will be stored

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

    enum PacketType_t
    {
        PACKET_TYPE_GFSK = 0x0,
        PACKET_TYPE_LORA = 0x1,
        PACKET_TYPE_LR_FHSS = 0x3
    };

    enum tcxoVoltage_t
    {
        TCXOVOLTAGE_1_6 = 0x0,
        TCXOVOLTAGE_1_7 = 0x1,
        TCXOVOLTAGE_1_8 = 0x2,
        TCXOVOLTAGE_2_2 = 0x3,
        TCXOVOLTAGE_2_4 = 0x4,
        TCXOVOLTAGE_2_7 = 0x5,
        TCXOVOLTAGE_3_0 = 0x6,
        TCXOVOLTAGE_3_3 = 0x7
    };

    typedef struct
    {
        bool RC64kCalibration;
        bool RC13MCalibration;
        bool PLLCalibration;
        bool ADCPulseCalibration;
        bool ADCBulkNCalibration;
        bool ADCBulkPCalibration;
        bool ImageCalibration;
    } Calibrate_t;

    enum ImageCalibrationFreq_t
    {
        FREQ_430_440,
        FREQ_470_510,
        FREQ_779_787,
        FREQ_863_870,
        FREQ_902_928
    };

    enum RxGain_t
    {
        RX_POWER_SAVE = 0x94,
        RX_BOOST = 0x96
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
        E22_CMD_ReadBuffer,
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

    enum SpredingFactor_t
    {
        SF_5 = 0x5,
        SF_6 = 0x6,
        SF_7 = 0x7,
        SF_8 = 0x8,
        SF_9 = 0x9,
        SF_10 = 0xA,
        SF_11 = 0xB,
        SF_12 = 0xC
    };

    enum BandWidth_t
    {
        LORA_BW_7 = 0x00,
        LORA_BW_10 = 0x08,
        LORA_BW_15 = 0x01,
        LORA_BW_20 = 0x09,
        LORA_BW_31 = 0x02,
        LORA_BW_41 = 0x0A,
        LORA_BW_62 = 0x03,
        LORA_BW_125 = 0x04,
        LORA_BW_250 = 0x05,
        LORA_BW_500 = 0x06
    };

    enum CodingRate_t
    {
        LORA_CR_4_5 = 0x1,
        LORA_CR_4_6 = 0x2,
        LORA_CR_4_7 = 0x3,
        LORA_CR_4_8 = 0x4
    };

    typedef struct
    {
        SpredingFactor_t spreadingFactor;
        BandWidth_t bandwidth;
        CodingRate_t codingRate;
    } ModulationParameters_t;

    enum PacketHeaderType_t
    {
        EXPLICIT_HEADER = 0x0,
        IMPLICIT_HEADER = 0x1
    };

    enum PacketIQType_t
    {
        STANDARD_IQ = 0x0,
        INVERTED_IQ = 0x1
    };

    typedef struct
    {
        uint16_t preambleLength;
        PacketHeaderType_t headerType;
        uint8_t payloadLength;
        bool crcType;
        PacketIQType_t iqType;
    } LoraPacketParams_t;

    enum SyncWordType_t
    {
        PUBLIC_SYNCWORD,
        PRIVATE_SYNCWORD
    };

    enum IRQRegBits_t
    {
        TX_DONE = 0x1,
        RX_DONE = 0x2,
        PREAMBLE_DETECTED = 0x4,
        SYNCWORD_VALID = 0x8,
        HEADER_VALID = 0x10,
        HEADER_ERR = 0x20,
        CRC_ERR = 0x40,
        CAD_DONE = 0x80,
        CAD_DETECTED = 0x100,
        TIMEOUT = 0x200,
        LRFGSS_HOP = 0x4000
    };

    typedef struct
    {
        bool txDone;           // TX_DONE = 0x1,
        bool rxDone;           // RX_DONE = 0x2,
        bool preambleDetected; // PREAMBLE_DETECTED = 0x4,
        bool syncWordValid;    // SYNCWORD_VALID = 0x8,
        bool headerValid;      // HEADER_VALID = 0x10,
        bool headerErr;        // HEADER_ERR = 0x20,
        bool crcErr;           // CRC_ERR = 0x40,
        bool cadDone;          // CAD_DONE = 0x80,
        bool cadDetected;      // CAD_DETECTED = 0x100,
        bool timeout;          // TIMEOUT = 0x200,
        bool lrFhssHop;        // LRFGSS_HOP = 0x4000
    } IRQReg_t;

    enum PaConfig_t
    {
        PA_22_DBM,
        PA_20_DBM,
        PA_17_DBM,
        PA_14_DBM
    };

    enum RampTime_t
    {
        SET_RAMP_10U = 0x00,   // 10 us
        SET_RAMP_20U = 0x01,   // 20 us
        SET_RAMP_40U = 0x02,   // 40 us
        SET_RAMP_80U = 0x03,   // 80 us
        SET_RAMP_200U = 0x04,  // 200 us
        SET_RAMP_800U = 0x05,  // 800 us
        SET_RAMP_1700U = 0x06, // 1700 us
        SET_RAMP_3400U = 0x07, // 3400 us
    };

    typedef enum{
        NONE = 0,
        TX = 1,
        RX = 2
    } E22SetUpState_t;

    typedef struct
    {
        E22Cmd_t commandCode;
        uint8_t paramCount; // Number of parameters present
        union params_t
        {
            uint8_t paramsArray[MAX_CMD_PARAMS]; // Array to hold various input parameter
            uint8_t *paramsPtr[MAX_CMD_PARAMS];  // Pointer to hold various input parameter
        } params;

        bool hasResponse;
        uint8_t responsesCount; // Number of responses of the command
    } E22Command_t;

    typedef struct
    {
        E22Cmd_t commandCode;
        uint8_t responsesCount; // Number of responses of the command
        union responses_t
        {
            uint8_t responsesArray[MAX_RESPONSES]; // Array to hold various responses
            uint8_t *responsesPtr8[MAX_RESPONSES]; // Pointer to hold various response of 8 bits
        } responses;
    } E22Response_t;

    bool Begin(void);

    bool setPacketType(PacketType_t _packetType);
    bool getPacketType(PacketType_t *packetType);
    bool writeRegister(E22_Reg_Addr addr, uint8_t dataIn);
    bool readRegister(E22_Reg_Addr addr, uint8_t *dataOut);
    bool writeBuffer(uint8_t offset, uint8_t dataIn);
    bool readBuffer(uint8_t offset, uint8_t *dataOut);
    bool getStatus(void);
    bool getRssiInst(void);
    bool setBufferBaseAddress(uint8_t TxBaseAddr, uint8_t RxBaseAddr);
    bool setRx(uint32_t Timeout);
    bool setStandBy(StdByMode_t mode);
    bool setDIO3asTCXOCtrl(tcxoVoltage_t voltage, uint32_t delay);
    bool setXtalCap(uint8_t XTA, uint8_t XTB);
    bool calibrate(Calibrate_t calibrationsToDo);
    uint8_t calibrationsToMask(Calibrate_t calibrations);
    static Calibrate_t calibrationsFromMask(uint8_t calibrationsMask);
    bool calibrateImage(ImageCalibrationFreq_t frequency);
    bool setFrequency(uint32_t frequency);
    bool setFrequencyAndCalibrate(uint32_t frequency);
    bool setRxGain(RxGain_t gain);
    bool setModulationParams(ModulationParameters_t modulation);
    bool setPacketParams(LoraPacketParams_t packetParams);
    bool setSyncWord(SyncWordType_t syncWord);
    bool messageIsAvailable(void);
    uint8_t getMessageLenght(void);
    bool getMessageRxByte(uint8_t *dataOut);
    bool getMessageRxLenght(uint8_t *dataOut, uint8_t length);
    bool beginTxPacket(void);
    bool writeMessageTxByte(uint8_t data);
    bool writeMessageTxLength(uint8_t *data, uint8_t length);
    bool changePacketPreambleLenght(uint16_t preambleLenght);
    bool changePacketHeaderType(PacketHeaderType_t headerType);
    bool changePacketPayloadLength(uint8_t payloadLength);
    bool changePacketCrcType(bool crcType);
    bool changePacketIQType(PacketIQType_t iqType);
    bool setTx(uint32_t Timeout);
    bool transmitPacket(uint32_t Timeout);
    bool receivePacket(uint32_t Timeout);
    void setMsgTimeoutms(uint32_t newMsgTimeoutms);
    bool setPaConfig(PaConfig_t PaConfig);
    bool setTxParams(RampTime_t RampTime);
    bool fixInvertedIq(PacketIQType_t iqType);
    bool fixModulationQuality(void);

    bool setDioIrqParams(IRQReg_t IRQMask, IRQReg_t DIO1Mask, IRQReg_t DIO2Mask, IRQReg_t DIO3Mask);
    bool getPacketStatus(uint8_t *RssiPkt, uint8_t *SnrPkt, uint8_t *SignalRssiPkt);

    static bool setUpForRx(void);
    static bool setUpForTx(void);

    const IRQReg_t IRQREGFULL = {
        .txDone = true,
        .rxDone = true,
        .preambleDetected = true,
        .syncWordValid = true,
        .headerValid = true,
        .headerErr = true,
        .crcErr = true,
        .cadDone = true,
        .cadDetected = true,
        .timeout = true,
        .lrFhssHop = true};

    const IRQReg_t IRQREGEMPTY = {
        .txDone = false,
        .rxDone = false,
        .preambleDetected = false,
        .syncWordValid = false,
        .headerValid = false,
        .headerErr = false,
        .crcErr = false,
        .cadDone = false,
        .cadDetected = false,
        .timeout = false,
        .lrFhssHop = false};

private:
    // Constructor privado
    E22() {}

    static void E22Task(void *pvParameters);
    static void IRAM_ATTR E22ISRHandler(void);

    bool processCmd(void);
    bool processInterrupt(void);
    bool processResponse(void);

    bool E22IOInit(void);
    bool InterruptInit(void);

    void processStatus(uint8_t msg);
    void updateIRQStatusFromMask(uint16_t IRQRegValue);
    uint16_t processIRQMask(IRQReg_t IRQMask);

    bool getRxBufferStatus(uint8_t *bufferLenght, uint8_t *bufferStart);
    bool updateIrqStatus(void);
    bool clearIrqStatus(IRQReg_t IRQRegClear);

    bool isBusy(void);
    bool resetOn(void);
    bool resetOff(void);

    bool checkDeviceConnection(void);         // This function bypasses the FRTOS queue and directly executes the command
    bool antennaMismatchCorrection(void);     // This function bypasses the FRTOS queue and directly executes the command
    bool getIRQStatusForInterrupt(void);      // This function bypasses the FRTOS queue and directly executes the command
    bool getRxBufferStatusForInterrupt(void); // This function bypasses the FRTOS queue and directly executes the command

    static TaskHandle_t E22TaskHandle;
    static SemaphoreHandle_t xE22InterruptSempahore;
    static SemaphoreHandle_t xE22ResponseWaitSempahore;
    QueueHandle_t xE22CmdQueue;
    QueueHandle_t xE22ResponseQueue;

    static uint8_t s_rssiInst;
    static uint32_t s_msgTimeoutms;
    static LoraPacketParams_t s_packetParams;
    static ModulationParameters_t s_modulationParams;
    static PacketType_t s_packetType;

    static uint8_t s_PayloadLenghtRx;
    static uint8_t s_RxBufferAddr;
    static uint8_t s_TxBufferAddr;

    static bool PacketReceived;

    static E22SetUpState_t E22SetUpState;

    static IRQReg_t IRQReg;
    static bool processIRQ;
};

#endif // E22DRIVER_H