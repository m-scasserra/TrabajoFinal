#include "FileSystem.h"
#include "dev/minIni.h"

//Includes para la solucion del format // MES: Esto es por el problema con esp_vfs_fat_spiflash_format_rw_wl, una vez resuleto se puede sacar
#include <stdlib.h>
#include <string.h>
#include "esp_check.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "vfs_fat_internal.h"
#include "diskio_impl.h"
#include "diskio_rawflash.h"
#include "wear_levelling.h"
#include "diskio_wl.h"

char FS::IniPath[MAX_PATH_LEN];

void FS::Begin(void)
{
    //mountPartition(PORTAL_PARTITION_NAME);
    //mountPartition(STORAGE_PARTITION_NAME);
    // esp_log_level_set(FSTAG, ESP_LOG_ERROR);
}

/*bool FS::checkPortalFiles(void)
{
    bool rc = true;
    struct stat fileStat;
    char FilePath[MAX_PATH_LEN];

    memset(FilePath, 0, sizeof(FilePath));
    snprintf(FilePath, MAX_PATH_LEN, "/%s/%s", PORTAL_PARTITION_NAME, ROOT_FILENAME);
    if (!stat(FilePath, &fileStat))
    {
        ESP_LOGI(FSTAG, "index.html   Size: %ld", fileStat.st_size);
    }
    else
    {
        ESP_LOGI(FSTAG, "index.html not found the filesystem.");
        rc = false;
    }

    memset(FilePath, 0, sizeof(FilePath));
    snprintf(FilePath, MAX_PATH_LEN, "/%s/%s", PORTAL_PARTITION_NAME, LOGO_FILENAME);
    if (!stat(FilePath, &fileStat))
    {
        ESP_LOGI(FSTAG, "SaelogoLittle.png   Size: %ld", fileStat.st_size);
    }
    else
    {
        ESP_LOGI(FSTAG, "SaelogoLittle.png not found on the filesystem.");
        rc = false;
    }

    memset(FilePath, 0, sizeof(FilePath));
    snprintf(FilePath, MAX_PATH_LEN, "/%s/%s", PORTAL_PARTITION_NAME, JS_FILENAME);
    if (!stat(FilePath, &fileStat))
    {
        ESP_LOGI(FSTAG, "s.js   Size: %ld", fileStat.st_size);
    }
    else
    {
        ESP_LOGI(FSTAG, "s.js not found the filesystem.");
        rc = false;
    }

    memset(FilePath, 0, sizeof(FilePath));
    snprintf(FilePath, MAX_PATH_LEN, "/%s/%s", PORTAL_PARTITION_NAME, CSS_FILENAME);
    if (!stat(FilePath, &fileStat))
    {
        ESP_LOGI(FSTAG, "style.css   Size: %ld", fileStat.st_size);
    }
    else
    {
        ESP_LOGI(FSTAG, "style.css not found the filesystem.");
        rc = false;
    }
    return rc;
}*/

int FS::getFileSize(const char *path, long *fileSize)
{
    struct stat fileStat;
    int rc;
    rc = stat(path, &fileStat);
    *fileSize = (long)fileStat.st_size;
    return rc;
}

int FS::getFileBuffer(const char *path, void *buffer, size_t sizeBuffer, size_t sizeFile)
{
    FILE *fd = NULL;
    int rc;
    fd = fopen(path, "r");
    if (!fd)
    {
        ESP_LOGE(FSTAG, "Failed to read existing file : %s", path);
        fclose(fd);
        rc = 0;
    }
    else
    {
        rc = fread(buffer, sizeBuffer, sizeFile, fd);
        fclose(fd);
    }
    return rc;
}

bool FS::checkIniFile(void)
{
    bool rc = true;
    struct stat fileStat;

    memset(IniPath, 0, sizeof(IniPath));
    snprintf(IniPath, MAX_PATH_LEN, "/%s%s", STORAGE_PARTITION_NAME, INI_FILENAME);
    if (!stat(IniPath, &fileStat))
    {
        ESP_LOGI(FSTAG, "%s   Size: %ld", INI_FILENAME, fileStat.st_size);
    }
    else
    {
        ESP_LOGI(FSTAG, "%s not found the filesystem.", INI_FILENAME);
        createIni();
        populateIni();
    }
    return rc;
}

