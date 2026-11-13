#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

static inline void getSubstring(char* out, const char* src, size_t start, size_t len) {
    if (out == NULL || src == NULL) return;

    size_t i = 0;
    for (; i < len && src[start + i] != '\0'; i++) {
        out[i] = src[start + i];
    }
    out[i] = '\0';
}

#endif