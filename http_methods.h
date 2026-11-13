#ifndef HTTP_METHODS_H
#define HTTP_METHODS_H

#include <stddef.h>
#include <string.h>

typedef enum {
    METHOD_INVALID = 0,
    METHOD_GET,
    METHOD_POST,
    METHOD_PUT,
    METHOD_DELETE,
    METHOD_HEAD,
    METHOD_PATCH,
    METHOD_OPTIONS
} HttpMethod;

static inline HttpMethod parse_method(const char* method_str) {
    if (method_str == NULL) return METHOD_INVALID;

    if (strcmp(method_str, "GET") == 0)     return METHOD_GET;
    if (strcmp(method_str, "POST") == 0)    return METHOD_POST;
    if (strcmp(method_str, "PUT") == 0)     return METHOD_PUT;
    if (strcmp(method_str, "DELETE") == 0)  return METHOD_DELETE;
    if (strcmp(method_str, "HEAD") == 0)    return METHOD_HEAD;
    if (strcmp(method_str, "PATCH") == 0)   return METHOD_PATCH;
    if (strcmp(method_str, "OPTIONS") == 0) return METHOD_OPTIONS;

    return METHOD_INVALID;
}

#endif