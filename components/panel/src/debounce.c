#include "debounce.h"

void debounce_init(debounce_t *debounce, bool raw_now, uint32_t now_ms)
{
    *debounce = (debounce_t){ .raw = raw_now, .stable = raw_now, .raw_change_ms = now_ms };
}

bool debounce_update(debounce_t *debounce, bool raw_now, uint32_t now_ms)
{
    if (raw_now != debounce->raw) {
        debounce->raw = raw_now;
        debounce->raw_change_ms = now_ms;
        return false;
    }
    if (debounce->stable != debounce->raw && (now_ms - debounce->raw_change_ms) >= DEBOUNCE_MS) {
        debounce->stable = debounce->raw;
        return debounce->stable;
    }
    return false;
}
