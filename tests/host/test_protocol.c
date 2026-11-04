// Teksty JSON pochodzą z przykładów w docs/PROTOCOL.md.
#include <string.h>
#include "check.h"
#include "protocol.h"

static protocol_msg_t s_msg;

static void test_parse_snapshot(void)
{
    const char *json =
        "{ \"v\": 1, \"t\": \"snapshot\", \"rev\": 128,"
        "  \"keys\": ["
        "    { \"addr\": \"0.0\", \"assigned\": true, \"state\": \"OK\", \"rev\": 121 },"
        "    { \"addr\": \"0.1\", \"assigned\": true, \"state\": \"OUT\", \"rev\": 128 },"
        "    { \"addr\": \"0.2\", \"assigned\": false }"
        "  ],"
        "  \"config\": { \"brightness\": 51,"
        "    \"colors\": { \"OK\": [0, 255, 0], \"LOW\": [255, 120, 0], \"OUT\": [255, 0, 0] } } }";
    CHECK(protocol_parse(json, &s_msg));
    CHECK(s_msg.type == PROTOCOL_MSG_SNAPSHOT);
    CHECK(s_msg.snapshot.key_count == 3);
    CHECK(s_msg.snapshot.keys[1].state == PANEL_STATE_OUT && s_msg.snapshot.keys[1].rev == 128);
    CHECK(!s_msg.snapshot.keys[2].assigned && s_msg.snapshot.keys[2].addr.key == 2);
    CHECK(s_msg.snapshot.has_config);
    CHECK(s_msg.snapshot.config.brightness == 51);
    CHECK(s_msg.snapshot.config.colors[PANEL_STATE_LOW].r == 255);
    CHECK(s_msg.snapshot.config.colors[PANEL_STATE_LOW].g == 120);
}

static void test_snapshot_survives_invalid_config(void)
{
    const char *json = "{ \"v\": 1, \"t\": \"snapshot\", \"rev\": 1,"
                       "  \"keys\": [ { \"addr\": \"0.0\", \"assigned\": true, \"state\": \"OK\", \"rev\": 1 } ],"
                       "  \"config\": { \"brightness\": 999 } }";
    CHECK(protocol_parse(json, &s_msg));
    CHECK(s_msg.snapshot.key_count == 1);
    CHECK(!s_msg.snapshot.has_config);

    const char *without_config = "{ \"v\": 1, \"t\": \"snapshot\", \"rev\": 1, \"keys\": [] }";
    CHECK(protocol_parse(without_config, &s_msg));
    CHECK(!s_msg.snapshot.has_config);
}

static void test_parse_config(void)
{
    const char *json = "{ \"v\": 1, \"t\": \"config\", \"brightness\": 30,"
                       "  \"colors\": { \"OK\": [1, 2, 3], \"LOW\": [4, 5, 6], \"OUT\": [7, 8, 9] } }";
    CHECK(protocol_parse(json, &s_msg));
    CHECK(s_msg.type == PROTOCOL_MSG_CONFIG);
    CHECK(s_msg.config.brightness == 30);
    CHECK(s_msg.config.colors[PANEL_STATE_OK].b == 3);
    CHECK(s_msg.config.colors[PANEL_STATE_OUT].r == 7);

    CHECK(!protocol_parse("{ \"v\": 1, \"t\": \"config\", \"brightness\": 256,"
                          "  \"colors\": { \"OK\": [1, 2, 3], \"LOW\": [4, 5, 6], \"OUT\": [7, 8, 9] } }", &s_msg));
    CHECK(!protocol_parse("{ \"v\": 1, \"t\": \"config\", \"brightness\": 30,"
                          "  \"colors\": { \"OK\": [1, 2], \"LOW\": [4, 5, 6], \"OUT\": [7, 8, 9] } }", &s_msg));
    CHECK(!protocol_parse("{ \"v\": 1, \"t\": \"config\", \"brightness\": 30,"
                          "  \"colors\": { \"OK\": [1, 2, 3], \"LOW\": [4, 5, 6] } }", &s_msg));
    CHECK(!protocol_parse("{ \"v\": 1, \"t\": \"config\", \"brightness\": 30,"
                          "  \"colors\": { \"OK\": [1, 2, -3], \"LOW\": [4, 5, 6], \"OUT\": [7, 8, 9] } }", &s_msg));
}

static void test_parse_ack(void)
{
    const char *json = "{ \"v\": 1, \"t\": \"ack\", \"seq\": 17, \"addr\": \"0.2\", \"accepted\": false,"
                       "  \"state\": \"LOW\", \"rev\": 129 }";
    CHECK(protocol_parse(json, &s_msg));
    CHECK(s_msg.type == PROTOCOL_MSG_ACK);
    CHECK(s_msg.ack.seq == 17 && !s_msg.ack.accepted && s_msg.ack.state == PANEL_STATE_LOW);
}

