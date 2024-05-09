#ifndef INCLUDES_H
#define INCLUDES_H

// Standard Libraries
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h>

// Esp Libraries
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_console.h"
#include "esp_pm.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "lwip/inet.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "esp_timer.h"
#include "esp_http_server.h"
#include "esp_vfs.h"
#include "esp_ota_ops.h"
#include "esp_check.h"
#include "soc/clk_tree_defs.h"

// Private Libraries
#include "Led.h"
#include "CLI.h"
#include "cmd_system.h"
#include "Hardware.h"
#include "SPI.h"
#include "FileSystem.h"
#include "E22Opcodes.h"
#include "E22Driver.h"
#include "IO.h"

#endif //INCLUDES_H