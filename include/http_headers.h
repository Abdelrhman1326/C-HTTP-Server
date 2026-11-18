#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H

#include <stddef.h>

#define HTTP_MAX_HEADERS 100
#define HTTP_MAX_HEADER_BLOCK (16 * 1024)

/* Values are the HTTP status codes the server should answer with. */
typedef enum {
    HEADERS_OK = 200,
    HEADERS_BAD_REQUEST = 400,
    HEADERS_TOO_LARGE = 431,
    HEADERS_NOT_IMPLEMENTED = 501
} HeadersStatus;

/* Slice into the caller's buffer; not NUL-terminated, OWS already trimmed. */
typedef struct {
    const char *name;
    size_t name_len;
    const char *value;
    size_t value_len;
} HeaderField;

typedef struct {
    HeaderField fields[HTTP_MAX_HEADERS + 1];
    size_t count;

    int has_host;
    int has_content_length;
    size_t content_length;
    int connection_close;
    int connection_keep_alive;
} RequestHeaders;

/* Validates one complete header block (must end with the blank-line CRLF). */
HeadersStatus parse_request_headers(const char *block, size_t n, RequestHeaders *out);

/* ASCII-case-insensitive lookup; NULL when absent. */
const HeaderField *find_header(const RequestHeaders *headers, const char *name);

#endif
