#include "presence.h"

presence_mode_t presence_get_mode(void)
{
    return PRESENCE_ACTIVE;
}

bool presence_leds_enabled(presence_mode_t mode)
{
    return mode != PRESENCE_SLEEP;
}

bool presence_keys_enabled(presence_mode_t mode)
{
    return mode == PRESENCE_ACTIVE;
}
