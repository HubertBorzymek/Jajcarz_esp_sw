// Jajcarz - firmware panelu na lodówkę. Dokumentacja: README.md, docs/PROTOCOL.md.
#include "controller.h"

void app_main(void)
{
    controller_init();
    controller_run();
}
