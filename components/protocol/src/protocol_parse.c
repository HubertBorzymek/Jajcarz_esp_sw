#include "protocol.h"

#include <stdio.h>
#include <string.h>
#include "cJSON.h"

static bool parse_addr(const cJSON *item, key_addr_t *addr)
{
    if (!cJSON_IsString(item)) {
        return false;
    }
    unsigned seg, key;
    char tail;
    if (sscanf(item->valuestring, "%u.%u%c", &seg, &key, &tail) != 2 || seg > UINT8_MAX || key > UINT8_MAX) {
        return false;
    }
    *addr = (key_addr_t){ .seg = (uint8_t)seg, .key = (uint8_t)key };
    return true;
}

static bool parse_state(const cJSON *item, panel_state_t *state)
{
    return cJSON_IsString(item) && panel_state_from_name(item->valuestring, state);
}

static bool parse_u32(const cJSON *item, uint32_t *value)
{
    if (!cJSON_IsNumber(item) || item->valuedouble < 0 || item->valuedouble > UINT32_MAX) {
        return false;
    }
    *value = (uint32_t)item->valuedouble;
    return true;
}

static bool parse_snapshot_key(const cJSON *item, protocol_key_t *key)
{
    if (!parse_addr(cJSON_GetObjectItemCaseSensitive(item, "addr"), &key->addr)) {
        return false;
    }
    key->assigned = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(item, "assigned"));
    if (!key->assigned) {
        return true;
    }
    return parse_state(cJSON_GetObjectItemCaseSensitive(item, "state"), &key->state)
           && parse_u32(cJSON_GetObjectItemCaseSensitive(item, "rev"), &key->rev);
}

static bool parse_snapshot(const cJSON *root, protocol_snapshot_t *snapshot)
{
    const cJSON *keys = cJSON_GetObjectItemCaseSensitive(root, "keys");
    if (!cJSON_IsArray(keys)) {
        return false;
    }
    snapshot->key_count = 0;
    const cJSON *item;
    cJSON_ArrayForEach(item, keys) {
        if (snapshot->key_count >= PANEL_MAX_KEYS
            || !parse_snapshot_key(item, &snapshot->keys[snapshot->key_count])) {
            return false;
        }
        snapshot->key_count++;
    }
    return true;
}

static bool parse_ack(const cJSON *root, protocol_ack_t *ack)
{
    const cJSON *accepted = cJSON_GetObjectItemCaseSensitive(root, "accepted");
    if (!cJSON_IsBool(accepted)) {
        return false;
    }
    ack->accepted = cJSON_IsTrue(accepted);
    return parse_u32(cJSON_GetObjectItemCaseSensitive(root, "seq"), &ack->seq)
           && parse_addr(cJSON_GetObjectItemCaseSensitive(root, "addr"), &ack->addr)
           && parse_state(cJSON_GetObjectItemCaseSensitive(root, "state"), &ack->state)
           && parse_u32(cJSON_GetObjectItemCaseSensitive(root, "rev"), &ack->rev);
}

static bool parse_state_set(const cJSON *root, protocol_state_set_t *state_set)
{
    return parse_addr(cJSON_GetObjectItemCaseSensitive(root, "addr"), &state_set->addr)
           && parse_state(cJSON_GetObjectItemCaseSensitive(root, "state"), &state_set->state)
           && parse_u32(cJSON_GetObjectItemCaseSensitive(root, "rev"), &state_set->rev);
}

static bool parse_identify(const cJSON *root, protocol_identify_t *identify)
{
    return parse_addr(cJSON_GetObjectItemCaseSensitive(root, "addr"), &identify->addr)
           && parse_u32(cJSON_GetObjectItemCaseSensitive(root, "duration_ms"), &identify->duration_ms);
}

static bool parse_body(const cJSON *root, protocol_msg_t *msg)
{
    const cJSON *version = cJSON_GetObjectItemCaseSensitive(root, "v");
    const cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "t");
    if (!cJSON_IsNumber(version) || version->valueint != PROTOCOL_VERSION || !cJSON_IsString(type)) {
        return false;
    }
    if (strcmp(type->valuestring, "snapshot") == 0) {
        msg->type = PROTOCOL_MSG_SNAPSHOT;
        return parse_snapshot(root, &msg->snapshot);
    }
    if (strcmp(type->valuestring, "ack") == 0) {
        msg->type = PROTOCOL_MSG_ACK;
        return parse_ack(root, &msg->ack);
    }
    if (strcmp(type->valuestring, "state_set") == 0) {
        msg->type = PROTOCOL_MSG_STATE_SET;
        return parse_state_set(root, &msg->state_set);
    }
    if (strcmp(type->valuestring, "identify") == 0) {
        msg->type = PROTOCOL_MSG_IDENTIFY;
        return parse_identify(root, &msg->identify);
    }
    return false;
}

bool protocol_parse(const char *json, protocol_msg_t *msg)
{
    cJSON *root = cJSON_Parse(json);
    if (root == NULL) {
        return false;
    }
    bool ok = parse_body(root, msg);
    cJSON_Delete(root);
    return ok;
}
