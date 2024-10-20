idf_component_register(SRCS "es8311_audio/i2s_es8311.c" "gpio_func/gpio_func.c" "./main_init.c" "qmi8658c_imu/qmi8658c.c" "esp32_wifi/esp32_wifi.c" "udp_com/udp_server.c" "epaper/EPD_GUI.c" "epaper/EPD.c"
                    INCLUDE_DIRS "." "es8311_audio" "gpio_func" "qmi8658c_imu" "esp32_wifi" "udp_com" "epaper"
                    PRIV_REQUIRES esp_driver_i2s esp_driver_gpio esp_event esp_netif esp_wifi nvs_flash)
                    # EMBED_FILES "../../mp3_to_pcm/a.pcm")
