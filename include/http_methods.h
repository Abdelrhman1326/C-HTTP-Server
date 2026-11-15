#ifndef HTTP_METHODS_H
#define HTTP_METHODS_H

typedef enum {
    METHOD_UNKNOWN = 0,
    METHOD_GET,
    METHOD_HEAD,
    METHOD_POST,
    METHOD_PUT,
    METHOD_DELETE,
    METHOD_OPTIONS,
    METHOD_PATCH
} HttpMethod;

/* Returns METHOD_UNKNOWN when the method is not one this server supports. */
HttpMethod parse_method(const char *s);

#endif
