// src/sim/main.cpp
//
// Native simulator for PrometheusGrapher. Same core:: and ui:: code
// that would run on the Cardputer runs here.
//
// Usage:
//   program --shot    <dir>       one frame -> <dir>/shot.ppm
//   program --frames  <dir> <n>   a scripted panning sequence, n frames
//   program --screens <dir>       a handful of named states
//   program --live                interactive terminal view (arrows pan,
//                                  1-4 graph a slot, 'c' centers, Esc quits)
//   program --serve <port>        live view in a browser tab, no shell UI

#include "Framebuffer.h"
#include "GrapherState.h"
#include "GraphRenderer.h"
#include "PpmWriter.h"
#include "TerminalView.h"
#include "InputTerminal.h"
#include "HttpServer.h"
#include "WebPage.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <chrono>
#include <thread>

namespace {

constexpr int kWidth = 240;
constexpr int kHeight = 135;

void printUsage() {
  std::printf(
      "usage:\n"
      "  program --shot    <dir>       single frame -> <dir>/shot.ppm\n"
      "  program --frames  <dir> <n>   n-frame panning sequence\n"
      "  program --screens <dir>       named demo screens\n"
      "  program --live                interactive terminal view\n"
      "                                 (arrows pan, 1-4 graph a slot, "
      "'c' centers, Esc quits)\n"
      "  program --serve <port>        live view in a browser tab, no "
      "shell UI\n"
      "                                 (open http://localhost:<port>)\n");
}

std::string joinPath(const std::string& dir, const std::string& file) {
  if (!dir.empty() && dir.back() == '/') return dir + file;
  return dir + "/" + file;
}

core::GrapherState makeDemoState() {
  core::GrapherState state;
  state.setEquationSlot(0, "sin(x/10)*30");
  state.graphEquation(0);
  return state;
}

int cmdShot(const std::string& dir) {
  core::GrapherState state = makeDemoState();
  core::Framebuffer fb(kWidth, kHeight);
  ui::renderGraph(state, fb);

  std::string path = joinPath(dir, "shot.ppm");
  if (!sim::writePpm(fb, path)) {
    std::fprintf(stderr, "failed to write %s\n", path.c_str());
    return 1;
  }
  std::printf("wrote %s (checksum=%u)\n", path.c_str(), fb.checksum());
  return 0;
}

int cmdFrames(const std::string& dir, int n) {
  core::GrapherState state = makeDemoState();
  for (int i = 0; i < n; ++i) {
    state.panBy(3, 0);  // scripted pan, deterministic frame to frame
    core::Framebuffer fb(kWidth, kHeight);
    ui::renderGraph(state, fb);

    char name[64];
    std::snprintf(name, sizeof(name), "frame_%03d.ppm", i);
    std::string path = joinPath(dir, name);
    if (!sim::writePpm(fb, path)) {
      std::fprintf(stderr, "failed to write %s\n", path.c_str());
      return 1;
    }
  }
  std::printf("wrote %d frames to %s\n", n, dir.c_str());
  return 0;
}

int cmdScreens(const std::string& dir) {
  struct Screen {
    const char* name;
    const char* equation;  // nullptr = leave nothing graphed
    int panX, panY;
  };
  const Screen screens[] = {
      {"empty_grid", nullptr, 0, 0},
      {"linear", "x", 0, 0},
      {"parabola", "x^2/10", 0, 0},
      {"sine", "sin(x/10)*30", 0, 0},
      {"sine_panned", "sin(x/10)*30", 40, -20},
      {"domain_error_gap", "sqrt(x)", 0, 0},  // NAN for x<0: should show a gap
  };

  for (const auto& s : screens) {
    core::GrapherState state;
    if (s.equation) {
      state.setEquationSlot(0, s.equation);
      state.graphEquation(0);
    }
    state.panBy(s.panX, s.panY);

    core::Framebuffer fb(kWidth, kHeight);
    ui::renderGraph(state, fb);

    std::string path = joinPath(dir, std::string(s.name) + ".ppm");
    if (!sim::writePpm(fb, path)) {
      std::fprintf(stderr, "failed to write %s\n", path.c_str());
      return 1;
    }
    std::printf("wrote %s (checksum=%u)\n", path.c_str(), fb.checksum());
  }
  return 0;
}

int cmdLive() {
  simhal::ansiClearScreen();
  simhal::ansiHideCursor();

  core::GrapherState state = makeDemoState();
  simhal::TerminalKeyboard keyboard;
  bool running = true;

  while (running) {
    hal::KeyPress kp = keyboard.poll();
    switch (kp.event) {
      case hal::KeyEvent::kLeft: state.panBy(-3, 0); break;
      case hal::KeyEvent::kRight: state.panBy(3, 0); break;
      case hal::KeyEvent::kUp: state.panBy(0, 3); break;
      case hal::KeyEvent::kDown: state.panBy(0, -3); break;
      case hal::KeyEvent::kChar:
        if (kp.ch == 'c') state.resetPan();
        if (kp.ch >= '1' && kp.ch <= '4') state.graphEquation(kp.ch - '1');
        break;
      case hal::KeyEvent::kQuit: running = false; break;
      default: break;
    }

    core::Framebuffer fb(kWidth, kHeight);
    ui::renderGraph(state, fb);
    simhal::printFramebufferAnsi(fb);

    std::this_thread::sleep_for(std::chrono::milliseconds(33));
  }

  simhal::ansiShowCursor();
  std::printf("\nbye\n");
  return 0;
}

void applyKeyCommand(core::GrapherState& state, const std::string& body) {
  if (body == "left") { state.panBy(-3, 0); return; }
  if (body == "right") { state.panBy(3, 0); return; }
  if (body == "up") { state.panBy(0, 3); return; }
  if (body == "down") { state.panBy(0, -3); return; }
  if (body == "center") { state.resetPan(); return; }
  if (body.rfind("graph:", 0) == 0) {
    int slot = std::atoi(body.c_str() + 6);
    state.graphEquation(slot);
  }
}

void applyEquationCommand(core::GrapherState& state, const std::string& body) {
  // format: "<slotIndex>:<equation text>"
  size_t colon = body.find(':');
  if (colon == std::string::npos) return;
  int slot = std::atoi(body.substr(0, colon).c_str());
  std::string text = body.substr(colon + 1);
  state.setEquationSlot(slot, text);
  state.graphEquation(slot);
}

int cmdServe(int port) {
  if (!simhal::socketInit()) {
    std::fprintf(stderr, "socket init failed\n");
    return 1;
  }
  int listenFd = simhal::listenOn(port);
  if (listenFd < 0) {
    std::fprintf(stderr,
                  "failed to listen on 127.0.0.1:%d (port busy or blocked?)\n",
                  port);
    simhal::socketCleanup();
    return 1;
  }

  std::printf("Serving at http://localhost:%d  (Ctrl+C to stop)\n", port);
  core::GrapherState state = makeDemoState();

  while (true) {
    int client = simhal::acceptClient(listenFd);
    if (client < 0) continue;

    simhal::HttpRequest req;
    if (simhal::readHttpRequest(client, req)) {
      if (req.method == "GET" && (req.path == "/" || req.path == "/index.html")) {
        simhal::sendHttpResponse(client, 200, "OK", "text/html; charset=utf-8",
                                  simhal::kIndexHtml, std::strlen(simhal::kIndexHtml));
      } else if (req.method == "GET" && req.path == "/frame.bin") {
        core::Framebuffer fb(kWidth, kHeight);
        ui::renderGraph(state, fb);

        std::string payload;
        payload.resize(4 + fb.rawSizeBytes());
        auto w = uint16_t(fb.width()), h = uint16_t(fb.height());
        payload[0] = char(w & 0xFF); payload[1] = char((w >> 8) & 0xFF);
        payload[2] = char(h & 0xFF); payload[3] = char((h >> 8) & 0xFF);
        std::memcpy(&payload[4], fb.raw(), fb.rawSizeBytes());
        simhal::sendHttpResponse(client, 200, "OK", "application/octet-stream",
                                  payload.data(), payload.size());
      } else if (req.method == "POST" && req.path == "/key") {
        applyKeyCommand(state, req.body);
        simhal::sendHttpResponse(client, 200, "OK", "text/plain", "ok", 2);
      } else if (req.method == "POST" && req.path == "/equation") {
        applyEquationCommand(state, req.body);
        simhal::sendHttpResponse(client, 200, "OK", "text/plain", "ok", 2);
      } else {
        simhal::sendHttpResponse(client, 404, "Not Found", "text/plain",
                                  "not found", 9);
      }
    }
    simhal::closeSocket(client);
  }

  simhal::closeSocket(listenFd);
  simhal::socketCleanup();
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc >= 3 && std::strcmp(argv[1], "--shot") == 0) return cmdShot(argv[2]);
  if (argc >= 4 && std::strcmp(argv[1], "--frames") == 0)
    return cmdFrames(argv[2], std::atoi(argv[3]));
  if (argc >= 3 && std::strcmp(argv[1], "--screens") == 0) return cmdScreens(argv[2]);
  if (argc >= 2 && std::strcmp(argv[1], "--live") == 0) return cmdLive();
  if (argc >= 3 && std::strcmp(argv[1], "--serve") == 0) return cmdServe(std::atoi(argv[2]));
  printUsage();
  return 0;
}
