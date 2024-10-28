
file(GLOB_RECURSE srcs *.c
                    es8311_audio/i2s_es8311.c 
                    gpio_func/gpio_func.c 
                    ./main_init.c 
                    qmi8658c_imu/qmi8658c.c 
                    esp32_wifi/*.c 
                    udp_com/udp_server.c 
                    epaper/EPD_GUI.c 
                    epaper/EPD.c
                    lvgl_gui/*.c
                    lvgl_gui/font/*.c
                    lvgl_gui/generated/*.c
                    lvgl_gui/generated/guider_fonts/*.c
                    lvgl_gui/custom/*.c
                    lvgl_gui/generated/guider_customer_fonts/*.c
                    )


set(include_dirs 
. es8311_audio 
gpio_func 
qmi8658c_imu 
esp32_wifi 
udp_com 
epaper 
lvgl_gui 
lvgl_gui/font/
lvgl_gui/custom
lvgl_gui/generated/guider_fonts
lvgl_gui/generated/guider_customer_fonts
lvgl_gui/generated/
)


idf_component_register(SRCS ${srcs} 
                    INCLUDE_DIRS ${include_dirs} 
                    PRIV_REQUIRES esp_driver_i2s esp_driver_gpio esp_event esp_netif esp_wifi nvs_flash esp_timer esp_http_client json esp-tls mbedtls)
                    # EMBED_FILES ../../mp3_to_pcm/a.pcm)
