#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "http_headers.h"
#include "request_line.h"

#define PORT 8080
#define BUFFER_SIZE (HTTP_MAX_HEADER_BLOCK + 1024)

static const char *reason_phrase(RequestLineStatus status) {
    switch (status) {
        case REQUEST_LINE_OK: return "OK";
        case REQUEST_LINE_BAD_REQUEST: return "Bad Request";
        case REQUEST_LINE_NOT_IMPLEMENTED: return "Not Implemented";
        case REQUEST_LINE_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";
    }
    return "Bad Request";
}

static void send_all(int fd, const char *buf, size_t len) {
    while (len > 0) {
        ssize_t written = write(fd, buf, len);
        if (written <= 0) return;
        buf += written;
        len -= (size_t)written;
    }
}

static void send_response(int fd, RequestLineStatus status) {
    const char *body = status == REQUEST_LINE_OK ? "Hello from my server!\n" : "";
    char response[512];

    int len = snprintf(response, sizeof response,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        (int)status, reason_phrase(status), strlen(body), body);

    if (len > 0) {
        send_all(fd, response, (size_t)len < sizeof response ? (size_t)len : sizeof response - 1);
    }
}

static RequestLineStatus read_request_line(int fd, char *buf, size_t cap) {
    ssize_t n = read(fd, buf, cap - 1);
    if (n <= 0) return REQUEST_LINE_BAD_REQUEST;

    buf[n] = '\0';

    char *line_end = memchr(buf, '\r', (size_t)n);
    if (line_end == NULL || (size_t)(line_end - buf) + 1 >= (size_t)n || line_end[1] != '\n') {
        return REQUEST_LINE_BAD_REQUEST;
    }

    return parse_request_line(buf, (size_t)(line_end - buf));
}

int main(void) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    int yes = 1; // lets you restart the server immediately
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, 16) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    printf("listening on port %d\n", PORT);

    for (;;) {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd < 0) { perror("accept"); continue; }

        char buf[BUFFER_SIZE];
        RequestLineStatus status = read_request_line(client_fd, buf, sizeof buf);
        send_response(client_fd, status);

        close(client_fd);
    }
}
