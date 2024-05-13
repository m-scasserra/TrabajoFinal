#include "FileSystem.h"

SemaphoreHandle_t FS::xFSSemaphore;

void FS::Begin(void)
{
    xFSSemaphore = xSemaphoreCreateBinary();
    if (xFSSemaphore == NULL)
    {
        ESP_LOGE(FSTAG, "Error al crear el SemaphoreFS");
        return;
    }
    xSemaphoreGive(xFSSemaphore);

    if (mountPartition(STORAGE_PARTITION_NAME) == ESP_OK)
    {
        ESP_LOGI(FSTAG, "Particion montada");
        ESP_LOGI(FSTAG, "Verificando existencia de directorios");
        if (!checkAndCreateDirs())
        {
            ESP_LOGE(FSTAG, "Error al crear directorios");
            return;
        }

        if (!CheckFileExists(DEVICE_CONFIG_FILE_PATH))
        {
            ESP_LOGI(FSTAG, "%s no encontrado. Creando ...", DEVICE_CONFIG_FILE_PATH);
            if (!CreateFile(DEVICE_CONFIG_FILE_PATH))
            {
                ESP_LOGE(FSTAG, "Error al crear el archivo %s", DEVICE_CONFIG_FILE_PATH);
                return;
            }
            ESP_LOGI(FSTAG, "Se ha creado el archivo %s", DEVICE_CONFIG_FILE_PATH);
            
            ESP_LOGI(FSTAG, "Populando archivo %s", DEVICE_CONFIG_FILE_PATH);
            if (!populateDeviceConfigIni())
            {
                ESP_LOGE(FSTAG, "Error al crear el archivo %s", DEVICE_CONFIG_FILE_PATH);
                return;
            }
            ESP_LOGI(FSTAG, "Populado archivo %s", DEVICE_CONFIG_FILE_PATH);
        }
    }

    // esp_log_level_set(FSTAG, ESP_LOG_ERROR);
}

bool FS::checkAndCreateDirs(void)
{
    if (CheckDirExists(AUTOMATIC_JOBS_FOLDER_PATH) == false)
    {
        ESP_LOGI(FSTAG, "Carpeta AutomaticJobs no encontrada. Creando carpeta");
        if (!CreateDir(AUTOMATIC_JOBS_FOLDER_PATH))
        {
            ESP_LOGE(FSTAG, "Error creando carpeta %s", AUTOMATIC_JOBS_FOLDER_PATH);
            return false;
        }
    }
    else
    {
        ESP_LOGI(FSTAG, "Carpeta AutomaticJobs encontrada");
    }

    if (CheckDirExists(PACKETS_RECEIVED_FOLDER_PATH) == false)
    {
        ESP_LOGI(FSTAG, "Carpeta PacketsReceived no encontrada. Creando carpeta");
        if (!CreateDir(PACKETS_RECEIVED_FOLDER_PATH))
        {
            ESP_LOGE(FSTAG, "Error creando carpeta %s", PACKETS_RECEIVED_FOLDER_PATH);
            return false;
        }
    }
    else
    {
        ESP_LOGI(FSTAG, "Carpeta PacketsReceived encontrada");
    }

    if (CheckDirExists(MEASUREMENTS_FOLDER_PATH) == false)
    {
        ESP_LOGI(FSTAG, "Carpeta Measurements no encontrada. Creando carpeta");
        if (!CreateDir(MEASUREMENTS_FOLDER_PATH))
        {
            ESP_LOGE(FSTAG, "Error creando carpeta %s", MEASUREMENTS_FOLDER_PATH);
            return false;
        }
    }
    else
    {
        ESP_LOGI(FSTAG, "Carpeta Measurements encontrada");
    }

    if (CheckDirExists(CONFIG_FOLDER_PATH) == false)
    {
        ESP_LOGI(FSTAG, "Carpeta Config no encontrada. Creando carpeta");
        if (!CreateDir(CONFIG_FOLDER_PATH))
        {
            ESP_LOGE(FSTAG, "Error creando carpeta %s", CONFIG_FOLDER_PATH);
            return false;
        }
    }
    else
    {
        ESP_LOGI(FSTAG, "Carpeta Config encontrada");
    }
    return true;
}

bool FS::CheckFileExists(const char *filePath)
{
    bool rc = true;
    struct stat fileStat;

    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        if (!stat(filePath, &fileStat))
        {
            ESP_LOGI(FSTAG, "%s   Size: %ld", filePath, fileStat.st_size);
        }
        else
        {
            ESP_LOGI(FSTAG, "%s not found the filesystem.", filePath);
            rc = false;
        }
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc;
}

