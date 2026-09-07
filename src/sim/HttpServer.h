#pragma once
// src/sim/HttpServer.h
//
// The smallest TCP/HTTP building blocks needed for --serve: listen,
// accept, read a request, write a response. Deliberately not a real
// HTTP library — this is a local dev tool talking to one browser tab,
// not a production server. Two implementations behind one interface:
//   - POSIX (Linux/macOS) sockets: verified in this sandbox.
//   - Windows (Winsock2): NOT compiled/verified here — no Windows
//     toolchain available in this sandbox — but it's the standard,
//     well-documented WSAStartup/socket/bind/listen/accept pattern.
//     Flag it if `pio run -e sim` fails to link on Windows; you may
//     need to add `-lws2_32` to the sim env's build_flags manually.

#include <string>
#include <cstdint>

namespace simhal {

struct HttpRequest {
  std::string method;
  std::string path;
  std::string body;
};

bool socketInit();      // WSAStartup on Windows; no-op on POSIX. false = failure.
void socketCleanup();   // WSACleanup on Windows; no-op on POSIX.

// Returns a listening socket bound to 127.0.0.1:port, or -1 on failure.
int listenOn(int port);

// Blocks until a client connects; returns its socket, or -1 on error.
int acceptClient(int listenFd);

void closeSocket(int fd);

// Reads one HTTP request (request line + headers + body, using
// Content-Length if present) off `fd`. Returns false if the
// connection closed or the request was malformed.
bool readHttpRequest(int fd, HttpRequest& out);

// Writes a complete HTTP response. `contentType` and `statusText` are
// plain C strings; `data`/`len` may be empty for a bodyless response.
void sendHttpResponse(int fd, int status, const char* statusText,
                       const char* contentType, const char* data, size_t len);

}  // namespace simhal
