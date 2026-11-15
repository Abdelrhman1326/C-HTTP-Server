#ifndef STRING_UTILS_H
#define STRING_UTILS_H
#include <stddef.h>
#include <string.h>

static inline void getSubstring(char *dest, const char *src, size_t start, size_t len) {
    memcpy(dest, src + start, len);
    dest[len] = '\0';
}

#endif