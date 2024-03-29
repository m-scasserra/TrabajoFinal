#include "includes.h"

extern "C" void app_main(void)
{
    LED &led = LED::getInstance();
    
    CLI &cli = CLI::getInstance();
    E22 &e22 = E22::getInstance();
    IO &io = IO::getInstance();
    io.Begin();
    led.Begin();
    cli.Begin();
    e22.Begin();
    led.SetBrightness(50);

    
    while (1)
    {
        led.SetLedColor(LED::blue);
        vTaskDelay(pdMS_TO_TICKS(1000));
        led.SetLedColor(LED::red);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}