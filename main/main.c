#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bt.h"
#include "zphs01b.h"

void app_main(void)
{
    bt_init();
    init_and_run_zphs01b();
}