bool FS::createIni(void)
{
    ESP_LOGI(FSTAG, "Creating %s", IniPath);
    bool rc = true;
    // Open the file in write mode ("w")
    // FILE *file = fopen(IniPath, "w");
    FILE *file = fopen(IniPath, "w");

    if (file == NULL)
    {
        // Error handling if file opening fails
        ESP_LOGE(FSTAG, "Error creating %s", IniPath);
        return false;
    }
    // Write content to the file using fwrite
    const char *content = "[ARION2]\n[LOG]\n[RM]\n[APN1]\n[APN2]\n[APN3]\n[APN4]\n[ENDPOINT1]\n[ENDPOINT2]\n[ENDPOINT3]\n[ENDPOINT4]";
    size_t content_size = strlen(content);

    // Use fwrite to write the content to the file
    size_t bytes_written = fwrite(content, 1, content_size, file);

    if (bytes_written != content_size)
    {
        // Error handling if not all bytes are written
        ESP_LOGE(FSTAG, "Error writing to file %s", IniPath);
        rc = false;
    }
    fclose(file);
    return rc;
}

bool FS::populateIni(void)
{
    // Populo el config.ini de los valores predeterminados
    /*
    [APN1]
        APN=saetech.ctimovil.com.ar
        SIM=1
    [APN2]
        APN=saetech.TEmpresas
        SIM=2
    [APN3]
        APN=igprs.claro.com.ar
        SIM=1
    [APN4]
        APN=wap.gprs.unifon.com.ar
        SIM=2
    */
    // Populo el APN privado de Claro
    if (!ini_puts("APN1", "APN", "saetech.ctimovil.com.ar", IniPath))
        return false;
    if (!ini_puts("APN1", "SIM", "1", IniPath))
        return false;
    // Populo el APN privado de Movistar
    if (!ini_puts("APN2", "APN", "saetech.TEmpresas", IniPath))
        return false;
    if (!ini_puts("APN2", "SIM", "2", IniPath))
        return false;
    // Populo el APN publico de Claro
    if (!ini_puts("APN3", "APN", "igprs.claro.com.ar", IniPath))
        return false;
    if (!ini_puts("APN3", "SIM", "1", IniPath))
        return false;
    // Populo el APN publico de Movistar
    if (!ini_puts("APN4", "APN", "wap.gprs.unifon.com.ar", IniPath))
        return false;
    if (!ini_puts("APN4", "SIM", "2", IniPath))
        return false;

    /*
    [ENDPOINT1]
        IP=200.123.98.10
        PORT=5580
        USER=5580
        PASS=5580
        PROTOCOL=MQTT
        APNs=1,2
        TIMEOUT=600
    [ENDPOINT2]
        IP=200.123.98.11
        PORT=5580
        USER=5580
        PASS=5580
        PROTOCOL=MQTT
        APNs=3
        TIMEOUT=600
    */
    // Populo el ENDPOINT 1
    if (!ini_puts("ENDPOINT1", "IP", "200.123.98.10", IniPath))
        return false;
    if (!ini_puts("ENDPOINT1", "PORT", "5580", IniPath))
        return false;
    if (!ini_puts("ENDPOINT1", "USER", "5580", IniPath))
        return false;
    if (!ini_puts("ENDPOINT1", "PASS", "5580", IniPath))
        return false;
    if (!ini_puts("ENDPOINT1", "PROTOCOL", "MQTT", IniPath))
        return false;
    if (!ini_puts("ENDPOINT1", "APNs", "1,2", IniPath))
        return false;
    if (!ini_puts("ENDPOINT1", "TIMEOUT", "600", IniPath))
        return false;
    // Populo el ENDPOINT 2
    if (!ini_puts("ENDPOINT2", "IP", "200.123.98.10", IniPath))
        return false;
    if (!ini_puts("ENDPOINT2", "PORT", "5580", IniPath))
        return false;
    if (!ini_puts("ENDPOINT2", "USER", "5580", IniPath))
        return false;
    if (!ini_puts("ENDPOINT2", "PASS", "5580", IniPath))
        return false;
    if (!ini_puts("ENDPOINT2", "PROTOCOL", "MQTT", IniPath))
        return false;
    if (!ini_puts("ENDPOINT2", "APNs", "3", IniPath))
        return false;
    if (!ini_puts("ENDPOINT2", "TIMEOUT", "600", IniPath))
        return false;

    return true;
}

