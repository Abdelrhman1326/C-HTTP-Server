# C HTTP Server

A small HTTP/1.1 server written from scratch in C, with no third-party
libraries. It accepts TCP connections, validates request lines, and responds
with the matching status code. Build with `make`, run with `make run`
(port 8080).

## Current stage: request headers

Request-line validation is done. The remaining work is
`parse_request_headers()` in `include/http_headers.h` / `src/http_headers.c`:
validate the header block between the request line and the blank line, and
store each field as a slice into the read buffer.

### Contract

    HeadersStatus parse_request_headers(const char *block, size_t n,
                                        RequestHeaders *out);

- `block` — first byte **after** the request line's CRLF. The request line is
  not part of the input.
- `n` — length of the header section **including** the terminating blank
  line's CRLF. A request with no headers is `block == "\r\n"`, `n == 2`.
- `block + n` — first byte of the body, or the end of the buffer.
- `out` — caller-owned struct, zeroed on entry. On any non-`200` return its
  contents are unspecified and must not be read.
- Input is **not** NUL-terminated and may contain any byte. Use `memchr`,
  `memcmp`, and index arithmetic only — no `strchr`, `strstr`, `strtok`,
  `strlen`, or `sscanf`.
- Fields are slices into `block`, so the caller's buffer must outlive `out`.
  No copies, no allocation, no modification of the input.
- Pure function: no printing, no logging, no socket writes.
- The block is always complete. `main.c` reads until it sees `\r\n\r\n` (or
  hits the cap) and only then calls this function; there is no `INCOMPLETE`
  status.

### Status codes

| Status | Condition |
| --- | --- |
| `200` | Header block is well formed |
| `400` | Bare CR or LF, obs-fold, bad field syntax, bad name byte, bad value byte, duplicate `Host`, malformed or conflicting `Content-Length`, `Content-Length` together with `Transfer-Encoding` |
| `431` | More than `HTTP_MAX_HEADERS` fields, or `n > HTTP_MAX_HEADER_BLOCK` |
| `501` | `Transfer-Encoding` present without `Content-Length` |

Add `431 -> "Request Header Fields Too Large"` to `reason_phrase()` in
`main.c`.

### Checklist

- [x] Add `include/http_headers.h` and `src/http_headers.c`.
- [x] Block must end with the blank-line CRLF (`n == 2`, or bytes `n-4..n-3`
      are CRLF and `n-2..n-1` are CRLF); otherwise `400`. Reject blocks over
      `HTTP_MAX_HEADER_BLOCK` with `431`.
- [x] Split on CRLF. A CR or LF that is not part of a CRLF pair inside a line
      is `400`.
- [x] Reject obsolete line folding: any line beginning with SP or HTAB is
      `400`.
- [x] Require `name ":" value`. Empty name, missing colon, and any SP or HTAB
      immediately before the colon are all `400`.
- [ ] Field name: at least one byte, every byte an RFC 7230 `tchar`
      (``! # $ % & ' * + - . ^ _ ` | ~``, DIGIT, ALPHA), else `400`.
- [ ] Field value: HTAB, SP, `0x21..0x7E`, and `0x80..0xFF` are allowed; every
      other control byte, including DEL, is `400`.
- [x] Trim SP and HTAB from both ends of the value. An empty value is legal.
- [x] Store names and values as `(pointer, length)` slices; keep the original
      name casing. Add `find_header()` doing ASCII-case-insensitive lookup.
- [ ] Require exactly one `Host` for HTTP/1.1; a duplicate or missing `Host`
      is `400`.
- [ ] `Content-Length`: one or more ASCII digits, no sign, no whitespace, no
      overflow of `size_t`, else `400`. Duplicates with differing values are
      `400`. Reject it together with `Transfer-Encoding` (`400`).
- [ ] `Transfer-Encoding` alone -> `501`.
- [ ] Cap header count at `HTTP_MAX_HEADERS`; exceeding it -> `431`.
- [ ] Parse `Connection` tokens case-insensitively, comma-separated with
      optional whitespace, and set `connection_close` / `connection_keep_alive`.
- [ ] In `main.c`, read through the blank line before parsing; the first
      failure wins, in request-line-then-headers order.
- [ ] Tests: malformed lines and folding, duplicate `Host` / `Content-Length`,
      bad lengths, split reads, oversized block.

## Roadmap

- **Header parsing (current)** — field syntax, `Host`, `Content-Length`, `Connection`, `431` limits
- **Request body parsing** — `Content-Length`, chunked encoding, `100-continue`, `413` limits
- **Response building** — status line, `Date`/`Server`/`Content-Length`, `HEAD`, consistent errors
- **Keep-alive** — HTTP/1.0/1.1 defaults, `Connection`, pipelining
- **URI parsing** — path/query split, percent-decoding, normalization
- **Routing and static files** — router, `404`/`405`, MIME types, index files, traversal protection, `ETag`/`304`, `Range`
- **Connection hardening** — timeouts, slowloris protection, caps, graceful shutdown, `SIGPIPE`
- **Concurrency** — thread pool or event loop (`kqueue`/`epoll`), backpressure, load testing
- **Config and observability** — CLI/config, structured logging, metrics/health
- **Testing and CI** — unit and integration tests, fuzzing, ASan/UBSan, benchmarks, CI
- **WebSocket from scratch (final)** — hand-written Upgrade handshake, SHA-1 + Base64, framing, masking, ping/pong, close; no WebSocket library
- **Stretch goals** — reverse proxy/CGI, TLS, HTTP/2
