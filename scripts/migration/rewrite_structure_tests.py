#!/usr/bin/env python3
"""Rewrite the structure tests that read the xmake build files (one-time step).

Structure tests read xmake.lua and build/xmake/**/*.lua and check that test
targets, libraries and examples are registered. After the migration the
registrations are the mcpp manifests: a test program is an entry of its
suite's [test] discover list (tests/<suite>/mcpp.toml), a library or an
example is a [targets.<name>] table of its member. This script

  * maps each xmake registration literal a test checks to the mcpp string
    that registers the same thing (target("x") -> "x_test.cpp" in the suite
    manifest, add_files("tests/ui/x.cpp") -> "x.cpp", target("cgpui_ui") ->
    [targets.cgpui_ui], ...),
  * replaces each read of an xmake build file by the concatenation of the
    manifests those literals live in, and renames the variable,
  * prints every literal it cannot map, for a manual rewrite.

  python scripts/migration/rewrite_structure_tests.py [--write] [files...]
"""
import argparse
import json
import pathlib
import re
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[2]
HELPERS = {
    "tests/ui/native_text_shaping_reference.cpp",
    "tests/platform/wayland_test_compositor.cpp",
    "tests/platform/wayland_fake_portal.cpp",
    "tests/platform/wayland_private_session_bus.cpp",
    "tests/platform/wayland_test_background_effect.cpp",
}
LIBRARY_PACKAGES = {
    "cgpui_core": "core", "cgpui_platform": "platform",
    "cgpui_platform_fallback": "platform-fallback", "cgpui_platform_win32": "platform-win32",
    "cgpui_platform_linux_atspi": "platform-linux-atspi",
    "cgpui_platform_linux_x11": "platform-linux-x11",
    "cgpui_platform_linux_wayland": "platform-linux-wayland",
    "cgpui_renderer": "renderer", "cgpui_renderer_fallback": "renderer-fallback",
    "cgpui_renderer_vulkan": "renderer-vulkan", "cgpui_ui": "ui", "cgpui_app": "app",
}
EXAMPLE_MANIFESTS = {
    "hello_window": "examples/hello_window/mcpp.toml",
    "example_backdrop_blur": "examples/backdrop_blur/mcpp.toml",
    "example_client_title_bar": "examples/client_title_bar/mcpp.toml",
    "example_workbench": "examples/workbench/mcpp.toml",
}
XMAKE_FILE = re.compile(r'"((?:xmake\.lua)|(?:build/xmake/[A-Za-z0-9_/]*\.lua))"')


def registrations():
    """xmake target name -> (manifest, mcpp literal text)."""
    inventory = json.loads((ROOT / "docs/migration/xmake-targets.json").read_text(encoding="utf-8"))
    result = {}
    for target in inventory["targets"]:
        name = target["name"]
        info = target["platforms"].get("windows") or target["platforms"].get("linux") \
            or target["platforms"].get("macosx")
        if name in LIBRARY_PACKAGES:
            result[name] = (f"packages/{LIBRARY_PACKAGES[name]}/mcpp.toml", f"[targets.{name}]")
        elif name in EXAMPLE_MANIFESTS:
            result[name] = (EXAMPLE_MANIFESTS[name], f"[targets.{name}]")
        elif name.startswith("api_parity_"):
            result[name] = ("examples/api_parity/mcpp.toml", f"[targets.{name}]")
        elif "windows" in target["platforms"] or "linux" in target["platforms"]:
            mains = [f for f in info["files"] if f.startswith("tests/") and f not in HELPERS]
            if len(mains) == 1:
                suite = mains[0].split("/")[1]
                result[name] = (f"tests/{suite}/mcpp.toml", '"' + mains[0].split("/")[-1] + '"')
    return result


LITERAL = re.compile(r'"(?:[^"\\\n]|\\.)*"')


def decode(literal):
    return bytes(literal[1:-1], "utf-8").decode("unicode_escape")


def encode(text):
    return '"' + text.replace("\\", "\\\\").replace('"', '\\"') + '"'


