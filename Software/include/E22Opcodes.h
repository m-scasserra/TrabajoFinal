#ifndef E22OPCODES_H
#define E22OPCODES_H

typedef enum {
    E22_OpCode_SetSleep                 = 0x84,
    E22_OpCode_SetStandby               = 0x80,
    E22_OpCode_SetFS                    = 0xC1,
    E22_OpCode_SetTX                    = 0x83,
    E22_OpCode_SetRX                    = 0x82,
    E22_OpCode_StopTimerOnPreamble      = 0x9F,
    E22_OpCode_SetRxDutyCycle           = 0x94,
    E22_OpCode_SetCad                   = 0xC5,
    E22_OpCode_SetTxContinuousWave      = 0xD1,
    E22_OpCode_SetInfinitePreamble      = 0xD2,
    E22_OpCode_SetRegulatorMode         = 0x96,
    E22_OpCode_Calibrate                = 0x89,
    E22_OpCode_CalibrateImage           = 0x98,
    E22_OpCode_SetPaConfig              = 0x95,
    E22_OpCode_SetRxTxFallbackMode      = 0x93,
    E22_OpCode_WriteRegister            = 0x0D,
    E22_OpCode_ReadRegister             = 0x1D,
    E22_OpCode_WriteBuffer              = 0x0E,
    E22_OpCode_ReadBuffer               = 0x1E,
    E22_OpCode_SetDioIrqParams          = 0x08,
    E22_OpCode_GetIrqStatus             = 0x12,
    E22_OpCode_ClearIrqStatus           = 0x02,
    E22_OpCode_SetDIO2AsRfSwitchCtrl    = 0x9D,
    E22_OpCode_SetDIO3AsTcxoCtrl        = 0x97,
    E22_OpCode_SetRfFrequency           = 0x86,
    E22_OpCode_SetPacketType            = 0x8A,
    E22_OpCode_GetPacketType            = 0x11,
    E22_OpCode_SetTxParams              = 0x8E,
    E22_OpCode_SetModulationParams      = 0x8B,
    E22_OpCode_SetPacketParams          = 0x8C,
    E22_OpCode_SetCadParams             = 0x88,
    E22_OpCode_SetBufferBaseAddress     = 0x8F,
    E22_OpCode_SetLoRaSymbNumTimeout    = 0xA0,
    E22_OpCode_GetStatus                = 0xC0,
    E22_OpCode_GetRssiInst              = 0x15,
    E22_OpCode_GetRxBufferStatus        = 0x13,
    E22_OpCode_GetPacketStatus          = 0x14,
    E22_OpCode_GetDeviceErrors          = 0x17,
    E22_OpCode_ClearDeviceErrors        = 0x07,
    E22_OpCode_GetStats                 = 0x10,
    E22_OpCode_ResetStats               = 0x00
} E22_OpCode_Cmd_t;

