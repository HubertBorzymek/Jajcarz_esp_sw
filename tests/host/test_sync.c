#include "check.h"
#include "sync.h"

static const key_addr_t ADDRS[] = { { 0, 0 }, { 0, 1 } };

static panel_t s_panel;
static sync_t  s_sync;

static void setup(void)
{
    panel_init(&s_panel, ADDRS, 2);
    sync_init(&s_sync);
}

static void receive_snapshot(panel_state_t state0, uint32_t rev0)
{
    protocol_snapshot_t snapshot = {
        .key_count = 2,
        .keys = {
            { .addr = ADDRS[0], .assigned = true, .state = state0, .rev = rev0 },
            { .addr = ADDRS[1], .assigned = true, .state = PANEL_STATE_OK, .rev = 1 },
        },
    };
    sync_apply_snapshot(&s_sync, &s_panel, &snapshot);
}

static void press(size_t index, uint32_t now_ms)
{
    panel_press(&s_panel, index);
    sync_local_change(&s_sync, index, now_ms);
}

static void test_changes_wait_for_snapshot(void)
{
    setup();
    press(0, 100);
    sync_outgoing_t out;
    CHECK(!sync_next_outgoing(&s_sync, &s_panel, 200, &out));

    receive_snapshot(PANEL_STATE_OK, 5);
    CHECK(sync_next_outgoing(&s_sync, &s_panel, 7000, &out));
    CHECK(out.index == 0 && out.age_ms == 6900);
}

static void test_snapshot_does_not_overwrite_pending_change(void)
{
    setup();
    press(0, 100);
    receive_snapshot(PANEL_STATE_OUT, 5);
    CHECK(s_panel.keys[0].state == PANEL_STATE_LOW);

    setup();
    receive_snapshot(PANEL_STATE_OUT, 5);
    CHECK(s_panel.keys[0].state == PANEL_STATE_OUT);
}

static void test_snapshot_applies_assignment_and_drops_its_pending_change(void)
{
    setup();
    press(0, 100);
    protocol_snapshot_t snapshot = {
        .key_count = 1,
        .keys = { { .addr = ADDRS[0], .assigned = false } },
    };
    sync_apply_snapshot(&s_sync, &s_panel, &snapshot);
    CHECK(!s_panel.keys[0].assigned);
    sync_outgoing_t out;
    CHECK(!sync_next_outgoing(&s_sync, &s_panel, 200, &out));
}

static void test_change_is_sent_once_and_retried_without_ack(void)
{
    setup();
    receive_snapshot(PANEL_STATE_OK, 5);
    press(0, 100);
    sync_outgoing_t out;
    CHECK(sync_next_outgoing(&s_sync, &s_panel, 100, &out));
    CHECK(!sync_next_outgoing(&s_sync, &s_panel, 200, &out));
    CHECK(sync_next_outgoing(&s_sync, &s_panel, 5200, &out));
}

static void test_ack_clears_pending(void)
{
    setup();
    receive_snapshot(PANEL_STATE_OK, 5);
    press(0, 100);
    sync_outgoing_t out;
    sync_next_outgoing(&s_sync, &s_panel, 100, &out);

    protocol_ack_t ack = { .seq = out.seq, .addr = ADDRS[0], .accepted = true, .state = PANEL_STATE_LOW, .rev = 6 };
    CHECK(sync_apply_ack(&s_sync, &s_panel, &ack));
    CHECK(!sync_next_outgoing(&s_sync, &s_panel, 9000, &out));
}

static void test_rejected_change_takes_server_state(void)
{
    setup();
    receive_snapshot(PANEL_STATE_OK, 5);
    press(0, 100);
    sync_outgoing_t out;
    sync_next_outgoing(&s_sync, &s_panel, 100, &out);

    protocol_ack_t ack = { .seq = out.seq, .addr = ADDRS[0], .accepted = false, .state = PANEL_STATE_OUT, .rev = 9 };
    CHECK(sync_apply_ack(&s_sync, &s_panel, &ack));
    CHECK(s_panel.keys[0].state == PANEL_STATE_OUT);
}

static void test_ack_of_older_press_keeps_newer_press(void)
{
    setup();
    receive_snapshot(PANEL_STATE_OK, 5);
    press(0, 100);
    sync_outgoing_t first;
    sync_next_outgoing(&s_sync, &s_panel, 100, &first);
    press(0, 200);
    CHECK(s_panel.keys[0].state == PANEL_STATE_OUT);

    protocol_ack_t ack = { .seq = first.seq, .addr = ADDRS[0], .accepted = true, .state = PANEL_STATE_LOW, .rev = 6 };
    CHECK(!sync_apply_ack(&s_sync, &s_panel, &ack));
    CHECK(s_panel.keys[0].state == PANEL_STATE_OUT);

    sync_outgoing_t second;
    CHECK(sync_next_outgoing(&s_sync, &s_panel, 200, &second));
    CHECK(second.seq != first.seq);
}

