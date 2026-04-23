#ifndef TIME_HELPER_H
#define TIME_HELPER_H

#include <stddef.h>
#include <stdbool.h>

void time_sync_init(void);
void time_wait_for_sync(void);
void time_get_iso8601(char *buffer, size_t size);

#endif