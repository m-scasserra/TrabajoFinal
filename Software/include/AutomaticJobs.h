#ifndef AUTOMATICJOBS_H
#define AUTOMATICJOBS_H

#include "Includes.h"

#define AJTAG "AUTOJOB"
#define AUTOMATICJOBS_BIN_PATH "/storage/AutomaticJobs/jobs.bin"
#define MAX_LINE_LENGTH 200

class AUTOJOB
{

public:
    // Eliminar las funciones de copia y asignación
    AUTOJOB(const AUTOJOB &) = delete;
    AUTOJOB &operator=(const AUTOJOB &) = delete;

    static AUTOJOB &getInstance()
    {
        static AUTOJOB instance; // Única instancia
        return instance;
    }

    // Define a callback function type (can be modified to fit your specific needs)
    typedef void (*callback_t)(void);

    // Structure to store a callback and its name
    typedef struct callbackNode
    {
        char *name;
        callback_t function;
        struct callbackNode *next;
    } callbackNode_t;

    typedef struct callback_node
    {
        char *name;
        callback_t function;
        struct callback_node *next;
    } callback_node_t;

    static bool loadJobsFromFs(void);
    static bool saveJobsToFs(void);
    static bool startJobs(void);
    static bool Begin(void);

private:
    // Constructor privado
    AUTOJOB() {}

    // Function to create a new callback node
    static callbackNode_t *createCallbackNode(const char *name, callback_t function);

    // Function to add a callback to the list
    static uint32_t addCallback(callbackNode_t **head, const char *name, callback_t function);

    // Function to find a callback by name
    static callback_t getCallback(callbackNode_t *head, const char *name);

    // Function to free the callback list (important for memory management)
    void freeCallbackList(callbackNode_t *head);

    static bool started;
};

#endif // DEVICE_H