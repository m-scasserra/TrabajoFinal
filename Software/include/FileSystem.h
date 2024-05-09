#ifndef FS_H
#define FS_H

#include "Includes.h"

#define FSTAG "FS"
#define STORAGE_PARTITION_NAME "storage"
#define INI_FILENAME "/config.ini"
#define INI_MAX_LEN 100
#define MAX_PATH_LEN 100

class FS{

public:
    // Eliminar las funciones de copia y asignación
    FS(const FS&) = delete;
    FS& operator=(const FS&) = delete;

    // Función para acceder a la instancia única del GPS
    static FS& getInstance() {
        static FS instance; // Única instancia
        return instance;
    }
    
    void Begin(void);
    // Solo hago un check por el html, el css, el js y el logopng
    bool checkPortalFiles(void);
    int getFileSize(const char *path, long *fileSize);
    int getFileBuffer(const char *path, void *buffer, size_t sizeBuffer, size_t sizeFile);
    bool checkIniFile(void);
    esp_err_t formatPartition(const char *PartitionName);
    bool Ini_gets(const char *Section, const char *Key, char *Result);
    bool Ini_puts(const char *Section, const char *Key, const char *Value);
    bool ls(const char *PartitionName);
    bool cat(const char *filePath);
    

private:
    // Constructor privado
    FS() {}

    bool createIni(void);
    bool populateIni();
    esp_err_t mountPartition(const char *PartitionName);
    static char IniPath[MAX_PATH_LEN];

};

#endif // FS_H