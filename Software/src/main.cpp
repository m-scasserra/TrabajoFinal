#include "includes.h"

extern "C" void app_main(void)
{
    LED &led = LED::getInstance();
    CLI &cli = CLI::getInstance();
    SPI &spi = SPI::getInstance();
    led.Begin();
    cli.ConsoleInit();
    spi.Begin();
    spi.GetStatus();

    while (1)
    {
        led.SetLedColor(LED::blue);
        vTaskDelay(pdMS_TO_TICKS(1000));
        led.SetLedColor(LED::red);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}