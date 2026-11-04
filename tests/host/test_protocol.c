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

    protocol_encode_state_change(text, sizeof(text), 17, (key_addr_t){ 0, 2 }, PANEL_STATE_LOW, 5400);
    CHECK(strcmp(text, "{\"v\":1,\"t\":\"state_change\",\"seq\":17,\"addr\":\"0.2\",\"state\":\"LOW\",\"age_ms\":5400}") == 0);

    protocol_encode_unassigned_press(text, sizeof(text), (key_addr_t){ 0, 3 });
    CHECK(strcmp(text, "{\"v\":1,\"t\":\"unassigned_press\",\"addr\":\"0.3\"}") == 0);
}

static void test_encode_detects_overflow(void)
{
    char text[32];
    const protocol_segment_t segments[] = { { 0, 4 } };
    CHECK(protocol_encode_hello(text, sizeof(text), "6825dd21a970", "v0.2.0", segments, 1) == 0);
    CHECK(protocol_encode_state_change(text, 10, 1, (key_addr_t){ 0, 0 }, PANEL_STATE_OK, 0) == 0);
}

void test_protocol_run(void)
{
    test_parse_snapshot();
    test_parse_ack();
    test_parse_state_set_and_identify();
    test_parse_rejects_invalid_messages();
    test_encode_messages();
    test_encode_detects_overflow();
}
