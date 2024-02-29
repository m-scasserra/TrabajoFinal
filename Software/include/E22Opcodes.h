#ifndef E22OPCODES_H
#define E22OPCODES_H

#include "includes.h"

typedef enum {
    E22_OpCode_SetSleep                = 0x84,
    E22_OpCode_SetStandby              = 0x80,
    E22_OpCode_SetFS                   = 0xC1,
    E22_OpCode_SetTX                   = 0x83,
    E22_OpCode_SetRX                   = 0x82,
    E22_OpCode_StopTimerOnPreamble     = 0x9F,
    E22_OpCode_SetRxDutyCycle          = 0x94,
    E22_OpCode_SetCad                  = 0xC5,
    E22_OpCode_SetTxContinuousWave     = 0xD1,
    E22_OpCode_SetInfinitePreamble     = 0xD2,
    E22_OpCode_SetRegulatorMode        = 0x96,
    E22_OpCode_Calibrate               = 0x89,
    E22_OpCode_CalibrateImage          = 0x98,
    E22_OpCode_SetPaConfig             = 0x95,
    E22_OpCode_SetRxTxFallbackMode     = 0x93
} E22_Operating_Modes_OpCode;

typedef enum {
    E22_OpCode_WriteRegister           = 0x0D,
    E22_OpCode_ReadRegister            = 0x1D,
    E22_OpCode_WriteBuffer             = 0x0E,
    E22_OpCode_ReadBuffer              = 0x1E
} E22_Register_FIFO_OpCode;

typedef enum {
    E22_OpCode_SetDioIrqParams         = 0x08,
    E22_OpCode_GetIrqStatus            = 0x12,
    E22_OpCode_ClearIrqStatus          = 0x02,
    E22_OpCode_SetDIO2AsRfSwitchCtrl   = 0x9D,
    E22_OpCode_SetDIO3AsTcxoCtrl       = 0x97
} E22_IRQ_DIO_OpCode;

typedef enum {
    E22_OpCode_SetRfFrequency          = 0x86,
    E22_OpCode_SetPacketType           = 0x8A,
    E22_OpCode_GetPacketType           = 0x11,
    E22_OpCode_SetTxParams             = 0x8E,
    E22_OpCode_SetModulationParams     = 0x8B,
    E22_OpCode_SetPacketParams         = 0x8C,
    E22_OpCode_SetCadParams            = 0x88,
    E22_OpCode_SetBufferBaseAddress    = 0x8F,
    E22_OpCode_SetLoRaSymbNumTimeout   = 0xA0
} E22_RF_Packets_OpCode;

typedef enum {
    E22_OpCode_GetStatus               = 0xC0,
    E22_OpCode_GetRssiInst             = 0x15,
    E22_OpCode_GetRxBufferStatus       = 0x13,
    E22_OpCode_GetPacketStatus         = 0x14,
    E22_OpCode_GetDeviceErrors         = 0x17,
    E22_OpCode_ClearDeviceErrors       = 0x07,
    E22_OpCode_GetStats                = 0x10,
    E22_OpCode_ResetStats              = 0x00
} E22_Status_Cmd;

#endif // E22OPCODES_H