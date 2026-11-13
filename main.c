#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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