/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#pragma once

#include "sdkconfig.h"

/* ES8311 configurations */
#define ES8311_RECV_BUF_SIZE   (2400)
#define ES8311_SAMPLE_RATE     (16000)
#define ES8311_MCLK_MULTIPLE   (384) // If not using 24-bit data width, 256 should be enough
#define ES8311_MCLK_FREQ_HZ    (ES8311_SAMPLE_RATE * ES8311_MCLK_MULTIPLE)
#define ES8311_VOICE_VOLUME    CONFIG_ES8311_VOICE_VOLUME
#if CONFIG_ES8311_MODE_ECHO
#define ES8311_MIC_GAIN        CONFIG_ES8311_MIC_GAIN
#endif

#if !defined(CONFIG_ES8311_BSP)

/* I2C port and GPIOs */
#define I2C_NUM         (0)
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define I2C_SCL_IO      (GPIO_NUM_16)
#define I2C_SDA_IO      (GPIO_NUM_17)
#elif CONFIG_IDF_TARGET_ESP32H2
#define I2C_SCL_IO      (GPIO_NUM_8)
#define I2C_SDA_IO      (GPIO_NUM_9)
#elif CONFIG_IDF_TARGET_ESP32P4
#define I2C_SCL_IO      (GPIO_NUM_8)
#define I2C_SDA_IO      (GPIO_NUM_7)
#else
#define I2C_SCL_IO      (GPIO_NUM_1)
#define I2C_SDA_IO      (GPIO_NUM_0)
#endif

/* I2S port and GPIOs */
#define I2S_NUM         (0)
#if CONFIG_IDF_TARGET_ESP32P4
#define I2S_MCK_IO      (GPIO_NUM_13)
#define I2S_BCK_IO      (GPIO_NUM_12)
#define I2S_WS_IO       (GPIO_NUM_10)
#define I2S_DO_IO       (GPIO_NUM_11)
#define I2S_DI_IO       (GPIO_NUM_9)
#else
#define I2S_MCK_IO      (GPIO_NUM_10)
#define I2S_BCK_IO      (GPIO_NUM_8)
#define I2S_WS_IO       (GPIO_NUM_12)
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define I2S_DO_IO       (GPIO_NUM_18)
#define I2S_DI_IO       (GPIO_NUM_19)
#else
#define I2S_DO_IO       (GPIO_NUM_11)
#define I2S_DI_IO       (GPIO_NUM_7)
#endif
#endif

#else // CONFIG_ES8311_BSP
#include "bsp/esp-bsp.h"
#define I2C_NUM BSP_I2C_NUM

#endif // CONFIG_ES8311_BSP
