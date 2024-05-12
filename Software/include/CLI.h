#ifndef CLI_H
#define CLI_H

#include "Includes.h"
#include "esp_clk_tree.h"

#define CLITAG "CLI"

#define MSG_QUEUE_LEN 20
#define MSG_MAX_LEN 200
#define QUEUE_TIMEOUT 100

#define CONSOLE_PROMPT "SATE>"
#define MAX_CMDLINE_LEN 60

class CLI
{

public:
    // Eliminar las funciones de copia y asignación
    CLI(const CLI &) = delete;
    CLI &operator=(const CLI &) = delete;


    static CLI &getInstance()
    {
        static CLI instance; // Única instancia
        return instance;
    }

    static void Begin(void);
    static void PrintLogo(void);

private:
    // Constructor privado
    CLI() {}

    static void esp_console_register_all_commands(void);
    static esp_err_t esp_console_register_simple_command(const char *commandtext, const char *help, esp_console_cmd_func_t func);

    static esp_err_t esp_console_register_timer_command(void);

    static int timerCmdFunc(int argc, char **argv);
    static esp_err_t esp_console_register_config_command(void);

    static int configCmdFunc(int argc, char **argv);
    static esp_err_t esp_console_register_FS_command(void);

    static int FSCmdFunc(int argc, char **argv);
    static esp_err_t esp_console_register_net_command(void);

    static int timeCmdFunc(int argc, char **argv);
    static esp_err_t esp_console_register_time_command(void);

    static int AutoJobCmdFunc(int argc, char **argv);
    static esp_err_t esp_console_register_AutoJob_command(void);

    static int netCmdFunc(int argc, char **argv);
    static int showStatusCMD(int argc, char **argv);
    static int transmitCMD(int argc, char **argv);
    static int recieveCMD(int argc, char **argv);
    static void PrintSeparator(void);

    // Argumentos para el comando timer
    struct timercmd_args_t
    {
        struct arg_str *command;
        struct arg_str *name;
        struct arg_int *limit;
        struct arg_end *end;
    };
    static timercmd_args_t timer_args;
    
    // Argumentos para el comando config
    struct configcmd_args_t
    {
        struct arg_str *command;
        struct arg_str *section;
        struct arg_str *key;
        struct arg_str *value;
        struct arg_end *end;
    };
    static configcmd_args_t config_args;

    struct FScmd_args_t
    {
        struct arg_str *command;
        struct arg_str *path;
        struct arg_end *end;
    };
    static FScmd_args_t FS_args;

    struct netcmd_args_t
    {
        struct arg_str *command;
        struct arg_str *ssid;
        struct arg_str *pass;
        struct arg_end *end;
    };
    static netcmd_args_t net_args;

    struct timecmd_args_t
    {
        struct arg_str *command;
        struct arg_str *ntpserver;
        struct arg_end *end;
    };
    static timecmd_args_t time_args;

    struct AJcmd_args_t
    {
        struct arg_str *command;
        struct arg_str *job;
        struct arg_end *end;
    };
    static AJcmd_args_t AJ_args;

};

#endif // CLI