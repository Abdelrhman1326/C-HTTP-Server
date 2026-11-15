# C HTTP Server

A small HTTP/1.1 server written from scratch in C, with no third-party
libraries. It accepts TCP connections, validates request lines, and responds
with the matching status code. Build with `make`, run with `make run`
(port 8080).

## Current stage: request headers

Request-line validation is done. The next step is `parse_request_headers()`:
validate the header block between the request line and the blank line, and
store each field as a slice into the read buffer. Returns `200`, `400`, `431`,
or `501`.

- [ ] Add `include/http_headers.h` and `src/http_headers.c`.
- [ ] Split the block on CRLF; a bare CR or LF inside a line is `400`.
- [ ] Reject obsolete line folding (continuation lines starting with SP/HTAB).
- [ ] Require `name ":" value`; no whitespace before the colon, no empty name.
- [ ] Validate field names against RFC 7230 `tchar` and values against control-byte rules.
- [ ] Trim optional whitespace around values.
- [ ] Store names/values as buffer slices and add a case-insensitive `find_header()`.
- [ ] Require exactly one `Host` for HTTP/1.1.
- [ ] Validate `Content-Length`; reject it with `Transfer-Encoding`, and return `501` for `Transfer-Encoding` alone.
- [ ] Cap header count and block size -> `431`.
- [ ] Parse `Connection` tokens case-insensitively.
- [ ] In `main.c`, read through the blank line; the first failure wins.
- [ ] Tests: malformed lines and folding, duplicate `Host`/`Content-Length`, bad lengths, split reads, oversized block.

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
