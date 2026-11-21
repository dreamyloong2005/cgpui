"""Build and run the packaged sample outside the checkout, from the final archive."""

import argparse
import json
import os
from pathlib import Path
import subprocess
import tarfile
import tempfile


def output(*args):
    return subprocess.check_output(args, text=True).strip()


def check_binary(path, arch):
    if output("xcrun", "lipo", "-archs", path) != arch:
        raise SystemExit(f"{path}: expected {arch}")
    build = output("xcrun", "vtool", "-show-build", path).splitlines()
    if "minos 13.0" not in [line.strip() for line in build]:
        raise SystemExit(f"{path}: expected macOS deployment target 13.0")
    deps = output("xcrun", "otool", "-L", path).splitlines()[1:]
    for dep in deps:
        if not dep.strip().startswith(("/usr/lib/", "/System/Library/")):
            raise SystemExit(f"{path}: non-system runtime dependency: {dep}")
    print(f"{path.name}: {arch}, minos 13.0, system runtime dependencies only", flush=True)


def verify(archive, arch, work):
    sdk = work / f"cgpui-macos-{arch}-release"
    with tarfile.open(archive, "r:gz") as tar:
        tar.extractall(work, filter="data")
    manifest = json.loads((sdk / "manifest.json").read_text())
    # Check the Phase J archive contract against real files, not the source
    # text of the packager; permit future additions to the graph.
    required = {"cgpui_core", "cgpui_platform", "cgpui_platform_macos",
                "cgpui_renderer", "cgpui_renderer_metal", "cgpui_ui", "cgpui_app",
                "lunasvg", "plutovg"}
    missing = {f"lib/lib{name}.a" for name in required} - set(manifest["libraries"])
    if missing:
        raise SystemExit(f"archives missing from SDK manifest: {sorted(missing)}")
    for library in manifest["libraries"]:
        if output("xcrun", "lipo", "-archs", sdk / library) != arch:
            raise SystemExit(f"{library}: expected {arch}")
    check_binary(sdk / "bin/hello_window", arch)
    config = sdk / manifest["cmake_config"]
    if not config.is_file():
        raise SystemExit(f"packaged CMake config is missing: {config}")
    env = os.environ.copy()
    for key in ("CPATH", "CPLUS_INCLUDE_PATH", "LIBRARY_PATH", "CXXFLAGS",
                "CPPFLAGS", "LDFLAGS", "CMAKE_PREFIX_PATH", "CMAKE_TOOLCHAIN_FILE"):
        env.pop(key, None)
    build = work / "build"
    subprocess.run([
        "cmake", "-S", sdk / manifest["consumer_sample"], "-B", build,
        "-G", "Unix Makefiles", "-DCMAKE_BUILD_TYPE=Release",
        f"-DCMAKE_OSX_ARCHITECTURES={arch}", "-DCMAKE_OSX_DEPLOYMENT_TARGET=13.0",
        f"-DCMAKE_CXX_COMPILER={output('xcrun', '--find', 'clang++')}",
        f"-Dcgpui_DIR={config.parent}",
    ], cwd=work, env=env, check=True)
    subprocess.run(["cmake", "--build", build, "--verbose"], cwd=work, env=env, check=True)
    binary = build / "sdk_consumer"
    check_binary(binary, arch)
    smoke = subprocess.run([binary], cwd=work, text=True, stdout=subprocess.PIPE,
                           env=dict(env, CGPUI_EXIT_AFTER_FIRST_FRAME="1"), timeout=60,
                           check=True)
    print(smoke.stdout, end="")
    if "sdk_consumer: first frame presented" not in smoke.stdout.splitlines():
        raise SystemExit("consumer exited without presenting a frame")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("arch", choices=("arm64",))
    args = parser.parse_args()
    # The macOS CI bootstrap sets TMPDIR *inside* the checkout.
    with tempfile.TemporaryDirectory(prefix="cgpui-sdk-", dir="/private/tmp") as tmp:
        verify(args.archive, args.arch, Path(tmp))
