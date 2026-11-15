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

## Roadmap

- Header and body parsing
- Keep-alive connections
- Routing and static files
- Request logging
