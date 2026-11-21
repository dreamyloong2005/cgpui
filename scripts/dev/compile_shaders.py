#!/usr/bin/env python3
"""Compiles the Slang shaders of CGPUI and writes the checked-in outputs.

  python scripts/dev/compile_shaders.py           regenerate every output
  python scripts/dev/compile_shaders.py --check   exit 1 if an output differs
  --metal  only the Metal outputs (a Mac without the Vulkan SDK; spirv-val
           is not needed)

Vulkan: SPIR-V 1.3 (the renderer requests Vulkan 1.1), checked with
spirv-val --target-env vulkan1.1, written as uint32 arrays to
src/renderer/vulkan/generated/<pipeline>_spirv.inc; the
vulkan_*_shader_binaries.cpp files include them.
Metal: one Metal Shading Language source per entry point in
src/renderer/metal/generated/<entry>.metal (slangc gives every entry point
of one compile all global parameters, so each is compiled alone);
packages/renderer-metal/build.mcpp embeds them.

slangc and spirv-val come from SLANGC / SPIRV_VAL, else the Vulkan SDK
($VULKAN_SDK), else PATH. Each output names the Slang sources it is compiled
from with the FNV-1a 64 hash of their text (LF line ends);
tests/architecture/shader_source_structure_test.cpp checks that the hashes
match the sources, so an edited shader must be compiled again.
"""

from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
SHARED = "src/renderer/shaders"
VULKAN_FLAGS = ["-target", "spirv", "-profile", "spirv_1_3", "-O2"]
METAL_FLAGS = ["-target", "metal", "-O2", "-line-directive-mode", "none"]

# <pipeline>: [(array name, Slang source, entry point, stage)]. The array
# names are those the vulkan_*_shader_binaries.cpp accessors return.
VULKAN = {
    "rounded_rect": [
        ("rounded_rect_vertex_shader_spirv", "rounded_rect", "vertex_main", "vertex"),
        ("rounded_rect_fragment_shader_spirv", "rounded_rect", "fragment_main", "fragment"),
    ],
    "text": [
        ("text_vertex_shader_spirv", "text", "vertex_main", "vertex"),
        ("text_fragment_shader_spirv", "text", "fragment_main", "fragment"),
    ],
    "image": [
        ("image_vertex_shader_spirv", "image", "vertex_main", "vertex"),
        ("image_fragment_shader_spirv", "image", "fragment_main", "fragment"),
    ],
    "box_shadow": [
        ("box_shadow_vertex_shader_spirv", "box_shadow", "vertex_main", "vertex"),
        ("box_shadow_fragment_shader_spirv", "box_shadow", "fragment_main", "fragment"),
    ],
    "backdrop_blur": [
        ("backdrop_fullscreen_vert_spirv", "backdrop", "fullscreen_vertex", "vertex"),
        ("backdrop_downsample_frag_spirv", "backdrop", "downsample_fragment", "fragment"),
        ("backdrop_blur_frag_spirv", "backdrop", "blur_fragment", "fragment"),
        ("backdrop_present_frag_spirv", "backdrop", "present_fragment", "fragment"),
        ("backdrop_composite_vert_spirv", "backdrop", "composite_vertex", "vertex"),
        ("backdrop_composite_frag_spirv", "backdrop", "composite_fragment", "fragment"),
    ],
}
METAL_SOURCE = "src/renderer/metal/shaders/cgpui_metal.slang"
# The function names src/renderer/metal/metal_pipeline_library.mm looks up.
METAL = [
    ("cgpui_vertex", "vertex"),
    ("cgpui_solid_fragment", "fragment"),
    ("cgpui_rounded_fragment", "fragment"),
    ("cgpui_glyph_fragment", "fragment"),
    ("cgpui_image_fragment", "fragment"),
    ("cgpui_box_shadow_vertex", "vertex"),
    ("cgpui_box_shadow_fragment", "fragment"),
    ("cgpui_backdrop_fragment", "fragment"),
]


