#ifndef FS_H
#define FS_H

#include "CommonIncludes.h"
#include "dev/minIni.h"
#include "esp_littlefs.h"

#define FSTAG "FS"
#define STORAGE_PARTITION_NAME "storage"
#define AUTOMATIC_JOBS_FOLDER_PATH "/storage/AutomaticJobs"
#define PACKETS_RECEIVED_FOLDER_PATH "/storage/Packets"
#define MEASUREMENTS_FOLDER_PATH "/storage/Measurements"
#define CONFIG_FOLDER_PATH "/storage/Config"

#define DEVICE_CONFIG_FILE_PATH "/storage/Config/deviceConfig.ini"
#define TIME_BIN_PATH "/storage/AutomaticJobs/time.bin"
#define ADC_BIN_PATH "/storage/Measurements/adc.bin"
#define PACKETS_BIN_PATH "/storage/Packets/packets.bin"
#define AUTOMATICJOBS_BIN_PATH "/storage/AutomaticJobs/jobs.bin"

#define INI_MAX_LEN 100
#define PATH_MAX_LEN 100
#define SECTION_MAX_LEN 100
#define KEY_MAX_LEN 100
#define VALUE_MAX_LEN 100

#define MAX_PATH_LEN 100

class FS
{

public:
    // Eliminar las funciones de copia y asignación
    FS(const FS &) = delete;
    FS &operator=(const FS &) = delete;

    // Función para acceder a la instancia única del FS
    static FS &getInstance()
    {
        static FS instance; // Única instancia
        return instance;
    }

    // Doxygen english comments
    /**
     * @brief Initializes the FileSystem
     *
     * @return true If the FileSystem was initialized correctly
     * @return false If there was an error initializing the FileSystem
     */
    void Begin(void);

    // Doxygen english comments

    int getFileSize(const char *path, long *fileSize);

    // Doxygen english comments
    /**
     * @brief Read the content of a file and store it in a buffer
     *
     * @param filePath Path of the file to read
     * @param bufferOut Buffer where the content of the file will be stored
     * @param objectSize Size of the object to read
     * @param objectCount Number of objects to read
     * @return int Number of objects read
     */
    int getFileBuffer(const char *path, void *buffer, size_t sizeBuffer, size_t sizeFile);

    // Doxygen english comments

    bool CreateFile(const char *filePath);

    /**
     * @brief Write the content of a buffer to a file
     *
     * @param content Content to write
     * @param contentSize Size of the content
     * @param contentNumber Number of contents
     * @param filePath Path of the file to write
     * @param operationType Operation type
     * @return true If the content was written correctly
     * @return false If there was an error writing the content
     */
    bool WriteFile(const void *content, size_t contentSize, size_t contentNumber, const char *filePath, const char *operationType);

    /**
    @brief Reads a specific portion of a file into a provided buffer.

    @param filePath The path to the file to be read.
    @param outputBuffer Pointer to the buffer where the data will be stored.
    @param size The number of bytes to read from the file.
    @param offset The number of bytes to skip from the beginning of the file (origin dependent).
    @param origin The reference point for the offset. (SEEK_SET, SEEK_CUR, SEEK_END)
    @return true on success, false otherwise.
    */
    bool seekAndReadFile(const char *filePath, void *outputBuffer, long int size, long int offset, int origin);

    /**
    @brief Writes a specific portion of data into a file at a specified location.

    @param filePath The path to the file to be written.
    @param inputBuffer Pointer to the buffer containing the data to be written.
    @param size The number of bytes to write to the file.
    @param offset The number of bytes to skip from the beginning of the file (origin dependent).
    @param origin The reference point for the offset. (SEEK_SET, SEEK_CUR, SEEK_END)
    @return true on success, false otherwise.
    */
    bool seekAndWriteFile(const char *filePath, void *outputBuffer, long int size, long int offset, int origin);

    /**
    @brief Checks if a file exists in the file system.

    @param filePath The path to the file to be checked.
    @return true if the file exists, false otherwise.
    */
    bool CheckFileExists(const char *filePath);

    /**
    @brief Checks if a directory exists in the file system.

    @param dirPath The path to the directory to be checked.
    @return true if the directory exists, false otherwise.
    */
    bool CheckDirExists(const char *dirPath);

    /**
    @brief Creates a directory in the file system, but checks for existing directory first.

    @param dirPath The path to the directory to be created.
    @return true on success, false otherwise.
    */
    bool CreateDir(const char *dirPath);

    /**
     * @brief Formats a partition
     *
     * @param PartitionName Name of the partition to format
     * @return esp_err_t ESP_OK if the partition was formatted correctly
     * @return esp_err_t ESP_FAIL if there was an error formatting the partition
     */
    esp_err_t formatPartition(const char *PartitionName);

