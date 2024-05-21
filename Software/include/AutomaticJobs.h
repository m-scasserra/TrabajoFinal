#ifndef AUTOMATICJOBS_H
#define AUTOMATICJOBS_H

#include <stdint.h>

#define AJTAG "AUTOJOB"
#define MAX_LINE_LENGTH 200
#define MAX_SCHEDULE_LENGTH 50
#define MAX_FUNCTION_NAME_LENGTH 50

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
    static bool deleteJobsFromFs(const char *functionToDelete);
    static bool startJobs(void);
    static bool stopJobs(void);
    static bool Begin(void);
    static bool addAllCallbacks(void);
    static bool printAllCallbacksNames(void);

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

#endif // AUTOMATICJOBS_H