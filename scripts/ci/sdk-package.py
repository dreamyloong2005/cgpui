#!/usr/bin/env python3
"""Shared steps of the CGPUI SDK packages (docs/sdk-packaging.md).

check-clean  refuses a checkout with tracked changes (and, unless
             --allow-untracked, untracked files that are not ignored).
finalize     turns a windows-package.ps1 / linux-package.sh package into an
             SDK: bundled third-party static libraries, link order taken
             from the real example_workbench link line, manifest.json with
             toolchain, dependency versions and a SHA-256 for every file,
             a CMake config package, the consumer sample and README.md.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import subprocess
from pathlib import Path

LOCK_FILE = "xmake-requires.lock"
LINUX_SYSTEM_PACKAGES = (
    "harfbuzz", "freetype2", "fontconfig", "wayland", "libxkbcommon",
    "dbus", "vulkan-icd-loader", "libxml2",
)


def git(repo: Path, *args: str) -> str:
    return subprocess.run(
        ["git", "-C", str(repo), *args], check=True, capture_output=True,
        text=True).stdout


def status_lines(repo: Path) -> list[str]:
    output = git(repo, "status", "--porcelain", "--untracked-files=all")
    return [line for line in output.splitlines() if line.strip()]


def check_clean(repo: Path, allow_untracked: bool, allow_lock: bool) -> None:
    problems = []
    for line in status_lines(repo):
        path = line[3:].strip('"')
        if line.startswith("??"):
            if not allow_untracked:
                problems.append(f"untracked: {path}")
        elif not (allow_lock and path == LOCK_FILE):
            problems.append(f"changed: {path}")
    if problems:
        raise SystemExit(
            f"refusing to package {repo}: the tree is not clean\n  "
            + "\n  ".join(problems))


def link_line(log: Path) -> str:
    raw = log.read_bytes()
    # PowerShell redirection may write UTF-16 with a byte order mark.
    utf16 = raw[:2] in (b"\xff\xfe", b"\xfe\xff")
    text = raw.decode("utf-16" if utf16 else "utf-8", "replace")
    text = re.sub(r"\x1b\[[0-9;]*m", "", text)
    for line in text.splitlines():
        # The link step, not a compile step (which writes main.cpp.o).
        if re.search(r"-out:\S*example_workbench\.exe(\s|$)", line) or \
                re.search(r"\s-o\s+\S*example_workbench(\s|$)", line):
            return line.strip()
    raise SystemExit(f"no example_workbench link line in {log}")


def package_origin(directory: str) -> str | None:
    """`name version` for a lib directory in an xmake package store
    (<store>/<letter>/<name>/<version>/<hash>/lib)."""
    parts = re.split(r"[\\/]+", directory)
    for index in range(len(parts) - 5, -1, -1):
        store = parts[index] in ("packages", "pkg-install")
        if store and parts[index + 5] == "lib":
            return f"{parts[index + 2]} {parts[index + 3]}"
    return None


def parse_link_line(line: str, platform: str, binary_root: Path) -> dict:
    """Classifies the libraries of the link line, in link order."""
    if platform == "windows":
        dirs = re.findall(r"-libpath:(\S+)", line)
        names = [token[:-4] for token in line.split()
                 if re.fullmatch(r"[\w.+-]+\.lib", token)]
        file_name = "{}.lib"
    else:
        dirs = re.findall(r"(?:^|\s)-L(\S+)", line)
        names = re.findall(r"(?:^|\s)-l(\S+)", line)
        file_name = "lib{}.a"
    libraries, system = [], []
    for name in dict.fromkeys(names):
        candidate = binary_root / file_name.format(name)
        if candidate.is_file():
            libraries.append({"name": name, "source": candidate,
                              "origin": "cgpui"})
            continue
        bundled = next(
            (Path(d) / file_name.format(name) for d in dirs
             if package_origin(d) and (Path(d) / file_name.format(name)).is_file()),
            None)
        if bundled is not None:
            libraries.append({"name": name, "source": bundled,
                              "origin": f"xmake package {package_origin(str(bundled.parent))}"})
        else:
            system.append(name)
    return {"libraries": libraries, "system": system, "dirs": dirs}


def run_text(command: list[str]) -> str:
    result = subprocess.run(command, capture_output=True, text=True,
                            errors="replace")
    return (result.stdout + result.stderr).strip()


def linker_path(line: str) -> str:
    match = re.match(r'"([^"]+)"|(\S+)', line)
    return (match.group(1) or match.group(2)).replace("\\\\", "\\")


def compiler_info(line: str, platform: str) -> dict:
    linker = linker_path(line)
    if platform == "windows":
        cl = str(Path(linker).with_name("cl.exe"))
        # The banner may be localized; the version follows "C/C++".
        banner = next((line for line in run_text([cl]).splitlines()
                       if "C/C++" in line), "unknown")
        version = re.search(r"C/C\+\+\D*(\d+\.\d+\.\d+)", banner)
        return {"id": "msvc", "path": cl,
                "version": version.group(1) if version else "unknown",
                "toolset": Path(cl).parents[3].name}
    banner = run_text([linker, "--version"]).splitlines()[0]
    version = re.search(r"(\d+\.\d+\.\d+)", banner)
    return {"id": "gcc" if "GCC" in banner else "clang", "path": linker,
            "banner": banner,
            "version": version.group(1) if version else "unknown"}


def msvc_runtime(line: str, library: Path) -> str:
    """The CRT a static library was compiled for, from its directives."""
    directives = run_text([linker_path(line), "/dump", "/directives",
                           str(library)]).upper()
    debug = "MSVCRTD" in directives or "LIBCMTD" in directives
    if "MSVCRT" in directives:
        return "MDd" if debug else "MD"
    if "LIBCMT" in directives:
        return "MTd" if debug else "MT"
    return "unknown"


def vulkan_sdk_version(dirs: list[str]) -> str | None:
    for directory in dirs:
        match = re.search(r"VulkanSDK[\\/]+([\d.]+)", directory)
        if match:
            return match.group(1)
    return None


def pacman_versions() -> dict[str, str]:
    versions = {}
    for package in LINUX_SYSTEM_PACKAGES:
        output = run_text(["pacman", "-Q", package]).split()
        versions[package] = output[1] if len(output) == 2 else "not installed"
    return versions


def cmake_config(platform: str, mode: str, runtime: str | None,
                 libraries: list[dict], system: list[str]) -> str:
    """cgpuiConfig.cmake: one imported static target per library and
    cgpui::cgpui linking all of them in link order, then the system
    libraries."""
    lines = [
        "# Generated by scripts/ci/sdk-package.py; see docs/sdk-packaging.md.",
        "cmake_minimum_required(VERSION 3.21)",
        "include(CMakeFindDependencyMacro)",
        'get_filename_component(_cgpui_prefix "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)',
        f'set(cgpui_PLATFORM "{platform}")',
        f'set(cgpui_BUILD_MODE "{mode}")',
    ]
    if platform == "windows":
        lines += [
            f'set(cgpui_MSVC_RUNTIME "{runtime}")',
            'if(DEFINED CMAKE_MSVC_RUNTIME_LIBRARY AND',
            '   CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "^MultiThreaded(Debug)?$")',
            '  message(FATAL_ERROR "cgpui was built with /${cgpui_MSVC_RUNTIME}; '
            'the static CRT (${CMAKE_MSVC_RUNTIME_LIBRARY}) cannot link it")',
            "endif()",
            "find_dependency(Vulkan)",
        ]
    lines += ["if(NOT TARGET cgpui::cgpui)"]
    link_items = []
    for library in libraries:
        target = f"cgpui::{library['name']}"
        lines += [
            f"  add_library({target} STATIC IMPORTED)",
            f"  set_target_properties({target} PROPERTIES",
            f'    IMPORTED_LOCATION "${{_cgpui_prefix}}/{library["path"]}")',
        ]
        link_items.append(target)
    for name in system:
        vulkan = name in ("vulkan-1",) and platform == "windows"
        link_items.append("Vulkan::Vulkan" if vulkan else name)
    lines += [
        "  add_library(cgpui::cgpui INTERFACE IMPORTED)",
        "  set_target_properties(cgpui::cgpui PROPERTIES",
        '    INTERFACE_INCLUDE_DIRECTORIES "${_cgpui_prefix}/include"',
        "    INTERFACE_COMPILE_FEATURES cxx_std_23",
        '    INTERFACE_LINK_LIBRARIES "' + ";".join(link_items) + '")',
        "endif()",
        "unset(_cgpui_prefix)",
        "",
    ]
    return "\n".join(lines)


def cmake_version_file(version: str) -> str:
    """Compatible when the request is not newer and the major version
    matches (0.x: the minor version too)."""
    return "\n".join([
        "# Generated by scripts/ci/sdk-package.py.",
        f'set(PACKAGE_VERSION "{version}")',
        'string(REPLACE "." ";" _cgpui_have "${PACKAGE_VERSION}")',
        'string(REPLACE "." ";" _cgpui_want "${PACKAGE_FIND_VERSION}.0.0")',
        "list(GET _cgpui_have 0 _cgpui_have_major)",
        "list(GET _cgpui_have 1 _cgpui_have_minor)",
        "list(GET _cgpui_want 0 _cgpui_want_major)",
        "list(GET _cgpui_want 1 _cgpui_want_minor)",
        "set(PACKAGE_VERSION_COMPATIBLE FALSE)",
        "if(NOT PACKAGE_FIND_VERSION OR",
        "   (PACKAGE_FIND_VERSION VERSION_LESS_EQUAL PACKAGE_VERSION AND",
        "    _cgpui_have_major EQUAL _cgpui_want_major AND",
        "    (NOT _cgpui_have_major EQUAL 0 OR",
        "     _cgpui_have_minor EQUAL _cgpui_want_minor)))",
        "  set(PACKAGE_VERSION_COMPATIBLE TRUE)",
        "endif()",
        "if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)",
        "  set(PACKAGE_VERSION_EXACT TRUE)",
        "endif()",
        'if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)',
        '  set(PACKAGE_VERSION "${PACKAGE_VERSION} (64-bit)")',
        "  set(PACKAGE_VERSION_UNSUITABLE TRUE)",
        "endif()",
        "",
    ])


def readme(manifest: dict) -> str:
    windows = manifest["platform"] == "windows"
    runtime = manifest.get("msvc_runtime")
    lines = [
        f"# {manifest['package']}",
        "",
        f"CGPUI {manifest['version']} SDK, {manifest['platform']} "
        f"{manifest['architecture']} {manifest['mode']}, built from commit "
        f"`{manifest['git_commit']}` (clean tree). `manifest.json` lists "
        "the toolchain, dependency versions, link order and a SHA-256 for "
        "every file. C++23 is required.",
        "",
        "## CMake",
        "",
        "```cmake",
        "find_package(cgpui 0.1 REQUIRED CONFIG)  # -Dcgpui_DIR=<sdk>/lib/cmake/cgpui",
        "add_executable(app main.cpp)",
        "target_link_libraries(app PRIVATE cgpui::cgpui)",
        "```",
        "",
        "`CMAKE_PREFIX_PATH=<sdk>` works too. `cgpui::cgpui` carries the include",
        "directory, `cxx_std_23` and every library in link order, followed by",
        "the system libraries.",
    ]
    if windows:
        lines += [
            f"This package uses the /{runtime} CRT: build Debug consumers with",
            "the debug package and Release consumers with the release package",
            "(CMake's default runtimes). Vulkan comes from `find_package(Vulkan)`",
            "(set `VULKAN_SDK`).",
        ]
    lines += [
        "",
        "## xmake",
        "",
        "```lua",
        'local sdk = os.getenv("CGPUI_SDK")  -- the extracted package',
        'target("app")',
        '    set_kind("binary")',
        '    set_languages("c++23")',
    ]
    if windows:
        lines.append(f'    set_runtimes("{runtime}")')
    lines += [
        '    add_files("main.cpp")',
        '    add_includedirs(path.join(sdk, "include"))',
        '    add_linkdirs(path.join(sdk, "lib"))',
        "    add_links(" + ", ".join(f'"{n}"' for n in manifest["link_names"]) + ")",
        "    add_syslinks(" + ", ".join(f'"{n}"' for n in manifest["system_libraries"]) + ")",
    ]
    if windows:
        lines.append('    add_linkdirs(path.join(os.getenv("VULKAN_SDK"), "Lib"))')
    lines += [
        "```",
        "",
        "`share/cgpui/sdk_consumer` holds a sample with both build files; it",
        "opens a window with a client title bar and text, and with",
        "`CGPUI_EXIT_AFTER_FIRST_FRAME=1` exits after the first frame.",
        "",
    ]
    return "\n".join(lines)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def file_entries(package_root: Path) -> list[dict]:
    files = sorted(
        (path.relative_to(package_root).as_posix(), path)
        for path in package_root.rglob("*")
        if path.is_file() and path.name not in (
            "manifest.json", "release-manifest.json"))
    return [{"path": relative, "size": path.stat().st_size,
             "sha256": sha256(path)} for relative, path in files]


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def project_version(repo: Path) -> str:
    match = re.search(r'set_version\("([^"]+)"\)',
                      (repo / "xmake.lua").read_text("utf-8"))
    if match is None:
        raise SystemExit("no set_version in xmake.lua")
    return match.group(1)


def copy_libraries(args, parsed: dict, package_root: Path) -> None:
    lib_root = package_root / "lib"
    for library in parsed["libraries"]:
        destination = lib_root / library["source"].name
        shutil.copyfile(library["source"], destination)
        library["path"] = f"lib/{destination.name}"
        # Debug libraries reference <name>.pdb (/Zi); ship it beside them so
        # consumers get symbols and no LNK4099.
        pdb = library["source"].with_suffix(".pdb")
        if args.platform == "windows" and pdb.is_file():
            shutil.copyfile(pdb, lib_root / pdb.name)
        if args.platform == "windows":
            library["msvc_runtime"] = msvc_runtime(args.line, destination)
            if library["msvc_runtime"] != args.runtime:
                raise SystemExit(
                    f"{destination.name} uses /{library['msvc_runtime']}, "
                    f"expected /{args.runtime}")
    linked = {Path(library["path"]).name for library in parsed["libraries"]}
    extra = [path.name for path in lib_root.iterdir()
             if path.is_file() and path.name not in linked
             and path.suffix != ".pdb"]
    if extra:
        raise SystemExit(f"libraries missing from the link order: {extra}")


def private_header_closure(repo: Path) -> list[Path]:
    """Repository headers outside include/ reached from public headers by
    relative includes (WindowRuntime's private members), transitively."""
    include_root = (repo / "include").resolve()
    pending, found = [], set()
    for header in include_root.rglob("*.hpp"):
        for name in re.findall(r'#include "([^"]+)"', header.read_text("utf-8")):
            target = (header.parent / name).resolve()
            if include_root not in target.parents:
                pending.append(target)
    while pending:
        header = pending.pop()
        if header in found:
            continue
        if not header.is_file():
            raise SystemExit(f"public header includes a missing file: {header}")
        found.add(header)
        for name in re.findall(r'#include "([^"]+)"', header.read_text("utf-8")):
            target = (header.parent / name).resolve()
            if target.is_file() and include_root not in target.parents:
                pending.append(target)
    return sorted(found)


def copy_private_headers(repo: Path, package_root: Path) -> list[str]:
    """Copies them to the same path relative to the package root, so the
    public headers' relative includes resolve."""
    copied = []
    for header in private_header_closure(repo):
        relative = header.relative_to(repo.resolve()).as_posix()
        destination = package_root / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(header, destination)
        copied.append(relative)
    return copied


def copy_programs(args, package_root: Path) -> list[str]:
    suffix = ".exe" if args.platform == "windows" else ""
    executables = []
    for name in args.executable:
        source = args.binary_root / f"{name}{suffix}"
        if not source.is_file():
            raise SystemExit(f"missing executable: {source}")
        shutil.copy2(source, package_root / "bin" / source.name)
        executables.append(f"bin/{source.name}")
    sample = package_root / "share" / "cgpui" / "sdk_consumer"
    if sample.exists():
        shutil.rmtree(sample)
    shutil.copytree(args.repo_root / "examples" / "sdk_consumer", sample)
    return executables


def finalize(args) -> None:
    repo, package_root = args.repo_root, args.package_root
    check_clean(repo, allow_untracked=False, allow_lock=True)
    lock_rewritten = any(line[3:] == LOCK_FILE for line in status_lines(repo))
    args.line = link_line(args.link_log)
    parsed = parse_link_line(args.line, args.platform, args.binary_root)
    copy_libraries(args, parsed, package_root)
    executables = copy_programs(args, package_root)
    private_headers = copy_private_headers(repo, package_root)
    architecture = "x64" if args.platform == "windows" else "x86_64"
    name = f"cgpui-sdk-{args.platform}-{architecture}-{args.mode}"
    manifest = {
        "schema_version": 2,
        "package": name,
        "version": project_version(repo),
        "git_commit": git(repo, "rev-parse", "HEAD").strip(),
        "dirty": False,
        "source_check": "git status --porcelain empty before the build; "
                        "after it only xmake-requires.lock may differ",
        "xmake_requires_lock_rewritten_by_build": lock_rewritten,
        "platform": args.platform,
        "architecture": architecture,
        "mode": args.mode,
        "cxx_standard": "c++23",
        "compiler": compiler_info(args.line, args.platform),
        "header_root": "include/cgpui",
        "private_headers": private_headers,
        "private_headers_reason": "WindowRuntime's private members are "
                                  "declared in src/ui headers included by "
                                  "include/cgpui/ui/window_runtime.hpp",
        "libraries": [library["path"] for library in parsed["libraries"]],
        "link_names": [library["name"] for library in parsed["libraries"]],
        "library_origins": {library["name"]: library["origin"]
                            for library in parsed["libraries"]},
        "system_libraries": parsed["system"],
        "link_order_source": "example_workbench link line (xmake build -v)",
        "executables": executables,
        "cmake_config": "lib/cmake/cgpui/cgpuiConfig.cmake",
        "consumer_sample": "share/cgpui/sdk_consumer",
        "shaders": "embedded as SPIR-V in cgpui_renderer_vulkan",
        "documents": ["README.md"],
    }
    if args.platform == "windows":
        manifest["msvc_runtime"] = args.runtime
        manifest["vulkan_sdk_version"] = vulkan_sdk_version(parsed["dirs"])
    else:
        manifest["system_packages"] = pacman_versions()
    write_text(package_root / "README.md", readme(manifest))
    cmake_root = package_root / "lib" / "cmake" / "cgpui"
    write_text(cmake_root / "cgpuiConfig.cmake", cmake_config(
        args.platform, args.mode, args.runtime, parsed["libraries"],
        parsed["system"]))
    write_text(cmake_root / "cgpuiConfigVersion.cmake",
               cmake_version_file(manifest["version"]))
    manifest["files"] = file_entries(package_root)
    write_text(package_root / "manifest.json",
               json.dumps(manifest, indent=2) + "\n")
    print(json.dumps({"package": name, "files": len(manifest["files"])}))


def main() -> int:
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest="command", required=True)
    clean = commands.add_parser("check-clean")
    clean.add_argument("--repo-root", type=Path, required=True)
    clean.add_argument("--allow-untracked", action="store_true")
    final = commands.add_parser("finalize")
    final.add_argument("--platform", choices=("windows", "linux"), required=True)
    final.add_argument("--mode", choices=("debug", "release"), required=True)
    final.add_argument("--runtime", choices=("MD", "MDd"))
    final.add_argument("--repo-root", type=Path, required=True)
    final.add_argument("--binary-root", type=Path, required=True)
    final.add_argument("--link-log", type=Path, required=True)
    final.add_argument("--package-root", type=Path, required=True)
    final.add_argument("--executable", action="append", default=[])
    args = parser.parse_args()
    if args.command == "check-clean":
        check_clean(args.repo_root, args.allow_untracked, allow_lock=False)
        print(git(args.repo_root, "rev-parse", "HEAD").strip())
        return 0
    if args.platform == "windows" and args.runtime is None:
        parser.error("--runtime is required on Windows")
    finalize(args)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
