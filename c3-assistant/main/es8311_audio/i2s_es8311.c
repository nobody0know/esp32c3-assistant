/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "esp_system.h"
#include "esp_check.h"
#include "es8311.h"
#include "es8311_config.h"
#include "gpio_func.h"
#include "common.h"
#include "lsm6dso.h"
// #include "esp_efuse_table.h"

extern QueueHandle_t gpio_evt_queue;
extern QueueHandle_t imu_evt_queue;

static const char *TAG = "i2s_es8311";
static const char err_reason[][30] = {"input param is invalid",
                                      "operation timeout"};
static i2s_chan_handle_t tx_handle = NULL;
static i2s_chan_handle_t rx_handle = NULL;

static es8311_handle_t es_handle;

/* Import music file as buffer */
#if CONFIG_ES8311_MODE_MUSIC
extern const uint8_t music_pcm_start[] asm("_binary_a_pcm_start");
extern const uint8_t music_pcm_end[] asm("_binary_a_pcm_end");
#endif

static esp_err_t es8311_codec_init(void)
{
    /* Initialize es8311 codec */
    es_handle = es8311_create(I2C_NUM, ES8311_ADDRRES_0);
    ESP_RETURN_ON_FALSE(es_handle, ESP_FAIL, TAG, "es8311 create failed");
    const es8311_clock_config_t es_clk = {
        .mclk_inverted = false,
        .sclk_inverted = false,
        .mclk_from_mclk_pin = true,
        .mclk_frequency = ES8311_MCLK_FREQ_HZ,
        .sample_frequency = ES8311_SAMPLE_RATE};

    ESP_ERROR_CHECK(es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16));
    ESP_RETURN_ON_ERROR(es8311_sample_frequency_config(es_handle, ES8311_SAMPLE_RATE * ES8311_MCLK_MULTIPLE, ES8311_SAMPLE_RATE), TAG, "set es8311 sample frequency failed");
    ESP_RETURN_ON_ERROR(es8311_voice_volume_set(es_handle, 70, NULL), TAG, "set es8311 volume failed");
    ESP_RETURN_ON_ERROR(es8311_microphone_config(es_handle, false), TAG, "set es8311 microphone failed");
#if CONFIG_ES8311_MODE_ECHO
    ESP_RETURN_ON_ERROR(es8311_microphone_gain_set(es_handle, ES8311_MIC_GAIN), TAG, "set es8311 microphone gain failed");
#endif
    return ESP_OK;
}

static esp_err_t i2s_driver_init(void)
{
#if !defined(CONFIG_ES8311_BSP)
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true; // Auto clear the legacy data in the DMA buffer
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(ES8311_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_MCK_IO,
            .bclk = I2S_BCK_IO,
            .ws = I2S_WS_IO,
            .dout = I2S_DO_IO,
            .din = I2S_DI_IO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    std_cfg.clk_cfg.mclk_multiple = ES8311_MCLK_MULTIPLE;

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));
#else
    ESP_LOGI(TAG, "Using BSP for HW configuration");
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(ES8311_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = BSP_I2S_GPIO_CFG,
    };
    std_cfg.clk_cfg.mclk_multiple = ES8311_MCLK_MULTIPLE;
    ESP_ERROR_CHECK(bsp_audio_init(&std_cfg, &tx_handle, &rx_handle));
    ESP_ERROR_CHECK(bsp_audio_poweramp_enable(true));
#endif
    return ESP_OK;
}