# includes("build/xmake/<module>.lua") -> the workspace member that now holds
# the module's targets (checked as an entry of the root members list).
INCLUDES = {
    "phase_h_macos": "packages/platform-macos",
    "phase_h_metal": "packages/renderer-metal",
    "phase_i_linux_backends": "packages/platform-linux-x11",
    "phase_h_structure_targets": "tests/architecture",
    "phase_h_closeout_targets": "tests/architecture",
    "phase_i_structure_targets": "tests/architecture",
    "phase_i_closeout_targets": "tests/architecture",
    "phase_j_audit_targets": "tests/architecture",
    "phase_j_example_targets": "tests/architecture",
    "phase_j_performance_targets": "tests/performance",
    "phase_j_stress_targets": "tests/stress",
    "phase_j_documentation_targets": "tests/architecture",
    "phase_j_release_targets": "tests/architecture",
    "phase_j_closeout_targets": "tests/architecture",
    "capabilities/*": "tests/ui",
}
P = "packages/"
MACOS_TESTS = ("tests/platform/mcpp.toml", '"macos/macos_*.mm"')
METAL_TESTS = ("tests/renderer/mcpp.toml", '"metal/metal_*.mm"')
# Exact xmake texts -> the mcpp manifest text that states the same thing.
MANUAL = {
    'add_files("src/ui/widgets/*.cpp")': (P + "ui/mcpp.toml", '"../../src/ui/widgets/*.cpp"'),
    "src/ui/workbench/*.cpp": (P + "ui/mcpp.toml", '"../../src/ui/workbench/*.cpp"'),
    "src/platform/linux/x11/*.cpp": (P + "platform-linux-x11/mcpp.toml", '"../../src/platform/linux/x11/*.cpp"'),
    "src/platform/macos/*.mm": (P + "platform-macos/mcpp.toml", '"../../src/platform/macos/*.mm"'),
    "src/renderer/metal/*.mm": (P + "renderer-metal/mcpp.toml", '"../../src/renderer/metal/*.mm"'),
    'add_files(path.join(os.projectdir(), "src/renderer/metal/*.mm"))':
        (P + "renderer-metal/mcpp.toml", '"../../src/renderer/metal/*.mm"'),
    'add_includedirs(path.join(os.projectdir(), "include")': (P + "renderer-metal/mcpp.toml", 'include_dirs = ["../../include"]'),
    'add_requires("fontconfig"': (P + "renderer/build.mcpp", '"fontconfig"'),
    'has_package("fontconfig")': (P + "platform-linux-wayland/build.mcpp", '"fontconfig"'),
    "CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND": (P + "platform-linux-wayland/mcpp.toml", "CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND"),
    'add_packages("fontconfig")': (P + "platform-linux-wayland/build.mcpp", '"fontconfig"'),
    'is_plat("windows")': ("tests/platform/mcpp.toml", "[target.'cfg(os = \"windows\")'.test]"),
    'is_plat("linux")': ("tests/platform/mcpp.toml", "[target.'cfg(os = \"linux\")'.test]"),
    'includes("build/xmake/capabilities/*.lua")': ("mcpp.toml", '"tests/ui",'),
    "remove_files(": (P + "ui/mcpp.toml", '"!../../src/ui/text_'),
    "text_shaping_directwrite.cpp": (P + "renderer/mcpp.toml", '"../../src/ui/text_shaping_directwrite.cpp"'),
    "text_dwrite_analysis_win32.cpp": (P + "renderer/mcpp.toml", '"../../src/ui/text_dwrite_analysis_win32.cpp"'),
    'add_requires("harfbuzz", {system = true})': (P + "renderer/build.mcpp", '"harfbuzz"'),
    'add_syslinks("wayland-cursor"': (P + "platform-linux-wayland/build.mcpp", '"wayland-cursor"'),
    'add_syslinks("shell32", "user32", {public = true})': (P + "platform/mcpp.toml", 'ldflags = ["shell32.lib", "user32.lib"]'),
    'add_includedirs("third_party/stb")': (P + "renderer/mcpp.toml", '"../../third_party/stb"'),
    'add_requires("stb': (P + "renderer/mcpp.toml", "vendor.stb"),
    'add_requires("dbus", {system = true})': (P + "platform-linux-atspi/build.mcpp", '"dbus-1"'),
    'add_packages("wayland", "dbus", "libxkbcommon")':
        (P + "platform-linux-wayland/build.mcpp", '"wayland-client", "wayland-cursor", "dbus-1", "xkbcommon"'),
    '"uiautomationcore"': (P + "platform-win32/mcpp.toml", '"uiautomationcore.lib"'),
    '"oleaut32"': (P + "platform-win32/mcpp.toml", '"oleaut32.lib"'),
    'add_syslinks("dwmapi"': (P + "platform-win32/mcpp.toml", '"dwmapi.lib"'),
    'add_syslinks("xcb"': (P + "platform-linux-x11/build.mcpp", '"xcb"'),
    "libxkbcommon": (P + "platform-linux-x11/build.mcpp", '"xkbcommon"'),
    "xkbcommon-x11": (P + "platform-linux-x11/build.mcpp", '"xkbcommon-x11"'),
    "xcb-cursor": (P + "platform-linux-x11/build.mcpp", '"xcb-cursor"'),
    'add_frameworks("AppKit", "QuartzCore")': (P + "platform-macos/mcpp.toml", '"AppKit"'),
    'add_frameworks("Metal", "QuartzCore")': (P + "renderer-metal/mcpp.toml", 'frameworks = ["Metal", "QuartzCore"]'),
    'add_frameworks("AppKit", "QuartzCore", "Metal")': ("examples/hello_window/mcpp.toml", "cgpui.renderer-metal"),
    'add_deps("cgpui_platform_macos", "cgpui_renderer_metal")': ("examples/hello_window/mcpp.toml", "cgpui.platform-macos"),
    "xcrun": (P + "renderer-metal/build.mcpp", "xcrun"),
    "metallib": (P + "renderer-metal/build.mcpp", "metallib"),
    "for _, metal_pixel_test in ipairs": METAL_TESTS,
    '"metal_primitive_pixel_test"': METAL_TESTS,
    '"metal_text_image_pixel_test"': METAL_TESTS,
    '"metal_clip_transform_pixel_test"': METAL_TESTS,
    'add_files(path.join(os.projectdir(), "tests/renderer/metal/metal_bootstrap_test.mm"))': METAL_TESTS,
    'add_files(path.join(os.projectdir(), "tests/architecture/phase_h_macos_configuration_structure_test.cpp"))':
        ("tests/architecture/mcpp.toml", '"phase_h_macos_configuration_structure_test.cpp"'),
    "CGPUI_SOURCE_ROOT": ("tests/architecture/build.mcpp", "source_root_define(2)"),
    "cgpui_ui": ("tests/stress/mcpp.toml", "cgpui.ui"),
    'add_tests("default")': ("tests/performance/mcpp.toml", "[test]"),
    'add_tests("x11_demo_smoke_flow"': ("examples/hello_window/mcpp.toml", '"tests/x11_demo_smoke_flow.cpp"'),
    'set_basename("phase_g_test_app_window_fixture_structure_test")':
        ("tests/architecture/mcpp.toml", '"phase_g_test_app_window_setup_structure_test.cpp"'),
    "cgpui_platform_win32": (P + "platform-win32/mcpp.toml", "[targets.cgpui_platform_win32]"),
    'add_files("src/ui/text_shaping_harfbuzz.cpp")': (P + "renderer/mcpp.toml", '"../../src/ui/text_shaping_harfbuzz.cpp"'),
    'remove_files("src/ui/text_shaping_harfbuzz.cpp")': (P + "ui/mcpp.toml", '"!../../src/ui/text_shaping_harfbuzz.cpp"'),
    'remove_files("src/ui/text_font.cpp")': (P + "ui/mcpp.toml", '"!../../src/ui/text_font.cpp"'),
    'set_policy("package.requires_lock", true)': ("third_party/VENDORED.md", "Source archive SHA-256"),
    'add_requires("lunasvg")': (P + "renderer/mcpp.toml", 'vendor.lunasvg = { path = "../../third_party/lunasvg" }'),
}
for _name in ("macos_accessibility_test", "macos_clipboard_test", "macos_drag_drop_test",
              "macos_example_smoke_test", "macos_input_event_test", "macos_platform_services_test",
              "macos_font_discovery_test", "macos_text_input_test", "macos_window_lifecycle_test"):
    MANUAL[f'target("{_name}")'] = MACOS_TESTS
