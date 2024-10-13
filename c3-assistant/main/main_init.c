#include "i2s_es8311.h"
#include "gpio_func.h"
static const char *TAG = "MAIN INIT";

void app_main()
{
    gpio_init();
    es8311_user_init();
}