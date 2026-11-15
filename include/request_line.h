#ifndef REQUEST_LINE_H
#define REQUEST_LINE_H

#include <stddef.h>

/* Values are the HTTP status codes the server should answer with. */
typedef enum {
    REQUEST_LINE_OK = 200,
    REQUEST_LINE_BAD_REQUEST = 400,
    REQUEST_LINE_NOT_IMPLEMENTED = 501,
    REQUEST_LINE_VERSION_NOT_SUPPORTED = 505
} RequestLineStatus;

/* Validates one request line (CRLF already stripped, `n` excludes it). */
RequestLineStatus parse_request_line(const char *line, size_t n);

#endif
