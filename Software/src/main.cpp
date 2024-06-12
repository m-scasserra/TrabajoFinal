#include "CommonIncludes.h"
#include "DeviceStatus.h"
#include "Led.h"
#include "CLI.h"
#include "cmd_system.h"
#include "Hardware.h"
#include "FileSystem.h"
#include "E22Opcodes.h"
#include "E22Driver.h"
#include "IO.h"
#include "Network.h"
#include "DeviceTime.h"
#include "AutomaticJobs.h"


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
    net.Begin();

    io.Begin();
    led.Begin();
    e22.Begin();
    led.SetBrightness(10);
    cli.Begin();
    autojob.Begin();

    switch (ds.deviceStatus.mode)
    {
    case DEVICESTATUS::NONE:
    {
        ESP_LOGI("Main", "Modo NONE");
    }
    break;
    case DEVICESTATUS::TX:
    {
        ESP_LOGI("Main", "Modo TX");
        e22.setUpForTx();
    }
    break;
    case DEVICESTATUS::RX:
    {
        ESP_LOGI("Main", "Modo RX");
        e22.setUpForRx();
    }
    break;
    default:
    {
        ESP_LOGI("Main", "Modo Desconocido");
    }
    break;
    }

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}