MANUAL['target("metal_bootstrap_test")'] = METAL_TESTS
MANUAL['target("cgpui_platform_macos")'] = (P + "platform-macos/mcpp.toml", "[targets.cgpui_platform_macos]")
MANUAL['target("cgpui_renderer_metal")'] = (P + "renderer-metal/mcpp.toml", "[targets.cgpui_renderer_metal]")


def map_literal(text, regs):
    """Return (manifest, mcpp text) for an xmake registration text, or None."""
    if text in MANUAL:
        return MANUAL[text]
    match = re.fullmatch(r'includes\("build/xmake/([a-z_/*]+)\.lua"\)', text)
    if match:
        return ("mcpp.toml", f'"{INCLUDES[match.group(1)]}",')
    match = re.fullmatch(r'target\("([A-Za-z0-9_]+)"\)', text)
    if match:
        return regs.get(match.group(1))
    match = re.fullmatch(r'(?:add_files\(")?(tests/([a-z_]+)/(?:[a-z_]+/)*([A-Za-z0-9_]+\.(?:cpp|mm)))(?:"\)?)?', text)
    if match:
        path, suite, base = match.groups()
        if (ROOT / path).exists() and (ROOT / "tests" / suite / "mcpp.toml").exists() and "/" not in path[len("tests/" + suite + "/"):]:
            return (f"tests/{suite}/mcpp.toml", f'"{base}"')
        return None
    match = re.fullmatch(r'(?:add_files\(")?examples/api_parity/([a-z_]+)/main\.cpp(?:"\)?)?', text)
    if match:
        return ("examples/api_parity/mcpp.toml", f'main = "{match.group(1)}/main.cpp"')
    match = re.fullmatch(r'(?:add_files\(")?examples/([a-z_]+)/main\.cpp(?:"\)?)?', text)
    if match and (ROOT / "examples" / match.group(1) / "mcpp.toml").exists():
        return (f"examples/{match.group(1)}/mcpp.toml", 'main = "main.cpp"')
    if re.fullmatch(r"[A-Za-z0-9_]+", text) and text in regs and not text.startswith("cgpui_"):
        return regs[text]
    match = re.fullmatch(r"[A-Za-z0-9_]+\.cpp", text)
    if match:
        suites = [p.parent.name for p in (ROOT / "tests").glob("*/" + text)]
        if len(suites) == 1:
            return (f"tests/{suites[0]}/mcpp.toml", f'"{text}"')
    match = re.fullmatch(r"tests/([a-z_]+)/", text)
    if match and (ROOT / "tests" / match.group(1) / "mcpp.toml").exists():
        suite = match.group(1)
        return (f"tests/{suite}/mcpp.toml", f'name = "tests-{suite.replace("_", "-")}"')
    return None