static void test_parse_state_set_and_identify(void)
{
    const char *state_set = "{ \"v\": 1, \"t\": \"state_set\", \"addr\": \"1.3\", \"state\": \"OK\", \"rev\": 130,"
                            "  \"changed_by\": \"web:user1\", \"changed_at\": \"2026-11-04T17:20:31Z\" }";
    CHECK(protocol_parse(state_set, &s_msg));
    CHECK(s_msg.type == PROTOCOL_MSG_STATE_SET);
    CHECK(s_msg.state_set.addr.seg == 1 && s_msg.state_set.addr.key == 3 && s_msg.state_set.rev == 130);

    const char *identify = "{ \"v\": 1, \"t\": \"identify\", \"addr\": \"0.2\", \"duration_ms\": 3000 }";
    CHECK(protocol_parse(identify, &s_msg));
    CHECK(s_msg.type == PROTOCOL_MSG_IDENTIFY && s_msg.identify.duration_ms == 3000);
}

static void test_parse_rejects_invalid_messages(void)
{
    CHECK(!protocol_parse("to nie json", &s_msg));
    CHECK(!protocol_parse("{ \"v\": 2, \"t\": \"ack\" }", &s_msg));
    CHECK(!protocol_parse("{ \"v\": 1, \"t\": \"nieznany\" }", &s_msg));
    CHECK(!protocol_parse("{ \"v\": 1, \"t\": \"ack\", \"seq\": 1 }", &s_msg));
    CHECK(!protocol_parse("{ \"v\": 1, \"t\": \"identify\", \"addr\": \"0.2x\", \"duration_ms\": 1 }", &s_msg));
    CHECK(!protocol_parse("{ \"v\": 1, \"t\": \"identify\", \"addr\": \"300.2\", \"duration_ms\": 1 }", &s_msg));
    CHECK(!protocol_parse("{ \"v\": 1, \"t\": \"state_set\", \"addr\": \"0.1\", \"state\": \"NIE\", \"rev\": 1 }", &s_msg));
    CHECK(!protocol_parse("{ \"v\": 1, \"t\": \"state_set\", \"addr\": \"0.1\", \"state\": \"OK\", \"rev\": -1 }", &s_msg));
}

static void test_encode_messages(void)
{
    char text[256];
    const protocol_segment_t segments[] = { { 0, 4 }, { 2, 8 } };
    size_t length = protocol_encode_hello(text, sizeof(text), "6825dd21a970", "v0.2.0", segments, 2);
    CHECK(length == strlen(text));
    CHECK(strcmp(text, "{\"v\":1,\"t\":\"hello\",\"device_id\":\"6825dd21a970\",\"fw\":\"v0.2.0\","
                       "\"segments\":[{\"seg\":0,\"keys\":4},{\"seg\":2,\"keys\":8}]}") == 0);

    protocol_state_change_t with_age = {
        .seq = 17, .addr = { 0, 2 }, .state = PANEL_STATE_LOW, .has_age = true, .age_ms = 5400,
    };
    protocol_encode_state_change(text, sizeof(text), &with_age);
    CHECK(strcmp(text, "{\"v\":1,\"t\":\"state_change\",\"seq\":17,\"addr\":\"0.2\",\"state\":\"LOW\",\"age_ms\":5400}") == 0);

    protocol_state_change_t after_restart = {
        .seq = 18, .addr = { 0, 2 }, .state = PANEL_STATE_OUT, .has_age = false, .base_rev = 121,
    };
    protocol_encode_state_change(text, sizeof(text), &after_restart);
    CHECK(strcmp(text, "{\"v\":1,\"t\":\"state_change\",\"seq\":18,\"addr\":\"0.2\",\"state\":\"OUT\",\"base_rev\":121}") == 0);

    protocol_encode_unassigned_press(text, sizeof(text), (key_addr_t){ 0, 3 });
    CHECK(strcmp(text, "{\"v\":1,\"t\":\"unassigned_press\",\"addr\":\"0.3\"}") == 0);
}

static void test_encode_detects_overflow(void)
{
    char text[32];
    const protocol_segment_t segments[] = { { 0, 4 } };
    CHECK(protocol_encode_hello(text, sizeof(text), "6825dd21a970", "v0.2.0", segments, 1) == 0);
    protocol_state_change_t change = { .seq = 1, .state = PANEL_STATE_OK, .has_age = true };
    CHECK(protocol_encode_state_change(text, 10, &change) == 0);
}

void test_protocol_run(void)
{
    test_parse_snapshot();
    test_snapshot_survives_invalid_config();
    test_parse_config();
    test_parse_ack();
    test_parse_state_set_and_identify();
    test_parse_rejects_invalid_messages();
    test_encode_messages();
    test_encode_detects_overflow();
}
