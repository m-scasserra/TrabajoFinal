#include "Includes.h"
#include "DeviceStatus.h"

extern "C" void app_main(void)
{
    LED &led = LED::getInstance();
    
    CLI &cli = CLI::getInstance();
    E22 &e22 = E22::getInstance();
    IO &io = IO::getInstance();
    FS &fs = FS::getInstance();
    NETWORK &net = NETWORK::getInstance();
    DEVICETIME &time = DEVICETIME::getInstance();
    AUTOJOB &autojob = AUTOJOB::getInstance();
    DEVICESTATUS &ds = DEVICESTATUS::getInstance();

    fs.Begin();
    time.Begin();
    ds.Begin();
    ds.loadStatusFromFs();

    io.Begin();
    led.Begin();
    e22.Begin();
    e22.setMsgTimeoutms(1000);
    led.SetBrightness(10);
    net.Begin();
    cli.Begin();
    autojob.Begin();

    
    while (1)
    {
        led.SetLedColor(LED::blue);
        vTaskDelay(pdMS_TO_TICKS(1000));
        led.SetLedColor(LED::red);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}