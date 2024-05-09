#include "includes.h"

extern "C" void app_main(void)
{
    esp_log_level_set("SPI", ESP_LOG_DEBUG);
    LED &led = LED::getInstance();
    
    CLI &cli = CLI::getInstance();
    E22 &e22 = E22::getInstance();
    IO &io = IO::getInstance();
    io.Begin();
    led.Begin();
    cli.Begin();
    e22.Begin();
    e22.setMsgTimeoutms(1000);
    led.SetBrightness(10);
    

    
    while (1)
    {
        led.SetLedColor(LED::blue);
        vTaskDelay(pdMS_TO_TICKS(1000));
        led.SetLedColor(LED::red);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}