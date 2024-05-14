#include "AutomaticJobs.h"
#include "include/cron.h"
#include "DeviceStatus.h"
#include "Message.h"

bool AUTOJOB::started = false;
AUTOJOB::callbackNode_t *callbackListHead = NULL;

void test_cron_job_sample_callback(cron_job *job)
{
    printf("Cron job sample callback\r\n");
    if ((int)(job->data) == 5)
    {
        printf("Cron job sample callback 5\r\n");
    }
    if ((int)(job->data) == 10)
    {
        printf("Cron job sample callback 10\r\n");
    }

    return;
}

void transmitCronJobCallback(cron_job *job)
{
    E22 &e22 = E22::getInstance();
    IO &io = IO::getInstance();
    MESSAGE &msg = MESSAGE::getInstance();
    DEVICESTATUS &ds = DEVICESTATUS::getInstance();

    static uint32_t msgID = 0;

    uint8_t arr [100];
    memset(arr, 0, sizeof(arr));


    int32_t ADCValue;
    io.getADCOneShotRaw(&ADCValue);
    memcpy(arr, &ADCValue, sizeof(int32_t));
    e22.beginTxPacket();
    e22.writeMessageTxLength((uint8_t *)ADCValue, sizeof(int32_t));
    uint8_t flags = 0xAA;
    memcpy(arr + 4, &flags, sizeof(uint8_t));
    e22.writeMessageTxByte(flags);
    e22.writeMessageTxLength((uint8_t *)msgID, sizeof(int32_t));
    msgID++;
    memcpy(arr + 5, &msgID, sizeof(int32_t));

    e22.transmitPacket(ds.deviceStatus.E22Status.transmitTimeout);
    if (e22.IsInTransaction())
    {
        ESP_LOGI("Cron", "Transmitting...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    if (e22.messageSent())
    {
        Message_t msgAux = msg.processMessageSent(arr);
        msg.saveMessage(msgAux);
    }

    return;
}

void recieveCronJobCallback(cron_job *job)
{
    DEVICESTATUS &ds = DEVICESTATUS::getInstance();
    E22 &e22 = E22::getInstance();
    MESSAGE &msg = MESSAGE::getInstance();
    e22.receivePacket(ds.deviceStatus.E22Status.recieveTimeout);
    if (e22.IsInTransaction())
    {
        ESP_LOGI("Cron", "Receiving...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (e22.messageRecieved())
    {
        uint8_t msgLen = e22.getMessageLength();
        uint8_t messageOut[100];
        memset(messageOut, 0, sizeof(messageOut));
        e22.getMessageRxLength(messageOut, msgLen);

        Message_t msgAux = msg.processMessageRecieved(messageOut);
        msg.saveMessage(msgAux);
    }

    return;
}

void saveTimeCronJobCallback(cron_job *job)
{
    DEVICETIME &time = DEVICETIME::getInstance();
    if (!time.saveTimeToFs())
    {
        ESP_LOGE("CRON", "Failed to save time");
    }

    return;
}

void TareaACronJobCallback(cron_job *job)
{
    DEVICETIME &time = DEVICETIME::getInstance();
    printf("Estoy en la tarea A que se corre cada 10 segundos desde el minuto.\r\n");
    time.printTime();
    return;
}

void TareaBCronJobCallback(cron_job *job)
{
    DEVICETIME &time = DEVICETIME::getInstance();
    printf("Estoy en la tarea B que se corre cada minuto.\r\n");
    time.printTime();
    return;
}

bool AUTOJOB::Begin(void)
{
    DEVICESTATUS &ds = DEVICESTATUS::getInstance();
    if (!addAllCallbacks())
    {
        ESP_LOGE(AJTAG, "Failed to add all callbacks");
        return false;
    }

    if (ds.deviceStatus.automatic == true)
    {
        ESP_LOGI(AJTAG, "Automatic jobs enabled");
        if (startJobs() == false)
        {
            ESP_LOGE(AJTAG, "Failed to start automatic jobs");
            return false;
        }
        else
        {
            ESP_LOGI(AJTAG, "Automatic jobs started");
        }
    }

    return true;
}

bool AUTOJOB::addAllCallbacks(void)
{
    addCallback(&callbackListHead, "Transmit", (callback_t)transmitCronJobCallback);
    addCallback(&callbackListHead, "Recieve", (callback_t)recieveCronJobCallback);
    addCallback(&callbackListHead, "saveTime", (callback_t)saveTimeCronJobCallback);
    addCallback(&callbackListHead, "TareaA", (callback_t)TareaACronJobCallback);
    addCallback(&callbackListHead, "TareaB", (callback_t)TareaBCronJobCallback);
    return true;
}

bool AUTOJOB::printAllCallbacksNames(void)
{
    callbackNode_t *current = callbackListHead;
    printf("Callback list:\r\n");
    while (current != NULL)
    {
        printf("\t%s\r\n", current->name);
        current = current->next;
    }
    return true;
}

bool AUTOJOB::startJobs(void)
{
    if (started == true)
    {
        ESP_LOGE(AJTAG, "Jobs already started");
        return false;
    }

    ESP_LOGI(AJTAG, "Starting automatic jobs");

    ESP_LOGI(AJTAG, "Clearing all jobs");
    cron_job_clear_all();
    ESP_LOGI(AJTAG, "All jobs cleared");

    ESP_LOGI(AJTAG, "Loading jobs from filesystem");
    if (loadJobsFromFs() == false)
    {
        ESP_LOGI(AJTAG, "Failed to load jobs from filesystem");
        return false;
    }
    ESP_LOGI(AJTAG, "Jobs loaded from filesystem");

    ESP_LOGI(AJTAG, "Starting cron");
    if (cron_start() != 0)
    {
        ESP_LOGI(AJTAG, "Failed to start cron");
        return false;
    }
    ESP_LOGI(AJTAG, "Cron started");

    started = true;

    return true;
}

bool AUTOJOB::stopJobs(void)
{
    if (started == false)
    {
        ESP_LOGI(AJTAG, "Jobs already stopped");
        return false;
    }

    ESP_LOGI(AJTAG, "Stopping automatic jobs");

    ESP_LOGI(AJTAG, "Clearing all jobs");
    cron_job_clear_all();
    ESP_LOGI(AJTAG, "All jobs cleared");

    ESP_LOGI(AJTAG, "Stopping cron");
    if (cron_stop() != 0)
    {
        ESP_LOGI(AJTAG, "Failed to stop cron");
        return false;
    }
    ESP_LOGI(AJTAG, "Cron stopped");

    started = false;

    return true;
}

bool AUTOJOB::loadJobsFromFs(void)
{
    FS &fs = FS::getInstance();
    char line[MAX_LINE_LENGTH];
    memset(line, 0, sizeof(line));
    long fileSize = 0;

    if (!fs.CheckFileExists(AUTOMATICJOBS_BIN_PATH))
    {
        ESP_LOGE(AJTAG, "No se ha encontrado el archivo %s", AUTOMATICJOBS_BIN_PATH);
        return false;
    }

    fs.getFileSize(AUTOMATICJOBS_BIN_PATH, &fileSize);
    char *buffer = (char *)malloc(fileSize + 1);
    memset(buffer, 0, fileSize);
    size_t bytesRead = fs.getFileBuffer(AUTOMATICJOBS_BIN_PATH, buffer, sizeof(*buffer), fileSize);
    buffer[bytesRead] = '\0';
    if (bytesRead)
    {
        // Tokenize buffer based on newline
        char *end_token = NULL;
        char *end_str = NULL;

        char *line = strtok_r(buffer, "\n", &end_str);
        while (line != NULL)
        {
            // Tokenize line based on comma
            // Parseo los dos valores separados por una coma
            // Copy extracted values (assuming allocated memory)
            char *scheduleValue = strtok_r(line, ",", &end_token);
            char *functionValue = strtok_r(NULL, ",", &end_token);

            ESP_LOGI(AJTAG, "SCHEDULE: %s FUNCTION: %s\n", scheduleValue, functionValue);
            // Busco la funcion con su nombre
            callback_t callback_func = getCallback(callbackListHead, functionValue);
            if (callback_func == NULL)
            {
                ESP_LOGE(AJTAG, "Callback not found: %s", functionValue);
            }
            else
            {
                // Creo el cronjob
                cron_job_create(scheduleValue, (cron_job_callback)callback_func, NULL);
            }
            // Move to the next line
            line = strtok_r(NULL, "\n", &end_str);
        }
        free(buffer);
        return true;
    }
    free(buffer);
    ESP_LOGE(AJTAG, "Error al leer el archivo %s", AUTOMATICJOBS_BIN_PATH);
    return false;
}

bool AUTOJOB::deleteJobsFromFs(const char *functionToDelete)
{
    FS &fs = FS::getInstance();
    char line[MAX_LINE_LENGTH];
    memset(line, 0, sizeof(line));
    long fileSize = 0;

    if (!fs.CheckFileExists(AUTOMATICJOBS_BIN_PATH))
    {
        ESP_LOGE(AJTAG, "No se ha encontrado el archivo %s", AUTOMATICJOBS_BIN_PATH);
        return false;
    }

    fs.getFileSize(AUTOMATICJOBS_BIN_PATH, &fileSize);
    char *buffer = (char *)malloc(fileSize + 1);
    char *bufferToWrite = (char *)malloc(fileSize + 1);
    memset(buffer, 0, fileSize + 1);
    memset(bufferToWrite, 0, fileSize + 1);
    size_t bytesRead = fs.getFileBuffer(AUTOMATICJOBS_BIN_PATH, buffer, sizeof(*buffer), fileSize);
    buffer[bytesRead] = '\0';
    if (bytesRead)
    {
        // Tokenize buffer based on newline
        char *end_token = NULL;
        char *end_str = NULL;
        char *line = strtok_r(buffer, "\n", &end_str);
        while (line != NULL)
        {
            // Tokenize line based on comma
            // Parseo los dos valores separados por una coma
            char *scheduleValue = strtok_r(line, ",", &end_token);
            char *functionValue = strtok_r(NULL, ",", &end_token);

            ESP_LOGD(AJTAG, "SCHEDULE: %s FUNCTION: %s\n", scheduleValue, functionValue);
            if (strcmp(functionValue, functionToDelete) != 0)
            {
                // Add to bufferToWrite
                strcat(bufferToWrite, scheduleValue);
                strcat(bufferToWrite, ",");
                strcat(bufferToWrite, functionValue);
                strcat(bufferToWrite, "\n");
            }

            // Move to the next line
            line = strtok_r(NULL, "\n", &end_str);
        }
        free(buffer);
        strcat(bufferToWrite, "\0");
        if (!fs.WriteFile(bufferToWrite, strlen(bufferToWrite), 1, AUTOMATICJOBS_BIN_PATH, "w"))
        {
            ESP_LOGE(AJTAG, "Error al escribir en el archivo");
            free(bufferToWrite);
            return false;
        }
        free(bufferToWrite);
        return true;
    }
    free(buffer);
    free(bufferToWrite);
    ESP_LOGE(AJTAG, "Error al leer el archivo %s", AUTOMATICJOBS_BIN_PATH);
    return false;
}

bool AUTOJOB::saveJobsToFs(void)
{
    FS &fs = FS::getInstance();

    char line1[] = "*/5 * * * * *,test\n";
    fs.WriteFile(line1, sizeof(line1) - 1, 1, AUTOMATICJOBS_BIN_PATH, "w");
    return true;
}

// Function to create a new callback node
AUTOJOB::callbackNode_t *AUTOJOB::createCallbackNode(const char *name, callback_t function)
{

    callbackNode_t *node = (callbackNode_t *)malloc(sizeof(callbackNode_t));
    if (node == NULL)
    {
        return NULL; // Handle memory allocation error
    }

    node->name = strdup(name); // Allocate and copy the name string
    if (node->name == NULL)
    {
        free(node);
        return NULL; // Handle memory allocation error for name
    }

    node->function = function;
    node->next = NULL;
    return node;
}

// Function to add a callback to the list
uint32_t AUTOJOB::addCallback(callbackNode_t **head, const char *name, callback_t function)
{
    callbackNode_t *newNode = createCallbackNode(name, function);
    if (newNode == NULL)
    {
        return -1; // Handle memory allocation error
    }

    // Add the new node to the beginning of the list
    newNode->next = *head;
    *head = newNode;
    return 0;
}

AUTOJOB::callback_t AUTOJOB::getCallback(callbackNode_t *head, const char *name)
{
    callbackNode_t *current = head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current->function;
        }
        current = current->next;
    }
    return NULL; // Callback not found
}

void AUTOJOB::freeCallbackList(callbackNode_t *head)
{
    callbackNode_t *current, *next;
    current = head;
    while (current != NULL)
    {
        next = current->next;
        free(current->name); // Free the allocated name string
        free(current);
        current = next;
    }
}
