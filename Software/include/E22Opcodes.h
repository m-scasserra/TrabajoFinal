#ifndef E22OPCODES_H
#define E22OPCODES_H

#include "includes.h"

typedef enum {
    E22_CMD_SetSleep                = 0x84,
    E22_CMD_SetStandby              = 0x80,
    E22_CMD_SetFS                   = 0xC1,
    E22_CMD_SetTX                   = 0x83,
    E22_CMD_SetRX                   = 0x82,
    E22_CMD_StopTimerOnPreamble     = 0x9F,
    E22_CMD_SetRxDutyCycle          = 0x94,
    E22_CMD_SetCad                  = 0xC5,
    E22_CMD_SetTxContinuousWave     = 0xD1,
    E22_CMD_SetInfinitePreamble     = 0xD2,
    E22_CMD_SetRegulatorMode        = 0x96,
    E22_CMD_Calibrate               = 0x89,
    E22_CMD_CalibrateImage          = 0x98,
    E22_CMD_SetPaConfig             = 0x95,
    E22_CMD_SetRxTxFallbackMode     = 0x93
} E22_Operating_Modes_Cmd;

typedef enum {
    E22_CMD_WriteRegister           = 0x0D,
    E22_CMD_ReadRegister            = 0x1D,
    E22_CMD_WriteBuffer             = 0x0E,
    E22_CMD_ReadBuffer              = 0x1E
} E22_Register_FIFO_Cmd;

typedef enum {
    E22_CMD_SetDioIrqParams         = 0x08,
    E22_CMD_GetIrqStatus            = 0x12,
    E22_CMD_ClearIrqStatus          = 0x02,
    E22_CMD_SetDIO2AsRfSwitchCtrl   = 0x9D,
    E22_CMD_SetDIO3AsTcxoCtrl       = 0x97
} E22_IRQ_DIO_Cmd;

typedef enum {
    E22_CMD_SetRfFrequency          = 0x86,
    E22_CMD_SetPacketType           = 0x8A,
    E22_CMD_GetPacketType           = 0x11,
    E22_CMD_SetTxParams             = 0x8E,
    E22_CMD_SetModulationParams     = 0x8B,
    E22_CMD_SetPacketParams         = 0x8C,
    E22_CMD_SetCadParams            = 0x88,
    E22_CMD_SetBufferBaseAddress    = 0x8F,
    E22_CMD_SetLoRaSymbNumTimeout   = 0xA0
} E22_RF_Packets_Cmd;

typedef enum {
    E22_CMD_GetStatus               = 0xC0,
    E22_CMD_GetRssiInst             = 0x15,
    E22_CMD_GetRxBufferStatus       = 0x13,
    E22_CMD_GetPacketStatus         = 0x14,
    E22_CMD_GetDeviceErrors         = 0x17,
    E22_CMD_ClearDeviceErrors       = 0x07,
    E22_CMD_GetStats                = 0x10,
    E22_CMD_ResetStats              = 0x00
} E22_Status_Cmd;

#endif // E22OPCODES_H