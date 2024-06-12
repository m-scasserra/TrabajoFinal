/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdio.h>


//#define ESP_LOGE(tag, format, ...) printf("\x1B[0mE [%s] " format "\x1B[0m\n", tag, ##_VA_ARGS_)

#define ESP_LOGE(tag, format, ...)
#define ESP_LOGW(tag, format, ...)
#define ESP_LOGI(tag, format, ...)
#define ESP_LOGD(tag, format, ...)
#define ESP_LOGV(tag, format, ...)

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

//#define ESP_LOGE(tag, format, ...) printf("%sE [%s] " format "%s\n",KRED, tag, ##_VA_ARGS_, KNRM)
//#define ESP_LOGW(tag, format, ...) printf("%sE [%s] " format "%s\n",KYEL, tag, ##_VA_ARGS_, KNRM)
//#define ESP_LOGI(tag, format, ...) printf("%sE [%s] " format "%s\n",KGRN, tag, ##_VA_ARGS_, KNRM)
//#define ESP_LOGD(tag, format, ...) printf("%sE [%s] " format "%s\n",KNRM, tag, ##_VA_ARGS_, KNRM)
//#define ESP_LOGV(tag, format, ...) printf("%sE [%s] " format "%s\n",KNRM, tag, ##_VA_ARGS_, KNRM)


