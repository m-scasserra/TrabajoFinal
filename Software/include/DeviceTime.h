#ifndef DEVICETIME_H
#define DEVICETIME_H

#include "Includes.h"

#define TIMETAG "TIME"
#define DEFAULT_TIMEZONE "GMT+3"
#define DEFAULT_NTP_SERVER "ar.pool.ntp.org"
#define WAIT_TIME_FOR_NTP_MS 10000
#define TIME_BIN_PATH "/storage/AutomaticJobs/time.bin"

class DEVICETIME
{

public:
    // Eliminar las funciones de copia y asignación
    DEVICETIME(const DEVICETIME &) = delete;
    DEVICETIME &operator=(const DEVICETIME &) = delete;


    static DEVICETIME &getInstance()
    {
        static DEVICETIME instance; // Única instancia
        return instance;
    }

    bool Begin(void);
    bool updateTimeFromNet(const char *server, uint8_t serverLen);
    bool printTime(void);
    bool saveTimeToFs(void);
    bool loadTimeFromFs(void);

private:
    // Constructor privado
    DEVICETIME() {}

};

#endif // DEVICE_H