bool FS::CheckDirExists(const char *dirPath)
{
    bool rc = true;

    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        DIR *dir = opendir(dirPath);

        if (dir)
        {
            closedir(dir);
            ESP_LOGI(FSTAG, "%s found in the filesystem.", dirPath);
            rc = true;
        }
        else
        {
            ESP_LOGE(FSTAG, "%s not found in the filesystem.", dirPath);
            rc = false;
        }
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc;
}

bool FS::CreateDir(const char *dirPath)
{
    bool rc = true;

    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        DIR *dir = opendir(dirPath);

        if (dir)
        {
            closedir(dir);
            ESP_LOGI(FSTAG, "%s already found in the filesystem.", dirPath);
            rc = true;
        }
        else
        {
            ESP_LOGI(FSTAG, "%s not found in the filesystem.", dirPath);
            if (mkdir(dirPath, 0777) == 0)
            {
                ESP_LOGI(FSTAG, "%s created in the filesystem.", dirPath);
                rc = true;
            }
            else
            {
                ESP_LOGE(FSTAG, "%s not created in the filesystem.", dirPath);
                rc = false;
            }
        }
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc;
}

int FS::getFileSize(const char *path, long *fileSize)
{
    struct stat fileStat;
    int rc = -1;
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        rc = stat(path, &fileStat);
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
    }

    *fileSize = (long)fileStat.st_size;
    return rc;
}

int FS::getFileBuffer(const char *filePath, void *bufferOut, size_t objectSize, size_t objectCount)
{
    FILE *fd = NULL;
    int rc = -1;
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        fd = fopen(filePath, "r");
        if (!fd)
        {
            ESP_LOGE(FSTAG, "Failed to read existing file : %s", filePath);
            rc = 0;
        }
        else
        {
            rc = fread(bufferOut, objectSize, objectCount, fd);
        }
        fclose(fd);
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
    }

    return rc;
}

