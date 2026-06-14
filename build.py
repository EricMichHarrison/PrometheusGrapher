
# Removes Warmning when editieng leaving runtime unaffected.
from typing import Any
try:
    # Provided by PlatformIO/SCons at build time; define for linters.
    from SCons.Script import Import  # type: ignore
except Exception:
    def Import(*args: Any, **kwargs: Any) -> None:  # type: ignore
        pass

env: Any  # injected by PlatformIO via Import("env") at runtime

# end
Import("env")
import shutil
import os

def post_build_copy(source, target, env):
    print("Copying ../firmware.bin -> root/Prometheus_Grapher.bin ...")
    build_dir = env.subst("$BUILD_DIR")
    firmware = os.path.join(build_dir, "firmware.bin")
    output = os.path.join(env.subst("$PROJECT_DIR"), "Prometheus_Grapher.bin")
    
    if os.path.exists(firmware):
        shutil.copy(firmware, output)
        print("Copied Succesfully")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", post_build_copy)
