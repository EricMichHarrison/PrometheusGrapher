#include "HttpServer.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <sstream>

// ===========================================================================
// Windows (Winsock2) implementation — NOT compiled or run in this sandbox
// (no Windows toolchain available here). Standard WSAStartup/socket/bind/
// listen/accept pattern. If linking fails, add -lws2_32 to the sim env's
// build_flags in platformio.ini.
// ===========================================================================
#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
// NOTE: no #pragma comment(lib, "ws2_32.lib") here — that's an
// MSVC-only trick; MinGW's g++ (what PlatformIO's native platform
// uses on Windows) just warns and ignores it. The actual link flag is
// added portably by scripts/link_winsock.py via platformio.ini's
// extra_scripts, so Linux/macOS builds aren't affected.

namespace simhal {

bool socketInit() {
  WSADATA wsa;
  return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
}

void socketCleanup() { WSACleanup(); }

int listenOn(int port) {
  SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s == INVALID_SOCKET) return -1;

  BOOL yes = TRUE;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(uint16_t(port));

  if (bind(s, (sockaddr*)&addr, sizeof(addr)) != 0) { closesocket(s); return -1; }
  if (listen(s, 8) != 0) { closesocket(s); return -1; }
  return int(s);
}

int acceptClient(int listenFd) {
  SOCKET c = accept(SOCKET(listenFd), nullptr, nullptr);
  return c == INVALID_SOCKET ? -1 : int(c);
}

void closeSocket(int fd) { closesocket(SOCKET(fd)); }

namespace {
int recvBytes(int fd, char* buf, int len) {
  return recv(SOCKET(fd), buf, len, 0);
}
int sendBytes(int fd, const char* buf, int len) {
  return send(SOCKET(fd), buf, len, 0);
}
}  // namespace

}  // namespace simhal

#else
// ===========================================================================
// POSIX implementation — compiled and exercised (curl-tested) in this
// sandbox.
// ===========================================================================

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace simhal {

bool socketInit() { return true; }
void socketCleanup() {}

int listenOn(int port) {
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) return -1;

  int yes = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(uint16_t(port));

  if (bind(s, (sockaddr*)&addr, sizeof(addr)) != 0) { close(s); return -1; }
  if (listen(s, 8) != 0) { close(s); return -1; }
  return s;
}

int acceptClient(int listenFd) {
  return accept(listenFd, nullptr, nullptr);
}

void closeSocket(int fd) { close(fd); }

namespace {
int recvBytes(int fd, char* buf, int len) { return int(recv(fd, buf, size_t(len), 0)); }
int sendBytes(int fd, const char* buf, int len) { return int(send(fd, buf, size_t(len), 0)); }
}  // namespace

}  // namespace simhal

#endif

// ===========================================================================
// Shared, platform-independent request/response parsing — built on the
// recvBytes/sendBytes primitives defined above for each platform.
// ===========================================================================
namespace simhal {

bool readHttpRequest(int fd, HttpRequest& out) {
  std::string buf;
  char chunk[4096];
  size_t headerEnd = std::string::npos;

  while (headerEnd == std::string::npos) {
    int n = recvBytes(fd, chunk, sizeof(chunk));
    if (n <= 0) return false;
    buf.append(chunk, size_t(n));
    headerEnd = buf.find("\r\n\r\n");
    if (buf.size() > 65536) return false;  // guard against runaway headers
  }

  std::string headerPart = buf.substr(0, headerEnd);
  std::string bodyPart = buf.substr(headerEnd + 4);

  size_t lineEnd = headerPart.find("\r\n");
  std::string requestLine = headerPart.substr(0, lineEnd);
  std::istringstream iss(requestLine);
  iss >> out.method >> out.path;

  size_t contentLength = 0;
  {
    std::string lower = headerPart;
    for (auto& c : lower) c = char(std::tolower((unsigned char)c));
    size_t pos = lower.find("content-length:");
    if (pos != std::string::npos) {
      contentLength = size_t(std::atoi(headerPart.c_str() + pos +
                                        std::strlen("content-length:")));
    }
  }

  while (bodyPart.size() < contentLength) {
    int n = recvBytes(fd, chunk, sizeof(chunk));
    if (n <= 0) break;
    bodyPart.append(chunk, size_t(n));
  }
  out.body = bodyPart.substr(0, std::min(bodyPart.size(), contentLength));
  return !out.method.empty();
}

void sendHttpResponse(int fd, int status, const char* statusText,
                       const char* contentType, const char* data, size_t len) {
  std::ostringstream headers;
  headers << "HTTP/1.1 " << status << " " << statusText << "\r\n"
          << "Content-Type: " << contentType << "\r\n"
          << "Content-Length: " << len << "\r\n"
          << "Cache-Control: no-store\r\n"
          << "Connection: close\r\n"
          << "Access-Control-Allow-Origin: *\r\n"
          << "\r\n";
  std::string h = headers.str();
  sendBytes(fd, h.data(), int(h.size()));
  if (len > 0) sendBytes(fd, data, int(len));
}

}  // namespace simhal