esp_err_t FS::mountPartition(const char *PartitionName)
{
    char PartitionPath[MAX_PATH_LEN];
    memset(PartitionPath, 0, sizeof(PartitionPath));
    snprintf(PartitionPath, MAX_PATH_LEN, "/%s", PartitionName);

    esp_vfs_littlefs_conf_t partitionConf;
    memset(&partitionConf, 0, sizeof(esp_vfs_littlefs_conf_t));

    partitionConf.base_path = PartitionPath;
    partitionConf.partition_label = PartitionName;
    partitionConf.format_if_mount_failed = true;
    partitionConf.dont_mount = false;
    partitionConf.read_only = false;
    partitionConf.grow_on_mount = false;

    // Intento montar la particion al VFS, en caso de error, formatea la particion
    esp_err_t err = esp_vfs_littlefs_register(&partitionConf);
    if (err != ESP_OK)
    {
        if (err == ESP_FAIL)
        {
            ESP_LOGE(FSTAG, "Failed to mount or format filesystem");
        }
        else if (err == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(FSTAG, "Failed to find LittleFS partition %s", PartitionPath);
        }
        else
        {
            ESP_LOGE(FSTAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(err));
        }
        return err;
    }
    else
    {
        ESP_LOGI(FSTAG, "Mounted %s", PartitionPath);
    }

    // Imprimo la informacion de la particion montada
    size_t totalBytes = 0, usedBytes = 0;
    err = esp_littlefs_info(partitionConf.partition_label, &totalBytes, &usedBytes);
    if (err != ESP_OK)
    {
        ESP_LOGE(FSTAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(FSTAG, "Partition size: totalBytes: %d, usedBytes: %d", totalBytes, usedBytes);
    }
    return err;
}

esp_err_t FS::formatPartition(const char *PartitionName)
{
    esp_err_t err;
    ESP_LOGI(FSTAG, "Formating %s", PartitionName);
    err = esp_littlefs_format(PartitionName);
    if (err != ESP_OK)
    {
        ESP_LOGE(FSTAG, "Failed to format %s", PartitionName);
    }
    else
    {
        ESP_LOGI(FSTAG, "Formatting successful %s", PartitionName);
    }
    return err;
}

bool FS::Ini_gets(const char *Section, const char *Key, char *Result, const char *FilePath)
{
    char result_str[INI_MAX_LEN];
    memset(result_str, 0, sizeof(result_str));
    bool rc = true;
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        if (!ini_gets(Section, Key, NULL, result_str, sizeof(result_str) / sizeof((result_str)[0]), FilePath))
        {
            rc = false;
        }
        else
        {
            strcpy(Result, result_str);
        }
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc;
}

bool FS::Ini_getl(const char *Section, const char *Key, long *Result, const char *FilePath)
{
    char result_str[INI_MAX_LEN];
    memset(result_str, 0, sizeof(result_str));
    bool rc = true;
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        *Result = ini_getl(Section, Key, -1, FilePath);

        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc;
}

bool FS::Ini_puts(const char *Section, const char *Key, const char *Value, const char *FilePath)
{
    bool rc = true;
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        if (!ini_puts(Section, Key, Value, FilePath))
        {
            rc = false;
        }
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc;
}

bool FS::Ini_putl(const char *Section, const char *Key, long Value, const char *FilePath)
{
    bool rc = true;
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        if (!ini_putl(Section, Key, Value, FilePath))
        {
            rc = false;
        }
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc;
}

bool FS::Ini_hassection(const char *Section, const char *FilePath)
{
    bool rc = true;
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        if (!ini_hassection(Section, FilePath))
        {
            rc = false;
        }
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc;
}

bool FS::Ini_haskey(const char *Section, const char *Key, const char *FilePath)
{
    bool rc = true;
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        if (!ini_haskey(Section, Key, FilePath))
        {
            rc = false;
        }
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc;
}

bool FS::ls(const char *PartitionPath)
{
    bool rc = true;
    DIR *dir;
    struct dirent *entry;
    struct stat file_info;
    printf("Archivos de %s\r\n", PartitionPath);

    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        dir = opendir(PartitionPath); // open all directory
        if (dir)
        {
            while ((entry = readdir(dir)) != NULL)
            {
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", PartitionPath, entry->d_name);

                // Get file info
                if (stat(full_path, &file_info) != 0)
                {
                    perror("stat");
                    continue;
                }

                // Check if entry is a directory
                if (entry->d_type == DT_DIR)
                {
                    printf("[%s]\n", entry->d_name);
                }
                else
                {
                    // Print file name and last modification time
                    printf("%s\t", entry->d_name);
                    printf("Last Modified: %s", ctime(&file_info.st_mtime));
                }
            }
            closedir(dir); // close all directory
        }
        else
        {
            ESP_LOGE(FSTAG, "Error en opendir.");
            rc = false;
        }

        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc;
}

bool FS::cat(const char *filePath)
{
    FILE *filePointer;
    int character;
    bool rc = true;

    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {

        // Open the file in read mode
        filePointer = fopen(filePath, "r");

        // Check if the file opened successfully
        if (filePointer == NULL)
        {
            printf("Unable to open the file %s\r\n", filePath);
            fclose(filePointer);
            xSemaphoreGive(xFSSemaphore);
            return false; // Exit with an error code
        }
        printf("Contents of %s\r\n", filePath);
        // Read and print each character until the end of the file
        while ((character = fgetc(filePointer)) != EOF)
        {
            putchar(character);
            fflush(stdout); // Flush stdout to ensure immediate printing
        }
        printf("\r\n");

        // Close the file
        fclose(filePointer);
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc; // Exit successfully
}

bool FS::catb(const char *filePath)
{
    FILE *filePointer;
    bool rc = true;
    uint8_t byte;

    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {

        // Open the file in read mode
        filePointer = fopen(filePath, "r");

        // Check if the file opened successfully
        if (filePointer == NULL)
        {
            printf("Unable to open the file %s\r\n", filePath);
            fclose(filePointer);
            xSemaphoreGive(xFSSemaphore);
            return false; // Exit with an error code
        }
        printf("Contents of %s\r\n", filePath);
        // Read and print each character until the end of the file
        while (fread(&byte, sizeof(uint8_t), 1, filePointer) == 1)
        {
            printf("0x%02X ", byte);
            fflush(stdout); // Flush stdout to ensure immediate printing
        }
        printf("\r\n");

        // Close the file
        fclose(filePointer);
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc; // Exit successfully
}

bool FS::CreateFile(const char *filePath)
{
    ESP_LOGI(FSTAG, "Creating %s", filePath);
    bool rc = true;
    // Open the file in write mode ("w")
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        FILE *file = fopen(filePath, "w");

        if (file == NULL)
        {
            // Error handling if file opening fails
            ESP_LOGE(FSTAG, "Error creating %s", filePath);
            xSemaphoreGive(xFSSemaphore);
            return false;
        }

        fclose(file);
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        rc = false;
    }

    return rc;
}

bool FS::WriteFile(const void *content, size_t contentSize, size_t contentNumber, const char *filePath, const char *operationType)
{
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        // Open file in binary write mode, handle errors
        FILE *file = fopen(filePath, operationType);
        if (file == NULL)
        {
            ESP_LOGE(FSTAG, "Error opening file %s", filePath);
            xSemaphoreGive(xFSSemaphore);
            return false;
        }
        if (fwrite(content, contentSize, contentNumber, file) != contentNumber)
        {
            ESP_LOGE(FSTAG, "Error writing characters");
            fclose(file);
            xSemaphoreGive(xFSSemaphore);
            return false;
        }
        fclose(file);
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        return false;
    }
    return true;
}

bool FS::seekAndReadFile(const char *filePath, void *outputBuffer, long int size, long int offset, int origin)
{
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        FILE *file = fopen(filePath, "r");
        if (file == NULL)
        {
            ESP_LOGE(FSTAG, "Error opening file %s", filePath);
            xSemaphoreGive(xFSSemaphore);
            return false;
        }
        if (fseek(file, offset, origin) != 0)
        {
            ESP_LOGE(FSTAG, "Error seeking file %s", filePath);
            fclose(file);
            xSemaphoreGive(xFSSemaphore);
            return false;
        }
        if (fread(outputBuffer, size, 1, file) != 1)
        {
            ESP_LOGE(FSTAG, "Error reading file %s", filePath);
            fclose(file);
            xSemaphoreGive(xFSSemaphore);
            return false;
        }
        fclose(file);
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        return false;
    }
    return true;
}