    /**
        @brief Creates a configuration file (DEVICE_CONFIG_FILE_PATH) with default settings for the device.

        @return true on success, false otherwise.
        This function populates a file (DEVICE_CONFIG_FILE_PATH) in INI format with default configuration options for the device. The function uses the Ini_puts function (assumed to be defined elsewhere) to write each key-value pair to the file.
        The following sections and key-value pairs are written to the file:
            DEVICE
                AUTOMATIC ([OFF]/ON) - Automatic mode (enabled/disabled)
                FUNCTION ([NONE]/TX/RX) - Device function (no operation, transmit, receive)
            PACKETTYPE
                TYPE ([LORA]/GFSK/LRFHSS) - Packet modulation type
            TCXO
                DIO3VOLTAGE (1_6/1_7/[1_8]/2_2/2_4/2_7/3_0/3_3) - Voltage level for DIO3 pin
                DIO3DELAY ([10]0-262143) - Delay for DIO3 activation (in microseconds)
            CALIBRATIONS
                DO ([0x7F]0x00-0x7F) - Calibration value
            XTAL
                USE ([Y]/N) - Enable/disable crystal oscillator
                A ([0x12]0x00-0x2F) - Crystal oscillator parameter A
                B ([0x12]0x00-0x2F) - Crystal oscillator parameter B
            FREQUENCY
                FREQUENCY ([915000000]) - Operating frequency
            PACONFIG
                DBM (14/[17]/20/22) - Transmission power (in dBm)
            TXPARAMS
                RAMPTIME (10/20/40/80/200/[800]/1700/3400) - Ramp-up time for transmission (in microseconds)
            MODULATION
                SF (5/6/[7]/8/9/10/11/12) - Spreading factor for LoRa modulation
                BW (7/10/15/20/31/41/62/[125]/250/500) - Bandwidth for LoRa modulation
                CR ([4/5], 4/6, 4/7, 4/8) - Coding rate for LoRa modulation
            PACKETPARAMS
                HEADER ([EXPLICIT]/IMPLICIT) - Packet header type
                PREAMBLELENGTH ([12]) - Preamble length in symbols
                PAYLOADLENGTH ([9]) - Payload length in bytes
                CRC ([ON]/OFF) - Enable/disable Cyclic Redundancy Check (CRC)
                IQTYPE ([STANDARD]/INVERTED) - In-phase/Quadrature (IQ) type
            SYNCWORD
                SYNCWORD ([0x3444]/0x1424) - Synchronization word for packet framing
            RXGAIN
                RXGAIN ([BOOST]/POWERSAVE) - Receiver gain mode (high sensitivity or low power)
            TIMEOUT
                TRANSMIT ([30000]) - Transmission timeout in milliseconds
                RECEIVE ([30000]) - Receive timeout in milliseconds
        If any error occurs while writing to the file, the function returns false. */
    bool populateDeviceConfigIni(void);

    /**
    @bref Checks for the existence of required directories and creates them if missing.

    @return true on success, false otherwise.
    */
    bool checkAndCreateDirs(void);

    /**
     * @brief Gets a string from an INI file
     *
     * @param Section Section of the INI file
     * @param Key Key of the INI file
     * @param Result Result of the INI file
     * @param FilePath Path of the INI file
     * @return true If the string was read correctly
     * @return false If there was an error reading the string
     */
    bool Ini_gets(const char *Section, const char *Key, char *Result, const char *FilePath);

    /**
     * @brief Gets a string from an INI file
     *
     * @param Section Section of the INI file
     * @param Key Key of the INI file
     * @param Result Result of the INI file
     * @param FilePath Path of the INI file
     * @return true If the string was read correctly
     * @return false If there was an error reading the string
     */
    bool Ini_getl(const char *Section, const char *Key, long *Result, const char *FilePath);

    /**
     * @brief Writes a string to an INI file
     *
     * @param Section Section of the INI file
     * @param Key Key of the INI file
     * @param Value Value to write in the INI file
     * @param FilePath Path of the INI file
     * @return true If the string was written correctly
     * @return false If there was an error writing the string
     */
    bool Ini_puts(const char *Section, const char *Key, const char *Value, const char *FilePath);

    /**
     * @brief Writes a long to an INI file
     *
     * @param Section Section of the INI file
     * @param Key Key of the INI file
     * @param Value Value to write in the INI file
     * @param FilePath Path of the INI file
     * @return true If the long was written correctly
     * @return false If there was an error writing the long
     */
    bool Ini_putl(const char *Section, const char *Key, long Value, const char *FilePath);

    /**
     * @brief Check if a section exists in an INI file
     *
     * @param Section Section of the INI file
     * @param FilePath Path of the INI file
     * @return true If the section exists
     * @return false If the section does not exist
     */
    bool Ini_hassection(const char *Section, const char *FilePath);

    /**
     * @brief Check if a key exists in an INI file
     *
     * @param Section Section of the INI file
     * @param Key Key of the INI file
     * @param FilePath Path of the INI file
     * @return true If the key exists
     * @return false If the key does not exist
     */
    bool Ini_haskey(const char *Section, const char *Key, const char *FilePath);

    /**
     * @brief List the files in a directory
     *
     * @param PartitionPath Path of the directory
     * @return true If the files were listed correctly
     * @return false If there was an error listing the files
     */
    bool ls(const char *PartitionName);

    /**
     * @brief Read the content of a file and print it to the console
     *
     * @param filePath Path of the file to read
     * @return true If the file was read and printed correctly
     * @return false If there was an error reading the file
     */
    bool cat(const char *filePath);

    /**
        @brief Reads a file and prints its contents byte by byte in hexadecimal format.

        @param filePath The path to the file to be read.
        @return true on success, false otherwise.
        */
    bool catb(const char *filePath);

private:
    // Constructor privado
    FS() {}

    /**
     * @brief Mounts a partition to the VFS
     *
     * @param PartitionName Name of the partition to mount
     * @return esp_err_t ESP_OK if the partition was mounted correctly
     * @return esp_err_t ESP_FAIL if there was an error mounting the partition
     * @return esp_err_t ESP_ERR_NOT_FOUND if the partition was not found
     * @return esp_err_t Another error occurred
     */
    esp_err_t mountPartition(const char *PartitionName);

    /**
     * @brief Handle to the file system semaphore
     */
    static SemaphoreHandle_t xFSSemaphore;
};

#endif // FS_H