esp_err_t FS::mountPartition(const char *PartitionName)
{
    char PartitionPath[MAX_PATH_LEN];
    memset(PartitionPath, 0, sizeof(PartitionPath));
    snprintf(PartitionPath, MAX_PATH_LEN, "/%s", PartitionName);
    // Handle del Wear Leveling
    wl_handle_t wl_handle;
    uint64_t totalBytes = 0, freeBytes = 0;
    esp_err_t err;

    // Configuracion del Virtual File System donde vamos a montar la particion FAT
    esp_vfs_fat_mount_config_t fatConfig = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 512,
        .disk_status_check_enable = false};
    // Intento montar la particion del portal al VFS, lo monta con el registro del WearLeveling todo incluido.
    // En caso de error, formatea la particion
    err = esp_vfs_fat_spiflash_mount_rw_wl(PartitionPath, PartitionName, &fatConfig, &wl_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(FSTAG, "Failed to mount FATVFS %s", PartitionPath);
    }
    else
    {
        ESP_LOGI(FSTAG, "Mounted %s", PartitionPath);
    }
    // Imprimo la informacion de la particion montada
    err = esp_vfs_fat_info(PartitionPath, &totalBytes, &freeBytes);
    if (err != ESP_OK)
    {
        ESP_LOGE(FSTAG, "Failed to get FatFs partition information (%s)", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(FSTAG, "%s Partition size: total: %llu, free: %llu", PartitionName, totalBytes, freeBytes);
    }
    return err;
}

esp_err_t FS::formatPartition(const char *PartitionName) //MES: No funciona esp_vfs_fat_spiflash_format_rw_wl. Se arregla en la version del IDF 5.2, actualmente al 2/2024 estamos usando la 5.1.2
{
    esp_err_t err;
    char PartitionPath[MAX_PATH_LEN];
    memset(PartitionPath, 0, sizeof(PartitionPath));
    snprintf(PartitionPath, MAX_PATH_LEN, "/%s", PartitionName);
    ESP_LOGI(FSTAG, "Formating %s", PartitionPath);
    err = esp_vfs_fat_spiflash_format_rw_wl(PartitionPath, PartitionName);
    if (err != ESP_OK)
    {
        ESP_LOGE(FSTAG, "Failed to format %s", PartitionPath);
    }
    else
    {
        ESP_LOGI(FSTAG, "Formatting successful %s", PartitionPath);
    }
    return err;
}

bool FS::Ini_gets(const char *Section, const char *Key, char *Result)
{
    char result_str[INI_MAX_LEN];
    memset(result_str, 0, sizeof(result_str));
    if (!ini_gets(Section, Key, NULL, result_str, sizeof(result_str) / sizeof((result_str)[0]), IniPath))
        return false;
    strcpy(Result, result_str);
    return true;
}

bool FS::Ini_puts(const char *Section, const char *Key, const char *Value)
{
    // if (!ini_puts("ENDPOINT1", "PASS", "5580", IniPath))
    if (!ini_puts(Section, Key, Value, IniPath))
        return false;
    return true;
}

bool FS::ls(const char *PartitionName)
{
    char PartitionPath[MAX_PATH_LEN];
    memset(PartitionPath, 0, sizeof(PartitionPath));
    snprintf(PartitionPath, MAX_PATH_LEN, "/%s", PartitionName);
    bool rc = true;
    DIR *dr;
    struct dirent *en;
    printf("Archivos de %s\r\n", PartitionPath);
    dr = opendir(PartitionPath); // open all directory
    if (dr)
    {
        while ((en = readdir(dr)) != NULL)
        {
            printf("\t%s \n", en->d_name);
            /*cout << " \n"
                 << en->d_name; // print all directory name*/
        }
        closedir(dr); // close all directory
    } else{
        ESP_LOGE(FSTAG, "Error en opendir.");
        rc = false;
    }
    return rc;
}

bool FS::cat(const char *filePath)
{
    FILE *filePointer;
    int character;

    // Open the file in read mode
    filePointer = fopen(filePath, "r");
    
    // Check if the file opened successfully
    if (filePointer == NULL) {
        printf("Unable to open the file %s\r\n", filePath);
        return false; // Exit with an error code
    }
    printf("Contents of %s\r\n", filePath);
    // Read and print each character until the end of the file
    while ((character = fgetc(filePointer)) != EOF) {
        putchar(character);
        fflush(stdout);  // Flush stdout to ensure immediate printing
    }
    printf("\r\n");

    // Close the file
    fclose(filePointer);

    return true; // Exit successfully
}