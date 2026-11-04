// Minimalne narzędzia testowe.
#pragma once

#include <stdio.h>

extern int test_failed;

#define CHECK(cond) do { \
        if (!(cond)) { printf("BLAD %s:%d: %s\n", __func__, __LINE__, #cond); test_failed++; } \
    } while (0)

void test_panel_run(void);
void test_protocol_run(void);
void test_sync_run(void);