typedef enum {
    E22_Reg_HoppingEnable               = 0x0385,
    E22_Reg_PacketLenght                = 0x0386,
    E22_Reg_NbHoppingBlocks             = 0x0387,
    E22_Reg_NbSymbols0_0                = 0x0388,
    E22_Reg_NbSymbols0_1                = 0x0389,
    E22_Reg_Freq0_0                     = 0x038A,
    E22_Reg_Freq0_1                     = 0x038B,
    E22_Reg_Freq0_2                     = 0x038C,
    E22_Reg_Freq0_3                     = 0x038D,
    E22_Reg_NbSymbols1_0                = 0x038E,
    E22_Reg_NbSymbols1_1                = 0x038F,
    E22_Reg_Freq1_0                     = 0x0390,
    E22_Reg_Freq1_1                     = 0x0391,
    E22_Reg_Freq1_2                     = 0x0392,
    E22_Reg_Freq1_3                     = 0x0393,
    E22_Reg_NbSymbols2_0                = 0x0394,
    E22_Reg_NbSymbols2_1                = 0x0395,
    E22_Reg_Freq2_0                     = 0x0396,
    E22_Reg_Freq2_1                     = 0x0397,
    E22_Reg_Freq2_2                     = 0x0398,
    E22_Reg_Freq2_3                     = 0x0399,
    E22_Reg_NbSymbols3_0                = 0x039A,
    E22_Reg_NbSymbols3_1                = 0x039B,
    E22_Reg_Freq3_0                     = 0x039C,
    E22_Reg_Freq3_1                     = 0x039D,
    E22_Reg_Freq3_2                     = 0x039E,
    E22_Reg_Freq3_3                     = 0x039F,
    E22_Reg_NbSymbols4_0                = 0x03A0,
    E22_Reg_NbSymbols4_1                = 0x03A1,
    E22_Reg_Freq4_0                     = 0x03A2,
    E22_Reg_Freq4_1                     = 0x03A3,
    E22_Reg_Freq4_2                     = 0x03A4,
    E22_Reg_Freq4_3                     = 0x03A5,
    E22_Reg_NbSymbols5_0                = 0x03A6,
    E22_Reg_NbSymbols5_1                = 0x03A7,
    E22_Reg_Freq5_0                     = 0x03A8,
    E22_Reg_Freq5_1                     = 0x03A9,
    E22_Reg_Freq5_2                     = 0x03AA,
    E22_Reg_Freq5_3                     = 0x03AB,
    E22_Reg_NbSymbols6_0                = 0x03AC,
    E22_Reg_NbSymbols6_1                = 0x03AD,
    E22_Reg_Freq6_0                     = 0x03AE,
    E22_Reg_Freq6_1                     = 0x03AF,
    E22_Reg_Freq6_2                     = 0x03B0,
    E22_Reg_Freq6_3                     = 0x03B1,
    E22_Reg_NbSymbols7_0                = 0x03B2,
    E22_Reg_NbSymbols7_1                = 0x03B3,
    E22_Reg_Freq7_0                     = 0x03B4,
    E22_Reg_Freq7_1                     = 0x03B5,
    E22_Reg_Freq7_2                     = 0x03B6,
    E22_Reg_Freq7_3                     = 0x03B7,
    E22_Reg_NbSymbols8_0                = 0x03B8,
    E22_Reg_NbSymbols8_1                = 0x03B9,
    E22_Reg_Freq8_0                     = 0x03BA,
    E22_Reg_Freq8_1                     = 0x03BB,
    E22_Reg_Freq8_2                     = 0x03BC,
    E22_Reg_Freq8_3                     = 0x03BD,
    E22_Reg_NbSymbols9_0                = 0x03BE,
    E22_Reg_NbSymbols9_1                = 0x03BF,
    E22_Reg_Freq9_0                     = 0x03C0,
    E22_Reg_Freq9_1                     = 0x03C1,
    E22_Reg_Freq9_2                     = 0x03C2,
    E22_Reg_Freq9_3                     = 0x03C3,
    E22_Reg_NbSymbols10_0               = 0x03C4,
    E22_Reg_NbSymbols10_1               = 0x03C5,
    E22_Reg_Freq10_0                    = 0x03C6,
    E22_Reg_Freq10_1                    = 0x03C7,
    E22_Reg_Freq10_2                    = 0x03C8,
    E22_Reg_Freq10_3                    = 0x03C9,
    E22_Reg_NbSymbols11_0               = 0x03CA,
    E22_Reg_NbSymbols11_1               = 0x03CB,
    E22_Reg_Freq11_0                    = 0x03CC,
    E22_Reg_Freq11_1                    = 0x03CD,
    E22_Reg_Freq11_2                    = 0x03CE,
    E22_Reg_Freq11_3                    = 0x03CF,
    E22_Reg_NbSymbols12_0               = 0x03D0,
    E22_Reg_NbSymbols12_1               = 0x03D1,
    E22_Reg_Freq12_0                    = 0x03D2,
    E22_Reg_Freq12_1                    = 0x03D3,
    E22_Reg_Freq12_2                    = 0x03D4,
    E22_Reg_Freq12_3                    = 0x03D5,
    E22_Reg_NbSymbols13_0               = 0x03D6,
    E22_Reg_NbSymbols13_1               = 0x03D7,
    E22_Reg_Freq13_0                    = 0x03D8,
    E22_Reg_Freq13_1                    = 0x03D9,
    E22_Reg_Freq13_2                    = 0x03DA,
    E22_Reg_Freq13_3                    = 0x03DB,
    E22_Reg_NbSymbols14_0               = 0x03DC,
    E22_Reg_NbSymbols14_1               = 0x03DD,
    E22_Reg_Freq14_0                    = 0x03DE,
    E22_Reg_Freq14_1                    = 0x03DF,
    E22_Reg_Freq14_2                    = 0x03E0,
    E22_Reg_Freq14_3                    = 0x03E1,
    E22_Reg_NbSymbols15_0               = 0x03E2,
    E22_Reg_NbSymbols15_1               = 0x03E3,
    E22_Reg_Freq15_0                    = 0x03E4,
    E22_Reg_Freq15_1                    = 0x03E5,
    E22_Reg_Freq15_2                    = 0x03E6,
    E22_Reg_Freq15_3                    = 0x03E7,
    E22_Reg_DIOxOutputEnable            = 0x0580,
    E22_Reg_DIOxInputEnable             = 0x0583,
    E22_Reg_DIOxPullUpControl           = 0x0584,
    E22_Reg_DIOxPullDownControl         = 0x0585,
    E22_Reg_WhiteningInitialValueMSB    = 0x06B8,
    E22_Reg_WhiteningInitialValueLSB    = 0x06B9,
    E22_Reg_CRCMSBInitialValue0         = 0x06BC,
    E22_Reg_CRCLSBInitialValue0         = 0x09BD,
    E22_Reg_CRCMSBPolynomialValue0      = 0x06BE,
    E22_Reg_CRCLSBPolynomialValue0      = 0x06BF,
    E22_Reg_SyncWord0                   = 0x06C0,
    E22_Reg_SyncWord1                   = 0x06C1,
    E22_Reg_SyncWord2                   = 0x06C2,
    E22_Reg_SyncWord3                   = 0x06C3,
    E22_Reg_SyncWord4                   = 0x06C4,
    E22_Reg_SyncWord5                   = 0x06C5,
    E22_Reg_SyncWord6                   = 0x06C6,
    E22_Reg_SyncWord7                   = 0x06C7,
    E22_Reg_NodeAddress                 = 0x06CD,
    E22_Reg_BroadcastAddress            = 0x06CE,
    E22_Reg_IQPolaritySetup             = 0x0736,
    E22_Reg_LoRaSyncWordMSB             = 0x0740,
    E22_Reg_LoRaSyncWordLSB             = 0x0741,
    E22_Reg_RandomNumberGen0            = 0x0819,
    E22_Reg_RandomNumberGen1            = 0x081A,
    E22_Reg_RandomNumberGen2            = 0x081B,
    E22_Reg_RandomNumberGen3            = 0x081C,
    E22_Reg_TxModulation                = 0x0889,
    E22_Reg_RxGain                      = 0x08AC,
    E22_Reg_TxClampConfig               = 0x08D8,
    E22_Reg_OCPConfiguration            = 0x08E7,
    E22_Reg_RTCControl                  = 0x0902,
    E22_Reg_XTATrim                     = 0x0911,
    E22_Reg_XTBTrim                     = 0x0912,
    E22_Reg_DIO3OutputVoltageControl    = 0x0920,
    E22_Reg_EventMask                   = 0x0944
} E22_Reg_Addr;

#endif // E22OPCODES_H