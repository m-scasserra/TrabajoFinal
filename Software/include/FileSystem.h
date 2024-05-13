#ifndef FS_H
#define FS_H

#include "Includes.h"
#include "dev/minIni.h"
#include "esp_littlefs.h"

#define FSTAG "FS"
#define STORAGE_PARTITION_NAME "storage"
#define AUTOMATIC_JOBS_FOLDER_PATH "/storage/AutomaticJobs"
#define PACKETS_RECEIVED_FOLDER_PATH "/storage/PacketsReceived"
#define MEASUREMENTS_FOLDER_PATH "/storage/Measurements"
#define CONFIG_FOLDER_PATH "/storage/Config"

#define DEVICE_CONFIG_FILE_PATH "/storage/Config/deviceConfig.ini"
#define TIME_BIN_PATH "/storage/AutomaticJobs/time.bin"

#define INI_MAX_LEN 100
#define PATH_MAX_LEN 100
#define SECTION_MAX_LEN 100
#define KEY_MAX_LEN 100
#define VALUE_MAX_LEN 100

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

    int getFileSize(const char *path, long *fileSize);
    int getFileBuffer(const char *path, void *buffer, size_t sizeBuffer, size_t sizeFile);
    bool CreateFile(const char *filePath);
    bool WriteFile(const void *content , size_t contentSize, size_t contentNumber, const char *filePath, const char *operationType);
    bool seekAndReadFile(const char *filePath, void *outputBuffer, long int size, long int offset, int origin);
    bool seekAndWriteFile(const char *filePath, void *outputBuffer, long int size, long int offset, int origin);
    bool CheckFileExists(const char *filePath);
    bool CheckDirExists(const char *dirPath);
    bool CreateDir(const char *dirPath);

    esp_err_t formatPartition(const char *PartitionName);

    bool populateDeviceConfigIni(void);
    bool checkAndCreateDirs(void);
    
    bool Ini_gets(const char *Section, const char *Key, char *Result, const char *FilePath);
    bool Ini_getl(const char *Section, const char *Key, long *Result, const char *FilePath);
    bool Ini_puts(const char *Section, const char *Key, const char *Value, const char *FilePath);
    bool Ini_putl(const char *Section, const char *Key, long Value, const char *FilePath);
    bool Ini_hassection(const char *Section, const char *FilePath);
    bool Ini_haskey(const char *Section, const char *Key, const char *FilePath);

    bool ls(const char *PartitionName);
    bool cat(const char *filePath);
    bool catb(const char *filePath);

private:
    // Constructor privado
    FS() {}
    
    esp_err_t mountPartition(const char *PartitionName);
    
    static SemaphoreHandle_t xFSSemaphore;

};

#endif // FS_H