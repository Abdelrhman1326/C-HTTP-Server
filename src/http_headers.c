#include "http_headers.h"
#include <string.h>

HeadersStatus parse_request_headers(const char *block, size_t n, RequestHeaders *out) {
    // block size must be below the maximum allowed size
    if (n > HTTP_MAX_HEADER_BLOCK) {
        return HEADERS_TOO_LARGE;
    }

    if (n < 4) {
        if (n == 2 && (block[0] == '\r' && block[1] == '\n')) {
            // legal empty headers block request
            memset(out, 0, sizeof *out);
            return HEADERS_OK;
        } else {
            return HEADERS_BAD_REQUEST;
        }
    }

    // block must end with the blank-line CRLF
    if (!(block[n - 4] == '\r' && block[n - 3] == '\n' 
        && block[n - 2] == '\r' && block[n - 1] == '\n')) {
        return HEADERS_BAD_REQUEST;
    }

    // block can't have endividual '\r' or '\n'
    for (size_t i = 0; i < n; i++) {
        if (block[i] == '\r') {
            if (i + 1 == n) {
                return HEADERS_BAD_REQUEST;
            } else if (block[i + 1] != '\n') {
                return HEADERS_BAD_REQUEST;
            }
        } else if (block[i] == '\n') {
            if (i == 0) {
                return HEADERS_BAD_REQUEST;
            } else if (block[i - 1] != '\r') {
                return HEADERS_BAD_REQUEST;
            }
        }
    }

    // we can't have a leading space at the beginning of any header line
    for (size_t i = 0; i < n; i++) {
        if (((block[i] == ' ' || block[i] == '\t') && i == 0) || 
            (i > 1 && (block[i] == ' ' || block[i] == '\t') && block[i - 1] == '\n' && block[i - 2] == '\r')) {
                return HEADERS_BAD_REQUEST;
        }
    }


    /* TODO: field parsing */

    
    return HEADERS_OK;
}