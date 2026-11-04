// Testy logiki bez płytki. Uruchomienie: tests/host/run.ps1
#include "check.h"

int test_failed;

int main(void)
{
    test_panel_run();
    test_protocol_run();
    test_sync_run();
    if (test_failed != 0) {
        printf("NIEZALICZONE: %d\n", test_failed);
        return 1;
    }
    printf("Wszystkie testy OK\n");
    return 0;
}
