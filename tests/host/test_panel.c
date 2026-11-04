#include "check.h"
#include "debounce.h"
#include "panel.h"
#include "presence.h"

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
    rgb_t color = panel_led_color(&panel, 1, 0);
    CHECK(color.r == 0 && color.g == 0 && color.b == 0);
}

static void test_brightness_scales_colors(void)
{
    panel_t panel;
    panel_init(&panel, ADDRS, 1);
    rgb_t color = panel_led_color(&panel, 0, 0);
    CHECK(color.r == 0 && color.g == 51 && color.b == 0);
    panel_set_state(&panel, 0, PANEL_STATE_LOW);
    color = panel_led_color(&panel, 0, 0);
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

static void test_identify_blinks_then_returns_to_state(void)
{
    panel_t panel;
    panel_init(&panel, ADDRS, 2);
    panel_set_assigned(&panel, 1, false);
    panel_identify(&panel, 1, 3000, 1000);

    rgb_t lit = panel_led_color(&panel, 1, 1000);
    CHECK(lit.r == 51 && lit.g == 51 && lit.b == 51);
    rgb_t dark = panel_led_color(&panel, 1, 1200);
    CHECK(dark.r == 0 && dark.g == 0 && dark.b == 0);

    rgb_t other = panel_led_color(&panel, 0, 1000);
    CHECK(other.g == 51 && other.r == 0);

    rgb_t after = panel_led_color(&panel, 1, 4000);
    CHECK(after.r == 0 && after.g == 0 && after.b == 0);
}

static void test_find_and_state_names(void)
{
    panel_t panel;
    panel_init(&panel, ADDRS, 2);
    size_t index = 99;
    CHECK(panel_find(&panel, (key_addr_t){ 0, 1 }, &index) && index == 1);
    CHECK(!panel_find(&panel, (key_addr_t){ 1, 0 }, &index));

    panel_state_t state;
    CHECK(panel_state_from_name("LOW", &state) && state == PANEL_STATE_LOW);
    CHECK(!panel_state_from_name("low", &state));
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

void test_panel_run(void)
{
    test_cycle_wraps_to_ok();
    test_unassigned_key_stays_dark_and_unchanged();
    test_brightness_scales_colors();
    test_invalid_saved_state_is_rejected();
    test_identify_blinks_then_returns_to_state();
    test_find_and_state_names();
    test_debounce_single_event_while_held();
    test_debounce_ignores_bounce_and_release();
    test_debounce_survives_timer_wrap();
    test_held_key_at_boot_gives_no_event();
    test_presence_policy();
}
