#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_utils.h"
#include "http_methods.h"

/* Decide what to print for ONE line (already cut at CRLF, CRLF not included).
   Print exactly one of: "OK <method> <target> <version>", "400", "501", "505". */
static void judge(const char *line, size_t n) {
    /* TODO 1: a CR or LF left inside the line means it was not terminated by CRLF -> 400.
       TODO 2: exactly three non-empty fields separated by single spaces -> else 400.
       TODO 3: method only A-Z; target starts with '/' (or is "*" for OPTIONS) and every byte
               is 0x21..0x7e; version is HTTP/<digit>.<digit>  -> any failure is 400.
       TODO 4: method not in GET HEAD POST PUT DELETE OPTIONS PATCH -> 501.
       TODO 5: version not HTTP/1.0 or HTTP/1.1 -> 505. */

    
    int sp1 = -1, sp2 = -1;

    for (size_t i = 0; i < n; i++) {
        // line can not contain CR of LF at all, these are used for line termination only
        if (line[i] == 0x0A || line[i] == 0x0D) {
            printf("%d\n", 400); // bad request
            return;
        }

        // we must have two spaces only, and they must be separated not concatenated
        if (line[i] == 0x20) {
            if (sp1 != -1 && sp2 != -1) {
                printf("%d\n", 400); // bad request
                return;
            } else if (sp1 != -1) {
                sp2 = i;
            } else if (i == 0) {
                printf("%d\n", 400); // leading space
                return;
            } else {
                sp1 = i;
            }

            if (sp1 != -1 && sp2 != -1 && sp2 - sp1 == 1) {
                printf("%d\n", 400); // bad request
                return;
            }
        }
    }

    // catching line with zero, one, and trailing spaces
    if (sp1 == -1 || sp2 == -1 || sp2 == (int)n - 1) {
        printf("%d\n", 400); // bad request
        return;
    }

    // extracts the line parts
    char method[16];
    char target[2048];
    char version[16];

    method[0] = '\0'; target[0] = '\0'; version[0] = '\0';

    size_t method_len = (size_t)sp1;
    size_t target_len = (size_t)(sp2 - sp1 - 1);
    size_t version_len = n - (size_t)sp2 - 1;

    if (method_len >= sizeof(method) || target_len >= sizeof(target) || version_len >= sizeof(version)) {
        printf("%d\n", 400);
        return;
    }

    getSubstring(method, line, 0, method_len);
    getSubstring(target, line, sp1 + 1, target_len);
    getSubstring(version, line, sp2 + 1, version_len);

    // method must be A–Z
    for (size_t i = 0; i < method_len; i++) {
        if (!(method[i] >= 'A' && method[i] <= 'Z')) {
            printf("%d\n", 400);
            return;
        }
    }

    // every byte of the target must be between 0x21 and 0x7e
    for (size_t i = 0; i < target_len; i++) {
        unsigned char c = (unsigned char)target[i];

        if (c < 0x21 || c > 0x7e) {
            printf("400\n");
            return;
        }
    }

    // HTTP/<digit>.<digit> -> at least 8 characters
    if (version_len != 8 ||
            strncmp(version, "HTTP/", 5) != 0 ||
            version[5] < '0' || version[5] > '9' ||
            version[6] != '.' ||
            version[7] < '0' || version[7] > '9') {

        printf("400\n");
        return;
    }

    // validate the target structure
    if (target[0] == '*') {
        if (target_len != 1) {
            printf("400\n");
            return;
        }
    } else if (target[0] != '/') {
        printf("400\n");
        return;
    }

    HttpMethod parsed_method = parse_method(method);

    // make sure that the used method is valid
    if (!parsed_method) {
        printf("501\n");
        return;
    }

    // enforce OPTIONS restriction
    if (target[0] == '*' && parsed_method != METHOD_OPTIONS) {
        printf("400\n");
        return;
    }


    // if version not HTTP/1.0 or HTTP/1.1; return 505
    if (strcmp(version, "HTTP/1.0") != 0 && strcmp(version, "HTTP/1.1") != 0) {
        printf("505\n");
        return;
    }
    
    printf("OK %.*s\n", (int)n, line);
}

int main(void) {
    /* Read every byte of stdin. fread on a binary-clean stream never rewrites \r\n. */
    size_t cap = 4096, len = 0, r;
    char *buf = malloc(cap);
    if (!buf) return 1;
    while ((r = fread(buf + len, 1, cap - len, stdin)) > 0) {
        len += r;
        if (len == cap) {
            cap *= 2;
            buf = realloc(buf, cap);
            if (!buf) return 1;
        }
    }
    /* Cut the stream at every CR LF pair. */
    size_t pos = 0, start = 0;
    while (pos < len) {
        if (pos + 1 < len && buf[pos] == '\r' && buf[pos + 1] == '\n') {
            judge(buf + start, pos - start);
            pos += 2;
            start = pos;
        } else {
            pos++;
        }
    }
    if (start < len) puts("400"); /* an unterminated fragment is a 400 */
    free(buf);
    return 0;
}