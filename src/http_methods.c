#include "http_methods.h"

#include <string.h>

HttpMethod parse_method(const char *s) {
    if (strcmp(s, "GET") == 0)     return METHOD_GET;
    if (strcmp(s, "HEAD") == 0)    return METHOD_HEAD;
    if (strcmp(s, "POST") == 0)    return METHOD_POST;
    if (strcmp(s, "PUT") == 0)     return METHOD_PUT;
    if (strcmp(s, "DELETE") == 0)  return METHOD_DELETE;
    if (strcmp(s, "OPTIONS") == 0) return METHOD_OPTIONS;
    if (strcmp(s, "PATCH") == 0)   return METHOD_PATCH;
    return METHOD_UNKNOWN;
}