def code_segments(source):
    """Yield (is_literal, text) pieces of a C++ source, skipping // comments."""
    pos = 0
    for match in re.finditer(r'//[^\n]*|R"([^(]*)\((?:.|\n)*?\)\1"|"(?:[^"\\\n]|\\.)*"|\'(?:[^\'\\\n]|\\.)*\'', source):
        if match.start() > pos:
            yield False, source[pos:match.start()]
        text = match.group(0)
        yield text.startswith('"'), text
        pos = match.end()
    yield False, source[pos:]


ASSIGN = re.compile(
    r'(?P<decl>(?:const\s+)?(?:std::string|auto)\s+(?P<var>\w+)\s*=\s*)'
    r'(?P<read>(?P<fn>\w+)\(\s*(?P<path>"(?:xmake\.lua|build/xmake/[A-Za-z0-9_/]*\.lua)")\s*\))\s*;')
RENAMES = {"xmake": "manifests", "xmake_text": "manifests_text"}


def checked_literals(source, var):
    """(start, end, text) of each literal passed to a check of `var`."""
    calls = re.compile(r"\b(?:contains|has|target_block|count|occurrences)\(\s*" + re.escape(var)
                       + r"\s*,\s*((?:" + LITERAL.pattern + r"\s*)+)\)")
    found = []
    for call in calls.finditer(source):
        group_start = call.start(1)
        for literal in LITERAL.finditer(call.group(1)):
            found.append((group_start + literal.start(), group_start + literal.end(), literal.group(0)))
    return found


def rename_identifiers(source, renames):
    pieces = []
    for is_literal, text in code_segments(source):
        if not is_literal and not text.startswith("//"):
            for old, new in renames.items():
                text = re.sub(r"\b" + re.escape(old) + r"\b", new, text)
        pieces.append(text)
    return "".join(pieces)


def rewrite(path, regs):
    source = path.read_text(encoding="utf-8")
    unmapped = []
    assignments = list(ASSIGN.finditer(source))
    edits = []
    renames = {}
    for assignment in assignments:
        var = assignment.group("var")
        manifests = []
        for start, end, literal in checked_literals(source, var):
            mapped = map_literal(decode(literal), regs)
            if mapped is None:
                line = source.count("\n", 0, start) + 1
                unmapped.append(f"{path.relative_to(ROOT).as_posix()}:{line}: {var}: {literal}")
                continue
            manifest, text = mapped
            if manifest not in manifests:
                manifests.append(manifest)
            edits.append((start, end, encode(text)))
        manifests = manifests or ["mcpp.toml"]
        fn = assignment.group("fn")
        read = " +\n      ".join(f'{fn}("{m}")' for m in manifests)
        edits.append((assignment.start("read"), assignment.end("read"), read))
        if var in RENAMES:
            renames[var] = RENAMES[var]
    for start, end, text in sorted(edits, reverse=True):
        source = source[:start] + text + source[end:]
    if renames:
        source = rename_identifiers(source, renames)
    return source, unmapped, len(assignments)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("files", nargs="*")
    args = parser.parse_args()
    files = args.files or subprocess.check_output(
        ["git", "grep", "-l", "-E", r'"(xmake\.lua|build/xmake/)', "--", "tests"],
        cwd=ROOT, text=True).split()
    regs = registrations()
    total_unmapped = 0
    for name in files:
        path = ROOT / name
        source, unmapped, reads = rewrite(path, regs)
        total_unmapped += len(unmapped)
        for entry in unmapped:
            print(entry)
        if args.write and reads:
            path.write_text(source, encoding="utf-8", newline="\n")
    print(f"{len(files)} files, {total_unmapped} unmapped literals")


if __name__ == "__main__":
    main()
