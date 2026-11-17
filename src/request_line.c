#include "request_line.h"
#include "http_methods.h"
#include <string.h>

RequestLineStatus parse_request_line(const char *line, size_t n) {
    int sp1 = -1, sp2 = -1;

    for (size_t i = 0; i < n; i++) {
        // A line can not contain CR or LF at all, these are used for line termination only.
        if (line[i] == 0x0A || line[i] == 0x0D) {
            return REQUEST_LINE_BAD_REQUEST;
        }

        // We must have two spaces only, and they must be separated, not concatenated.
        if (line[i] == 0x20) {
            if (sp1 != -1 && sp2 != -1) {
                return REQUEST_LINE_BAD_REQUEST;
            } else if (sp1 != -1) {
                sp2 = (int)i;
            } else if (i == 0) {
                return REQUEST_LINE_BAD_REQUEST; // leading space
            } else {
                sp1 = (int)i;
            }

            if (sp1 != -1 && sp2 != -1 && sp2 - sp1 == 1) {
                return REQUEST_LINE_BAD_REQUEST;
            }
        }
    }

    // Catch lines with zero/one space and trailing spaces.
    if (sp1 == -1 || sp2 == -1 || sp2 == (int)n - 1) {
        return REQUEST_LINE_BAD_REQUEST;
    }

    char method[16];
    char target[2048];
    char version[16];

    size_t method_len = (size_t)sp1;
    size_t target_len = (size_t)(sp2 - sp1 - 1);
    size_t version_len = n - (size_t)sp2 - 1;

    if (method_len >= sizeof(method) || target_len >= sizeof(target) || version_len >= sizeof(version)) {
        return REQUEST_LINE_BAD_REQUEST;
    }

    memcpy(method, line, method_len);
    method[method_len] = '\0';

    memcpy(target, line + sp1 + 1, target_len);
    target[target_len] = '\0';

    memcpy(version, line + sp2 + 1, version_len);
    version[version_len] = '\0';

    // Method must be A-Z.
    for (size_t i = 0; i < method_len; i++) {
        if (!(method[i] >= 'A' && method[i] <= 'Z')) {
            return REQUEST_LINE_BAD_REQUEST;
        }
    }

    // Every byte of the target must be between 0x21 and 0x7e.
    for (size_t i = 0; i < target_len; i++) {
        unsigned char c = (unsigned char)target[i];

        if (c < 0x21 || c > 0x7e) {
            return REQUEST_LINE_BAD_REQUEST;
        }
    }

    // HTTP/<digit>.<digit>
    if (version_len != 8 ||
            strncmp(version, "HTTP/", 5) != 0 ||
            version[5] < '0' || version[5] > '9' ||
            version[6] != '.' ||
            version[7] < '0' || version[7] > '9') {
        return REQUEST_LINE_BAD_REQUEST;
    }

    // Validate the target structure.
    if (target[0] == '*') {
        if (target_len != 1) {
            return REQUEST_LINE_BAD_REQUEST;
        }
    } else if (target[0] != '/') {
        return REQUEST_LINE_BAD_REQUEST;
    }

    HttpMethod parsed_method = parse_method(method);

    // Make sure the used method is valid.
    if (!parsed_method) {
        return REQUEST_LINE_NOT_IMPLEMENTED;
    }

    // Only OPTIONS may use the asterisk-form target.
    if (target[0] == '*' && parsed_method != METHOD_OPTIONS) {
        return REQUEST_LINE_BAD_REQUEST;
    }

    // Only HTTP/1.0 and HTTP/1.1 are supported.
    if (strcmp(version, "HTTP/1.0") != 0 && strcmp(version, "HTTP/1.1") != 0) {
        return REQUEST_LINE_VERSION_NOT_SUPPORTED;
    }

    return REQUEST_LINE_OK;
}
