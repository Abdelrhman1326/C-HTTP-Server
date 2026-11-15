# C HTTP Server

A small HTTP/1.1 server written in C. It accepts TCP connections, validates the
request line, and answers with the matching status code.

## Layout

```
include/
  http_methods.h   HTTP method enum and parse_method()
  request_line.h   request-line status codes and parser
src/
  http_methods.c   method lookup
  request_line.c   request-line validation
  main.c           TCP server: accept, read, validate, respond
Makefile
```

## Build and run

```sh
make          # builds build/server
make run      # starts the server on port 8080
make clean    # removes build/
```

Then point a client at it:

```sh
curl -i http://localhost:8080/
```

## Request line rules

The server reads the first line of a request and answers with:

| Status | When |
| --- | --- |
| `200` | valid `METHOD target HTTP/1.x` request |
| `400` | malformed line, target, or `*` used with a method other than `OPTIONS` |
| `501` | method not in `GET HEAD POST PUT DELETE OPTIONS PATCH` |
| `505` | version other than `HTTP/1.0` / `HTTP/1.1` |

A connection is closed after one response (`Connection: close`).

## Next step: request headers (TODO)

Implement the header-block counterpart to `parse_request_line()`: a
`parse_request_headers()` that validates every byte between the request line's
CRLF and the blank line, and stores the valid fields as slices into the read
buffer.

```c
typedef enum {
    REQUEST_HEADERS_OK = 200,
    REQUEST_HEADERS_BAD_REQUEST = 400,
    REQUEST_HEADERS_FIELDS_TOO_LARGE = 431,
    REQUEST_HEADERS_NOT_IMPLEMENTED = 501 /* e.g. Transfer-Encoding, for now */
} RequestHeadersStatus;

typedef struct {
    const char *name;
    size_t name_len;
    const char *value;
    size_t value_len;
} HttpHeader;

/* parse_request_headers(block, len, headers) -> status */
```

| Status | When |
| --- | --- |
| `200` | every header line is valid |
| `400` | malformed line, bad field name, bare CR/LF, duplicate/invalid `Host`, conflicting `Content-Length` |
| `431` | too many header fields or header block too large |
| `501` | `Transfer-Encoding` used (until chunked decoding exists) |

### Todo

- [ ] Add `include/http_headers.h` and `src/http_headers.c` with the API above.
- [ ] Split the block on CRLF; a bare CR or LF inside a line is `400`.
- [ ] Reject obsolete line folding (a continuation line starting with SP/HTAB) with `400`.
- [ ] Require `name ":" value`; no whitespace before the colon; empty name is `400`.
- [ ] Validate field-name bytes against RFC 7230 `tchar`; reject everything else.
- [ ] Validate values: allow HTAB and obs-text (`0x80`-`0xFF`), reject other control bytes.
- [ ] Trim optional whitespace (SP/HTAB) around the value.
- [ ] Store names and values as slices into the connection buffer; no per-header copies.
- [ ] Add a case-insensitive `find_header()` helper.
- [ ] Require exactly one `Host` for HTTP/1.1 and reject duplicates.
- [ ] Validate `Content-Length` as digits only, reject overflow, and require duplicate values to match.
- [ ] Reject `Content-Length` together with `Transfer-Encoding`; return `501` for `Transfer-Encoding` alone for now.
- [ ] Cap the header count and block size, returning `431`.
- [ ] Parse `Connection` tokens case-insensitively (`close`, `keep-alive`).
- [ ] In `main.c`, read through the blank line before responding; a request that ends before the blank line is `400`.
- [ ] Respond with the combined result: the first failure wins (request line, then headers).

### Tests to add

- [ ] Valid request with `Host`, `User-Agent`, and an empty-value header.
- [ ] Missing colon, whitespace before colon, empty name.
- [ ] Leading-space (folded) continuation line.
- [ ] Bare CR or LF inside the header block.
- [ ] Duplicate `Host`; duplicate matching `Content-Length`; duplicate conflicting `Content-Length`.
- [ ] `Content-Length` with a sign, non-digit, or overflow.
- [ ] `Transfer-Encoding: chunked` -> `501`.
- [ ] Header split across two `read()` calls.
- [ ] Oversized header block -> `431`.

## Roadmap

- **Header parsing (next)** — field syntax, `Host`, `Content-Length`, `Connection`, `431` limits
- **Request body parsing** — `Content-Length`, chunked `Transfer-Encoding`, `100-continue`, `413` limits
- **Response building** — status line, `Date`/`Server`/`Content-Length`, `HEAD` responses, consistent error responses
- **Keep-alive connections** — HTTP/1.0 vs 1.1 defaults, `Connection` semantics, request pipelining
- **URI parsing** — path/query split, percent-decoding, normalization, `414` limits
- **Routing and static files** — method+path router, `404`/`405` with `Allow`, MIME types, index files, path-traversal protection, `ETag`/`Last-Modified`/`304`, `Range`
- **Connection hardening** — read/write timeouts, slowloris protection, connection and size caps, graceful shutdown, `SIGPIPE` handling
- **Concurrency** — thread pool or event loop (`kqueue`/`epoll`), backpressure, load testing
- **Configuration and observability** — CLI/config for port and document root, structured request logging, metrics/health endpoint
- **Testing and CI** — parser unit tests, socket-level integration tests, fuzzing, ASan/UBSan, benchmarks, CI matrix
- **WebSocket from scratch (final)** — own HTTP Upgrade handshake, `Sec-WebSocket-Accept` with hand-written SHA-1 + Base64, frame parser/builder, masking, opcodes, fragmentation, ping/pong and close handshake, no third-party WebSocket library
- **Stretch goals** — reverse proxy/CGI, TLS via reverse proxy or OpenSSL, HTTP/2
