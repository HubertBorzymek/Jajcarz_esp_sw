// Testy logiki bez płytki. Uruchomienie: tests/host/run.ps1
#include <stdio.h>
#include "debounce.h"
#include "panel.h"
#include "presence.h"

static int s_failed;

#define CHECK(cond) do { \
        if (!(cond)) { printf("BLAD %s:%d: %s\n", __func__, __LINE__, #cond); s_failed++; } \
    } while (0)

static const key_addr_t ADDRS[] = { { 0, 0 }, { 0, 1 } };

static void test_cycle_wraps_to_ok(void)
{
    panel_t panel;
    panel_init(&panel, ADDRS, 2);
    CHECK(panel.keys[0].state == PANEL_STATE_OK);
    panel_press(&panel, 0);
    CHECK(panel.keys[0].state == PANEL_STATE_LOW);
    panel_press(&panel, 0);
    CHECK(panel.keys[0].state == PANEL_STATE_OUT);
    panel_press(&panel, 0);
    CHECK(panel.keys[0].state == PANEL_STATE_OK);
    CHECK(panel.keys[1].state == PANEL_STATE_OK);
}

static void test_unassigned_key_stays_dark_and_unchanged(void)
{
    panel_t panel;
    panel_init(&panel, ADDRS, 2);
    panel_set_assigned(&panel, 1, false);
    CHECK(panel_press(&panel, 1) == PANEL_PRESS_UNASSIGNED);
    CHECK(panel.keys[1].state == PANEL_STATE_OK);
    rgb_t color = panel_led_color(&panel, 1);
    CHECK(color.r == 0 && color.g == 0 && color.b == 0);
}

static void test_brightness_scales_colors(void)
{
    panel_t panel;
    panel_init(&panel, ADDRS, 1);
    rgb_t color = panel_led_color(&panel, 0);
    CHECK(color.r == 0 && color.g == 51 && color.b == 0);
    panel_set_state(&panel, 0, PANEL_STATE_LOW);
    color = panel_led_color(&panel, 0);
    CHECK(color.r == 51 && color.g == 24 && color.b == 0);
}

static void test_invalid_saved_state_is_rejected(void)
{
    panel_t panel;
    panel_init(&panel, ADDRS, 1);
    CHECK(!panel_set_state(&panel, 0, PANEL_STATE_COUNT));
    CHECK(!panel_set_state(&panel, 5, PANEL_STATE_LOW));
    CHECK(panel.keys[0].state == PANEL_STATE_OK);
}

static void test_debounce_single_event_while_held(void)
{
    debounce_t debounce;
    debounce_init(&debounce, false, 0);
    CHECK(!debounce_update(&debounce, true, 100));
    CHECK(!debounce_update(&debounce, true, 110));
    CHECK(debounce_update(&debounce, true, 130));
    CHECK(!debounce_update(&debounce, true, 200));
    CHECK(!debounce_update(&debounce, true, 5000));
}

static void test_debounce_ignores_bounce_and_release(void)
{
    debounce_t debounce;
    debounce_init(&debounce, false, 0);
    CHECK(!debounce_update(&debounce, true, 100));
    CHECK(!debounce_update(&debounce, false, 110));
    CHECK(!debounce_update(&debounce, true, 120));
    CHECK(!debounce_update(&debounce, true, 140));
    CHECK(debounce_update(&debounce, true, 150));
    CHECK(!debounce_update(&debounce, false, 300));
    CHECK(!debounce_update(&debounce, false, 340));
}

static void test_debounce_survives_timer_wrap(void)
{
    debounce_t debounce;
    debounce_init(&debounce, false, 0xFFFFFFF0u);
    CHECK(!debounce_update(&debounce, true, 0xFFFFFFF0u));
    CHECK(debounce_update(&debounce, true, 0xFFFFFFF0u + 40));
}

static void test_held_key_at_boot_gives_no_event(void)
{
    debounce_t debounce;
    debounce_init(&debounce, true, 0);
    CHECK(!debounce_update(&debounce, true, 1000));
}

static void test_presence_policy(void)
{
    CHECK(!presence_leds_enabled(PRESENCE_SLEEP) && !presence_keys_enabled(PRESENCE_SLEEP));
    CHECK(presence_leds_enabled(PRESENCE_DISPLAY) && !presence_keys_enabled(PRESENCE_DISPLAY));
    CHECK(presence_leds_enabled(PRESENCE_ACTIVE) && presence_keys_enabled(PRESENCE_ACTIVE));
}

int main(void)
{
    test_cycle_wraps_to_ok();
    test_unassigned_key_stays_dark_and_unchanged();
    test_brightness_scales_colors();
    test_invalid_saved_state_is_rejected();
    test_debounce_single_event_while_held();
    test_debounce_ignores_bounce_and_release();
    test_debounce_survives_timer_wrap();
    test_held_key_at_boot_gives_no_event();
    test_presence_policy();
    if (s_failed != 0) {
        printf("NIEZALICZONE: %d\n", s_failed);
        return 1;
    }
    printf("Wszystkie testy OK\n");
    return 0;
}