bool FS::seekAndWriteFile(const char *filePath, void *inputBuffer, long int size, long int offset, int origin)
{
    if (xSemaphoreTake(xFSSemaphore, portMAX_DELAY) == pdTRUE)
    {
        FILE *file = fopen(filePath, "r+");
        if (file == NULL)
        {
            ESP_LOGE(FSTAG, "Error opening file %s", filePath);
            xSemaphoreGive(xFSSemaphore);
            return false;
        }
        if (fseek(file, offset, origin) != 0)
        {
            ESP_LOGE(FSTAG, "Error seeking file %s", filePath);
            fclose(file);
            xSemaphoreGive(xFSSemaphore);
            return false;
        }
        if (fwrite(inputBuffer, size, 1, file) != 1)
        {
            ESP_LOGE(FSTAG, "Error writing file %s", filePath);
            fclose(file);
            xSemaphoreGive(xFSSemaphore);
            return false;
        }
        fclose(file);
        xSemaphoreGive(xFSSemaphore);
    }
    else
    {
        ESP_LOGE(FSTAG, "File System Busy");
        return false;
    }
    return true;
}

bool FS::populateDeviceConfigIni(void)
{
    // Populo el config.ini de los valores predeterminados
    /*
    [DEVICE]
        AUTOMATIC=OFF
    [PACKETTYPE]
        TYPE=LORA
    [TCXO]
        DIO3VOLTAGE=1.8
        DIO3DELAY=10
    [CALIBRATIONS]
        DO=0x7F
    [XTAL]
        USE=Y
        A=0x12
        B=0x12
    [FREQUENCY]
        FREQUENCY=915000000
    [PACONFIG]
        DBM=17
    [TXPARAMS]
        RAMPTIME=800
    [MODULATION]
        SF=7
        BW=12
        CR=4/5
    [PACKETPARAMS]
        HEADER=EXPLICIT
        PREAMBLELENGTH=12
        PAYLOADLENGTH=15
        CRC=ON
        IQTYPE=STANDARD
    [SYNCWORD]
        SYNCWORD=0x3444
    [RXGAIN]
        RXGAIN=BOOST
    [TIMEOUT]
        TRANSMIT=30000
        RECIEVE=30000
    */
    if (!Ini_puts("DEVICE", "AUTOMATIC", "OFF", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("PACKETTYPE", "TYPE", "LORA", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("TCXO", "DIO3VOLTAGE", "1_8", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("TCXO", "DIO3DELAY", "10", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("CALIBRATIONS", "DO", "0x7F", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("XTAL", "USE", "Y", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("XTAL", "A", "0x12", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("XTAL", "B", "0x12", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("FREQUENCY", "FREQUENCY", "915000000", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("PACONFIG", "DBM", "17", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("TXPARAMS", "RAMPTIME", "800", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("MODULATION", "SF", "7", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("MODULATION", "BW", "125", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("MODULATION", "CR", "4/5", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("PACKETPARAMS", "HEADER", "EXPLICIT", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("PACKETPARAMS", "PREAMBLELENGTH", "12", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("PACKETPARAMS", "PAYLOADLENGTH", "15", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("PACKETPARAMS", "CRC", "ON", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("PACKETPARAMS", "IQTYPE", "STANDARD", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("SYNCWORD", "SYNCWORD", "0x3444", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("RXGAIN", "RXGAIN", "BOOST", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("TIMEOUT", "TRANSMIT", "30000", DEVICE_CONFIG_FILE_PATH))
        return false;
    if (!Ini_puts("TIMEOUT", "RECIEVE", "30000", DEVICE_CONFIG_FILE_PATH))
        return false;
    return true;
}