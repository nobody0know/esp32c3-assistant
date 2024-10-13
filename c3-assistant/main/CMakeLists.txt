idf_component_register(SRCS "es8311_audio/i2s_es8311.c" "gpio_func/gpio_func.c" "./main_init.c"
                    INCLUDE_DIRS "." "es8311_audio" "gpio_func"
                    PRIV_REQUIRES esp_driver_i2s esp_driver_gpio
                    EMBED_FILES "../../mp3_to_pcm/a.pcm")