static void test_state_set_respects_rev(void)
{
    setup();
    receive_snapshot(PANEL_STATE_OK, 5);

    protocol_state_set_t stale = { .addr = ADDRS[0], .state = PANEL_STATE_OUT, .rev = 5 };
    sync_apply_state_set(&s_sync, &s_panel, &stale);
    CHECK(s_panel.keys[0].state == PANEL_STATE_OK);

    protocol_state_set_t fresh = { .addr = ADDRS[0], .state = PANEL_STATE_OUT, .rev = 6 };
    sync_apply_state_set(&s_sync, &s_panel, &fresh);
    CHECK(s_panel.keys[0].state == PANEL_STATE_OUT);
}

static void test_state_set_does_not_overwrite_pending_change(void)
{
    setup();
    receive_snapshot(PANEL_STATE_OK, 5);
    press(0, 100);
    protocol_state_set_t state_set = { .addr = ADDRS[0], .state = PANEL_STATE_OUT, .rev = 6 };
    sync_apply_state_set(&s_sync, &s_panel, &state_set);
    CHECK(s_panel.keys[0].state == PANEL_STATE_LOW);
}

static void test_reconnect_resends_unconfirmed_change(void)
{
    setup();
    receive_snapshot(PANEL_STATE_OK, 5);
    press(0, 100);
    sync_outgoing_t out;
    sync_next_outgoing(&s_sync, &s_panel, 100, &out);

    sync_on_disconnected(&s_sync);
    CHECK(!sync_next_outgoing(&s_sync, &s_panel, 200, &out));

    receive_snapshot(PANEL_STATE_OK, 5);
    CHECK(sync_next_outgoing(&s_sync, &s_panel, 300, &out));
    CHECK(out.age_ms == 200);
}

static void test_restored_change_is_sent_without_age(void)
{
    setup();
    panel_set_state(&s_panel, 0, PANEL_STATE_OUT);
    sync_restore_key(&s_sync, 0, true, 7);
    receive_snapshot(PANEL_STATE_OK, 9);
    CHECK(s_panel.keys[0].state == PANEL_STATE_OUT);

    sync_outgoing_t out;
    CHECK(sync_next_outgoing(&s_sync, &s_panel, 100, &out));
    CHECK(out.index == 0 && !out.has_age);

    protocol_ack_t ack = { .seq = out.seq, .addr = ADDRS[0], .accepted = true, .state = PANEL_STATE_OUT, .rev = 10 };
    CHECK(sync_apply_ack(&s_sync, &s_panel, &ack));
}

static void test_press_after_restore_has_age_again(void)
{
    setup();
    sync_restore_key(&s_sync, 0, true, 7);
    receive_snapshot(PANEL_STATE_OK, 9);
    press(0, 1000);

    sync_outgoing_t out;
    CHECK(sync_next_outgoing(&s_sync, &s_panel, 1500, &out));
    CHECK(out.has_age && out.age_ms == 500);
}

static void test_restored_rev_rejects_stale_state_set(void)
{
    setup();
    sync_restore_key(&s_sync, 0, false, 7);

    protocol_state_set_t stale = { .addr = ADDRS[0], .state = PANEL_STATE_OUT, .rev = 7 };
    sync_apply_state_set(&s_sync, &s_panel, &stale);
    CHECK(s_panel.keys[0].state == PANEL_STATE_OK);

    sync_outgoing_t out;
    receive_snapshot(PANEL_STATE_OK, 7);
    CHECK(!sync_next_outgoing(&s_sync, &s_panel, 100, &out));
}

void test_sync_run(void)
{
    test_restored_change_is_sent_without_age();
    test_press_after_restore_has_age_again();
    test_restored_rev_rejects_stale_state_set();
    test_changes_wait_for_snapshot();
    test_snapshot_does_not_overwrite_pending_change();
    test_snapshot_applies_assignment_and_drops_its_pending_change();
    test_change_is_sent_once_and_retried_without_ack();
    test_ack_clears_pending();
    test_rejected_change_takes_server_state();
    test_ack_of_older_press_keeps_newer_press();
    test_state_set_respects_rev();
    test_state_set_does_not_overwrite_pending_change();
    test_reconnect_resends_unconfirmed_change();
}
