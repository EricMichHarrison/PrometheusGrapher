# scripts/link_winsock.py
#
# PlatformIO extra_script for the `sim` and `cardputer`... actually
# just `sim`: the native platform links on whatever host OS you're
# building on. src/sim/HttpServer.cpp needs -lws2_32 on Windows (for
# WSAStartup/socket/etc.) but that flag doesn't exist on Linux/macOS,
# so it can't just live in platformio.ini's build_flags — it has to be
# conditional on the host OS. This script does that conditionally at
# build time via PlatformIO's SCons environment, rather than relying
# on the MSVC-only `#pragma comment(lib, ...)` trick, which MinGW's
# g++ silently ignores (that's the warning you saw).
#
# The library is appended through LIBS so PlatformIO emits it after the
# object files; MinGW's linker needs that order to resolve the symbols.

Import("env")
import sys

if sys.platform == "win32":
    env.Append(LIBS=["ws2_32"])
