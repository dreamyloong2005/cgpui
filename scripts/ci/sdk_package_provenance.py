"""Toolchain and dependency provenance for the SDK manifest
(docs/sdk-packaging.md): compiler, CRT, mcpp, Vulkan SDK, Linux system
packages and the vendored third_party/ trees as committed at HEAD."""

from __future__ import annotations

import hashlib
import json
import re
import subprocess
import tomllib
from pathlib import Path

LINUX_SYSTEM_PACKAGES = (
    "harfbuzz", "freetype2", "fontconfig", "wayland", "libxkbcommon",
    "dbus", "vulkan-icd-loader", "libxml2",
)


def git(repo: Path, *args: str) -> str:
    return subprocess.run(
        ["git", "-C", str(repo), *args], check=True, capture_output=True,
        text=True).stdout


def run_text(command: list[str]) -> str:
    result = subprocess.run(command, capture_output=True, text=True,
                            errors="replace")
    return (result.stdout + result.stderr).strip()


def compiler_path(build_dir: Path) -> str:
    entries = json.loads((build_dir / "compile_commands.json").read_text("utf-8"))
    for entry in entries:
        words = entry.get("arguments") or entry["command"].split()
        if words and Path(words[0]).name.lower() in ("cl.exe", "g++", "c++", "clang++"):
            return words[0]
    raise SystemExit(f"no C++ compiler in {build_dir / 'compile_commands.json'}")


def compiler_info(build_dir: Path, platform: str) -> dict:
    compiler = compiler_path(build_dir)
    if platform == "windows":
        # The banner may be localized; the version follows "C/C++".
        banner = next((line for line in run_text([compiler]).splitlines()
                       if "C/C++" in line), "unknown")
        version = re.search(r"C/C\+\+\D*(\d+\.\d+\.\d+)", banner)
        return {"id": "msvc", "path": compiler,
                "version": version.group(1) if version else "unknown",
                "toolset": Path(compiler).parents[3].name}
    banner = run_text([compiler, "--version"]).splitlines()[0]
    version = re.search(r"(\d+\.\d+\.\d+)", banner)
    return {"id": "gcc" if "GCC" in banner or "g++" in banner else "clang",
            "path": compiler, "banner": banner,
            "version": version.group(1) if version else "unknown"}


def msvc_runtime(build_dir: Path, library: Path) -> str:
    """The CRT a static library was compiled for, from its directives."""
    link = str(Path(compiler_path(build_dir)).with_name("link.exe"))
    directives = run_text([link, "/dump", "/directives", str(library)]).upper()
    debug = "MSVCRTD" in directives or "LIBCMTD" in directives
    if "MSVCRT" in directives:
        return "MDd" if debug else "MD"
    if "LIBCMT" in directives:
        return "MTd" if debug else "MT"
    return "unknown"


def vulkan_sdk_version(dirs: list[str]) -> str | None:
    for directory in dirs:
        match = re.search(r"VulkanSDK[\/]+([\d.]+)", directory)
        if match:
            return match.group(1)
    return None


def pacman_versions() -> dict[str, str]:
    versions = {}
    for package in LINUX_SYSTEM_PACKAGES:
        output = run_text(["pacman", "-Q", package]).split()
        versions[package] = output[1] if len(output) == 2 else "not installed"
    return versions


def tree_sha256(repo: Path, directory: str) -> str:
    """SHA-256 over the committed files of a directory (the bytes in HEAD,
    so line-ending conversion of a checkout does not change it): one line
    `<sha256>  <path>` per file, sorted by path (sha256sum format)."""
    files = sorted(n for n in git(repo, "ls-tree", "-r", "-z", "--name-only",
                                  "HEAD", "--", directory).split("\0") if n)
    lines = []
    for name in files:
        blob = subprocess.run(["git", "-C", str(repo), "show", f"HEAD:{name}"],
                              check=True, capture_output=True).stdout
        lines.append(f"{hashlib.sha256(blob).hexdigest()}  {name}\n")
    return hashlib.sha256("".join(lines).encode("utf-8")).hexdigest()


def vendored_dependencies(repo: Path, bundled: set[str]) -> list[dict]:
    """third_party/VENDORED.md rows, with a SHA-256 of each committed tree."""
    rows = []
    table = (repo / "third_party" / "VENDORED.md").read_text("utf-8")
    for line in table.splitlines():
        cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
        if len(cells) != 6 or not cells[0].startswith("`") or cells[0] == "Directory":
            continue
        directory = cells[0].strip("`").rstrip("/")
        name = re.sub(r"\[([^\]]+)\].*", r"\1", cells[1])
        rows.append({
            "name": name,
            "directory": f"third_party/{directory}",
            "version": cells[2].strip("`"),
            "upstream_commit": cells[3].strip("`"),
            "license": cells[4],
            "source_archive_sha256": cells[5].strip("`") if cells[5] != "-" else None,
            "tree_sha256": tree_sha256(repo, f"third_party/{directory}"),
            "bundled": directory in bundled,
        })
    return rows


def mcpp_info(mcpp: str, workspace: Path) -> dict:
    version = run_text([mcpp, "--version"]).splitlines()[0]
    manifest = tomllib.loads((workspace / "mcpp.toml").read_text("utf-8"))
    return {"version": version.removeprefix("mcpp ").strip(), "path": mcpp,
            "toolchain": manifest["workspace"].get("toolchain", {}),
            "cxx_runtime": manifest["workspace"]["build"]["cxx_runtime"]}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()