def fnv1a64(path: str) -> str:
    value = 0xCBF29CE484222325
    for byte in (REPO / path).read_bytes().replace(b"\r\n", b"\n"):
        value = ((value ^ byte) * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return f"{value:016x}"


def sources(path: str) -> list[str]:
    """The Slang file and the shared modules it imports (one level)."""
    text = (REPO / path).read_text("utf-8")
    imported = re.findall(r"^\s*import\s+([A-Za-z_][A-Za-z0-9_]*)\s*;", text, re.M)
    return [path] + [f"{SHARED}/{name}.slang" for name in imported]


def tool(variable: str, name: str) -> str:
    if os.environ.get(variable):
        return os.environ[variable]
    sdk = os.environ.get("VULKAN_SDK")
    if sdk:
        for sub in ("Bin", "bin"):
            for exe in (name + ".exe", name):
                candidate = Path(sdk) / sub / exe
                if candidate.is_file():
                    return str(candidate)
    found = shutil.which(name)
    if not found:
        raise SystemExit(f"{name} not found: set {variable} or VULKAN_SDK")
    return found


def run(argv: list[str]) -> str:
    done = subprocess.run(argv, capture_output=True, text=True, encoding="utf-8",
                          errors="replace")
    if done.returncode != 0:
        raise SystemExit(f"failed ({done.returncode}): {' '.join(argv)}\n"
                         f"{done.stdout}{done.stderr}")
    return done.stdout


def slangc_version(slangc: str) -> str:
    done = subprocess.run([slangc, "-version"], capture_output=True, text=True)
    return (done.stdout + done.stderr).strip().splitlines()[0]


def compile_one(slangc: str, source: str, entry: str, stage: str,
                flags: list[str], output: Path) -> None:
    run([slangc, str(REPO / source), "-I", str(REPO / SHARED), *flags,
         "-entry", entry, "-stage", stage, "-o", str(output)])


def header(lines: list[str], version: str, flags: list[str],
           files: list[str]) -> list[str]:
    out = ["// Generated by scripts/dev/compile_shaders.py; do not edit.",
           f"// slangc {version}: {' '.join(flags)}"]
    out += [f"// source {path} fnv1a64 {fnv1a64(path)}" for path in files]
    return out + lines


def vulkan_output(slangc: str, validator: str, version: str, pipeline: str,
                  scratch: Path) -> str:
    entries = VULKAN[pipeline]
    files: list[str] = []
    body: list[str] = []
    for array, name, entry, stage in entries:
        source = f"src/renderer/vulkan/shaders/{name}.slang"
        files += [f for f in sources(source) if f not in files]
        spv = scratch / f"{array}.spv"
        compile_one(slangc, source, entry, stage, VULKAN_FLAGS, spv)
        run([validator, "--target-env", "vulkan1.1", str(spv)])
        data = spv.read_bytes()
        words = [int.from_bytes(data[i:i + 4], "little")
                 for i in range(0, len(data), 4)]
        if len(data) % 4 or not words or words[0] != 0x07230203:
            raise SystemExit(f"{array}: not a SPIR-V module")
        body.append(f"// {array}: {source} {entry} ({stage})")
        body.append(f"constexpr std::array<std::uint32_t, {len(words)}> {array}{{")
        for i in range(0, len(words), 6):
            body.append("    " + " ".join(f"0x{w:08x}U," for w in words[i:i + 6]))
        body.append("};")
    return "\n".join(header(body, version, VULKAN_FLAGS, files)) + "\n"


def metal_output(slangc: str, version: str, entry: str, stage: str,
                 scratch: Path) -> str:
    out = scratch / f"{entry}.metal"
    compile_one(slangc, METAL_SOURCE, entry, stage, METAL_FLAGS, out)
    text = out.read_text("utf-8").replace("\r\n", "\n").rstrip("\n")
    if ")CGPUI_METAL\"" in text:
        raise SystemExit(f"{entry}: output contains the raw-string delimiter")
    lines = header([f"// entry {entry} ({stage})", text], version, METAL_FLAGS,
                   sources(METAL_SOURCE))
    return "\n".join(lines) + "\n"


def outputs(metal_only: bool) -> dict[str, str]:
    slangc = tool("SLANGC", "slangc")
    version = slangc_version(slangc)
    result: dict[str, str] = {}
    with tempfile.TemporaryDirectory() as directory:
        scratch = Path(directory)
        validator = "" if metal_only else tool("SPIRV_VAL", "spirv-val")
        for pipeline in ([] if metal_only else VULKAN):
            path = f"src/renderer/vulkan/generated/{pipeline}_spirv.inc"
            result[path] = vulkan_output(slangc, validator, version, pipeline, scratch)
        for entry, stage in METAL:
            path = f"src/renderer/metal/generated/{entry}.metal"
            result[path] = metal_output(slangc, version, entry, stage, scratch)
    return result


def main() -> int:
    check = "--check" in sys.argv[1:]
    stale = []
    for path, text in outputs("--metal" in sys.argv[1:]).items():
        target = REPO / path
        current = target.read_text("utf-8").replace("\r\n", "\n") if target.exists() else None
        if current == text:
            continue
        stale.append(path)
        if not check:
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(text, "utf-8", newline="\n")
    for path in stale:
        print(("stale: " if check else "wrote: ") + path)
    return 1 if check and stale else 0


if __name__ == "__main__":
    sys.exit(main())