#if CONFIG_ES8311_MODE_MUSIC
static void i2s_music(void *args)
{
    esp_err_t ret = ESP_OK;
    size_t bytes_write = 0;
    uint8_t *data_ptr = (uint8_t *)music_pcm_start;
    uint32_t io_num;
    uint32_t atti_flag;
    int8_t door_state = -1;

    /* (Optional) Disable TX channel and preload the data before enabling the TX channel,
     * so that the valid data can be transmitted immediately */
    ESP_ERROR_CHECK(i2s_channel_disable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_preload_data(tx_handle, data_ptr, music_pcm_end - data_ptr, &bytes_write));
    data_ptr += bytes_write; // Move forward the data pointer

    /* Enable the TX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    while (1)
    {
        if ((xQueueReceive(gpio_evt_queue, &io_num, 10) == pdTRUE)) //||(xQueueReceive(imu_evt_queue, &atti_flag, 100)==pdTRUE))
        {
            door_state = -door_state;
            int16_t gyro_x=0,gyro_y=0,gyro_z=0;
            ESP_LOGI(TAG,"get imu/gpio INT isr");
            lsm6dso_read_gyroscope(&gyro_x, &gyro_y, &gyro_z);
            printf("gyro x is %d\n", gyro_x);
            if (door_state == 1 && abs(gyro_x) > 2000) // mean human has enter house
            {
                ESP_LOGI(TAG,"play the sound");
                // vTaskDelay(3000 / portTICK_PERIOD_MS);
                es8311_power_on(es_handle);
                /* Write music to earphone */
                ret = i2s_channel_write(tx_handle, data_ptr, music_pcm_end - data_ptr, &bytes_write, portMAX_DELAY);
                if (ret != ESP_OK)
                {
                    /* Since we set timeout to 'portMAX_DELAY' in 'i2s_channel_write'
                       so you won't reach here unless you set other timeout value,
                       if timeout detected, it means write operation failed. */
                    ESP_LOGE(TAG, "[music] i2s write failed, %s", err_reason[ret == ESP_ERR_TIMEOUT]);
                    abort();
                }
                if (bytes_write > 0)
                {
                    ESP_LOGI(TAG, "[music] i2s music played, %d bytes are written.", bytes_write);
                }
                else
                {
                    ESP_LOGE(TAG, "[music] i2s music play failed.");
                    abort();
                }
                data_ptr = (uint8_t *)music_pcm_start;
                vTaskDelay(3000 / portTICK_PERIOD_MS);
                es8311_power_down(es_handle);
            }
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

#else
static void i2s_echo(void *args)
{
    int *mic_data = malloc(ES8311_RECV_BUF_SIZE);
    if (!mic_data)
    {
        ESP_LOGE(TAG, "[echo] No memory for read data buffer");
        abort();
    }
    esp_err_t ret = ESP_OK;
    size_t bytes_read = 0;
    size_t bytes_write = 0;
    ESP_LOGI(TAG, "[echo] Echo start");

    while (1)
    {
        memset(mic_data, 0, ES8311_RECV_BUF_SIZE);
        /* Read sample data from mic */
        ret = i2s_channel_read(rx_handle, mic_data, ES8311_RECV_BUF_SIZE, &bytes_read, 1000);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "[echo] i2s read failed, %s", err_reason[ret == ESP_ERR_TIMEOUT]);
            abort();
        }
        /* Write sample data to earphone */
        ret = i2s_channel_write(tx_handle, mic_data, ES8311_RECV_BUF_SIZE, &bytes_write, 1000);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "[echo] i2s write failed, %s", err_reason[ret == ESP_ERR_TIMEOUT]);
            abort();
        }
        if (bytes_read != bytes_write)
        {
            ESP_LOGW(TAG, "[echo] %d bytes read but only %d bytes are written", bytes_read, bytes_write);
        }
    }
    vTaskDelete(NULL);
}
#endif

void es8311_user_init(void)
{
    /* Wait for wifi to get weather data */
    extern EventGroupHandle_t my_event_group;
    xEventGroupWaitBits(my_event_group, WIFI_GET_RTWEATHER_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    printf("i2s es8311 codec ES8311 start\n-----------------------------\n");
    /* Initialize i2s peripheral */
    if (i2s_driver_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "i2s driver init failed");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "i2s driver init success");
    }
    /* Initialize i2c peripheral and config es8311 codec by i2c */
    if (es8311_codec_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "es8311 codec init failed");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "es8311 codec init success");
    }
    es8311_power_down(es_handle);
#if CONFIG_ES8311_MODE_MUSIC
    /* Play a piece of music in music mode */
    xTaskCreate(i2s_music, "i2s_music", 4096, NULL, 5, NULL);
#else
    /* Echo the sound from MIC in echo mode */
    xTaskCreate(i2s_echo, "i2s_echo", 8192, NULL, 5, NULL);
#endif
}
