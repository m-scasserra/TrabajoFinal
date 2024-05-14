#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h>

#pragma pack(push, 1)

typedef struct
{
    time_t timeAction;
    bool direction; // 0 is sent, 1 is recieved
    uint8_t RssiPkt;
    uint8_t SnrPkt;
    uint8_t SignalRssiPkt;
    // This is the message paylod
    int32_t ADCValue;
    uint8_t flags;
    uint32_t messageId;

} Message_t;

#pragma pack(pop)

class MESSAGE
{
public:
    // Eliminar las funciones de copia y asignación
    MESSAGE(const MESSAGE &) = delete;
    MESSAGE &operator=(const MESSAGE &) = delete;

    // Función para acceder a la instancia única del MESSAGE
    static MESSAGE &getInstance()
    {
        static MESSAGE instance; // Única instancia
        return instance;
    }

    Message_t processMessageSent(uint8_t arr[]);
    Message_t processMessageRecieved(uint8_t arr[]);
    bool saveMessage(Message_t message);
    bool readAllPackets(void);

private:
    // Constructor privado
    MESSAGE() {}
};
