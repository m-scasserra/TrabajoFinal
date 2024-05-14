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
    net.Begin();

    io.Begin();
    led.Begin();
    e22.Begin();
    led.SetBrightness(10);
    cli.Begin();
    autojob.Begin();

    switch (ds.deviceStatus.mode)
    {
    case NONE:
    {
        ESP_LOGI(E22TAG, "Modo NONE");
    }
    break;
    case TX:
    {
        ESP_LOGI(E22TAG, "Modo TX");
        e22.setUpForTx();
    }
    break;
    case RX:
    {
        ESP_LOGI(E22TAG, "Modo RX");
        e22.setUpForRx();
    }
    break;
    default:
    {
        ESP_LOGI(E22TAG, "Modo Desconocido");
    }
    break;
    }

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}