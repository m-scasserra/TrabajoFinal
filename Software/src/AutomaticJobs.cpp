#include "AutomaticJobs.h"
#include "include/cron.h"

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

bool AUTOJOB::Begin(void)
{
    addCallback(&callbackListHead, "test", (callback_t)test_cron_job_sample_callback);
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
    char *buffer = (char *)malloc(fileSize);
    size_t bytesRead = fs.getFileBuffer(AUTOMATICJOBS_BIN_PATH, buffer, sizeof(*buffer), fileSize);
    buffer[bytesRead] = '\0';
    if (bytesRead)
    {
        // Tokenize buffer based on newline
        char *end_str;
        char *line = strtok_r(buffer, "\n", &end_str);
        while (line != NULL)
        {
            // Process each line (in this example, just print it)
            char *end_token;
            // Tokenize line based on comma
            // Parseo los dos valores separados por una coma
            char *scheduleValue = strtok_r(line, ",", &end_token);
            char *functionValue = strtok_r(NULL, ",", &end_token);
            ESP_LOGD(AJTAG, "SCHEDULE: %s FUNCTION: %s\n", scheduleValue, functionValue);
            // Busco la funcion con su nombre
            callback_t callback_func = getCallback(callbackListHead, functionValue);
            // Creo el cronjob
            cron_job_create(scheduleValue, (cron_job_callback)callback_func, NULL);

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