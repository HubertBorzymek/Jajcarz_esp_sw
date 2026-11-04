#include "protocol.h"

#include <stdarg.h>
#include <stdio.h>

// Dopisuje tekst od pozycji `used`. Zwraca nową długość albo 0 po przepełnieniu.
static size_t append(char *buffer, size_t size, size_t used, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int written = vsnprintf(buffer + used, size - used, format, args);
    va_end(args);
    if (written < 0 || (size_t)written >= size - used) {
        return 0;
    }
    return used + (size_t)written;
}

size_t protocol_encode_hello(char *buffer, size_t size, const char *device_id, const char *fw,
                             const protocol_segment_t *segments, size_t segment_count)
{
    size_t used = append(buffer, size, 0, "{\"v\":%d,\"t\":\"hello\",\"device_id\":\"%s\",\"fw\":\"%s\",\"segments\":[",
                         PROTOCOL_VERSION, device_id, fw);
    for (size_t i = 0; used != 0 && i < segment_count; i++) {
        used = append(buffer, size, used, "%s{\"seg\":%u,\"keys\":%u}", i == 0 ? "" : ",",
                      segments[i].id, segments[i].key_count);
    }
    return used == 0 ? 0 : append(buffer, size, used, "]}");
}

size_t protocol_encode_state_change(char *buffer, size_t size, const protocol_state_change_t *change)
{
    return append(buffer, size, 0,
                  "{\"v\":%d,\"t\":\"state_change\",\"seq\":%lu,\"addr\":\"%u.%u\",\"state\":\"%s\",\"%s\":%lu}",
                  PROTOCOL_VERSION, (unsigned long)change->seq, change->addr.seg, change->addr.key,
                  panel_state_name(change->state),
                  change->has_age ? "age_ms" : "base_rev",
                  (unsigned long)(change->has_age ? change->age_ms : change->base_rev));
}

size_t protocol_encode_unassigned_press(char *buffer, size_t size, key_addr_t addr)
{
    return append(buffer, size, 0, "{\"v\":%d,\"t\":\"unassigned_press\",\"addr\":\"%u.%u\"}",
                  PROTOCOL_VERSION, addr.seg, addr.key);
}
