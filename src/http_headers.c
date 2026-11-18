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

    /*
      spaces are allowed after colon of the header only but neven before it
      Host: Example.com -> fine
      Host:    Example.com -> fine
      Host : Example.com -> invalid, 400 
    */

    size_t line_start = 0;
    unsigned short colon_found = 0;

    for (size_t i = 0; i < n; i++) {
        if (block[i] == ':' && !colon_found) {
            if (i == line_start) {
                return HEADERS_BAD_REQUEST; // Empty name (": value")
            }

            if (out->count >= HTTP_MAX_HEADERS) return HEADERS_TOO_LARGE;

            colon_found = 1;
            out->fields[out->count].name = &block[line_start];
            out->fields[out->count].name_len = i - line_start;

            line_start = i + 1;

        } else if (colon_found == 1) {
            if (block[i] == '\r') {
                // reached the end

                // trim leading/trailing spaces and tabs from the value
                size_t vs = line_start, ve = i;
                while (vs < ve && (block[vs] == ' ' || block[vs] == '\t')) vs++;
                while (ve > vs && (block[ve - 1] == ' ' || block[ve - 1] == '\t')) ve--;

                out->fields[out->count].value = &block[vs];
                out->fields[out->count].value_len = ve - vs;

                out->count++;

                line_start = i + 2;
                i++; // step onto '\n' so the loop's i++ lands on the next line
                colon_found = 0;
            }

        // everything below runs only while we're still reading a header name
        } else if (block[i] == '\r') {
            // a line with no colon is only legal as the final blank line
            if (i == line_start && i == n - 2) break;
            return HEADERS_BAD_REQUEST;
        } else if (block[i] == ' ' || block[i] == '\t') {
            return HEADERS_BAD_REQUEST; // "Host : x"
        }
    }


    return HEADERS_OK;

}