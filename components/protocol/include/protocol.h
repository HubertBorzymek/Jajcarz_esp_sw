// Wiadomości JSON z docs/PROTOCOL.md. Czysta logika, bez sieci i RTOS.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "jajcarz_types.h"
#include "panel.h"

#define PROTOCOL_VERSION 1

typedef enum {
    PROTOCOL_MSG_SNAPSHOT,
    PROTOCOL_MSG_ACK,
    PROTOCOL_MSG_STATE_SET,
    PROTOCOL_MSG_CONFIG,
    PROTOCOL_MSG_IDENTIFY,
} protocol_msg_type_t;

typedef struct {
    key_addr_t    addr;
    bool          assigned;
    panel_state_t state;    // ważny tylko gdy assigned
    uint32_t      rev;      // ważny tylko gdy assigned
} protocol_key_t;

typedef struct {
    size_t         key_count;
    protocol_key_t keys[PANEL_MAX_KEYS];
    bool           has_config;  // błędna konfiguracja nie unieważnia stanów klawiszy
    panel_config_t config;
} protocol_snapshot_t;

typedef struct {
    uint32_t      seq;
    key_addr_t    addr;
    bool          accepted;
    panel_state_t state;
    uint32_t      rev;
} protocol_ack_t;

typedef struct {
    key_addr_t    addr;
    panel_state_t state;
    uint32_t      rev;
} protocol_state_set_t;

typedef struct {
    key_addr_t addr;
    uint32_t   duration_ms;
} protocol_identify_t;

typedef struct {
    protocol_msg_type_t type;
    union {
        protocol_snapshot_t  snapshot;
        protocol_ack_t       ack;
        protocol_state_set_t state_set;
        panel_config_t       config;
        protocol_identify_t  identify;
    };
} protocol_msg_t;

typedef struct {
    uint8_t id;
    uint8_t key_count;
} protocol_segment_t;

// false dla błędnego JSON, innej wersji protokołu lub nieobsługiwanego typu.
bool protocol_parse(const char *json, protocol_msg_t *msg);

// Zwracają długość tekstu albo 0, gdy nie mieści się w buforze.
size_t protocol_encode_hello(char *buffer, size_t size, const char *device_id, const char *fw,
                             const protocol_segment_t *segments, size_t segment_count);
size_t protocol_encode_state_change(char *buffer, size_t size, uint32_t seq, key_addr_t addr,
                                    panel_state_t state, bool has_age, uint32_t age_ms);
size_t protocol_encode_unassigned_press(char *buffer, size_t size, key_addr_t addr);
