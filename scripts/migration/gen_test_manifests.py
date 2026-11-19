#!/usr/bin/env python3
"""Write tests/<suite>/mcpp.toml from the xmake inventory (one-time step).

For each test directory, the discover list names every test file the xmake
build registered on Windows and on Linux: files of both platforms in
[test] discover, files of one in [target.'cfg(os = ...)'.test] discover. The
dependencies, include directories and support sources per suite are given in
SUITES below (the union of what that directory's xmake targets used). The
manifests are maintained by hand afterwards; this script documents how they
were derived.

  python scripts/migration/gen_test_manifests.py
"""
import collections
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
HELPERS = {
    "tests/ui/native_text_shaping_reference.cpp",
    "tests/platform/wayland_test_compositor.cpp",
    "tests/platform/wayland_fake_portal.cpp",
    "tests/platform/wayland_private_session_bus.cpp",
    "tests/platform/wayland_test_background_effect.cpp",
}
CORE = ["core", "platform", "renderer", "ui"]
WIDE = ["core", "platform", "renderer", "renderer-vulkan", "ui", "app"]
# suite: (description, deps, windows deps, linux deps, include dirs,
#         windows include dirs, linux include dirs, build program helpers)
SUITES = {
    "architecture": ("Structure tests: read the repository and check module boundaries.",
                     ["core", "platform", "renderer"], [], [], [], [], [], []),
    "api_parity": ("GPUI API parity, phase audits and closeout tests.", WIDE, [], [], [], [], [], []),
    "header_cleanliness": ("Each program includes one public header and nothing else.",
                           ["core", "platform", "renderer", "ui", "app"], [], [], [], [], [], []),
    "ui": ("Elements, layout, text, widgets and the window runtime.",
           ["core", "platform", "renderer", "ui", "app"], [], [], ["src/ui", "tests/ui"], [], [], ["harfbuzz"]),
    "platform": ("Platform services and the Win32 / Wayland / X11 backends.",
                 CORE, ["platform-win32", "renderer-vulkan", "app"], ["platform-linux-wayland"],
                 ["src/platform/linux", "tests/ui"], ["src/platform/win32"],
                 ["src/platform/linux/x11", "tests/platform"], ["platform"]),
    "renderer": ("Renderer, text pipeline and Vulkan backend.",
                 ["core", "platform", "renderer", "renderer-vulkan", "ui"], ["platform-win32"],
                 ["platform-linux-wayland"], ["src/renderer/vulkan", "src/ui", "tests/renderer"], [], [],
                 ["vulkan"]),
    "async": ("Async runtime tests.", CORE, [], [], ["tests/ui"], [], [], []),
    "animation": ("Animation tests.", CORE, [], [], ["tests/ui"], [], [], []),
    "assets": ("Asset loading tests.", CORE, [], [], ["tests/ui"], [], [], []),
    "core": ("Core entity store tests.", ["core"], [], [], [], [], [], []),
    "performance": ("Performance baseline runner.", CORE, [], [], [], [], [], []),
    "stress": ("Stress matrix runner.", CORE, [], [], [], [], [], []),
}


def main_file(files):
    mains = [f for f in files if f.startswith("tests/") and f not in HELPERS]
    assert len(mains) == 1, files
    return mains[0]


def collect():
    inventory = json.loads((ROOT / "docs/migration/xmake-targets.json").read_text(encoding="utf-8"))
    tests = collections.defaultdict(lambda: collections.defaultdict(set))
    for target in inventory["targets"]:
        for platform in ("windows", "linux"):
            info = target["platforms"].get(platform)
            if not info or not info["tests"] or info["kind"] != "binary":
                continue
            if not any(f.startswith("tests/") for f in info["files"]):
                continue
            main = main_file(info["files"])
            suite = main.split("/")[1]
            tests[suite][main].add(platform)
    return tests


SUPPORT = {
    "ui": (["native_text_shaping_reference.cpp"], [], []),
    # linux_backend_selection.cpp is part of cgpui_platform on Linux only.
    "platform": ([], ["../../src/platform/linux/linux_backend_selection.cpp"],
                 ["wayland_test_compositor.cpp", "wayland_fake_portal.cpp",
                  "wayland_private_session_bus.cpp", "wayland_test_background_effect.cpp"]),
}
PROGRAMS = {
    "harfbuzz": ('  if (cgpui_build::target_is("linux")) {\n'
                 '    cgpui_build::pkg_config({"harfbuzz"});\n  }\n'),
    "platform": ('  if (cgpui_build::target_is("linux")) {\n'
                 '    cgpui_build::pkg_config({"wayland-client", "wayland-server", "wayland-cursor",\n'
                 '                             "dbus-1", "xcb", "xkbcommon", "xkbcommon-x11"});\n  }\n'),
    "vulkan": ('  if (cgpui_build::target_is("windows")) {\n'
               '    cgpui_build::vulkan_sdk(false);\n'
               '  } else if (cgpui_build::target_is("linux")) {\n'
               '    cgpui_build::pkg_config({"vulkan", "xcb", "wayland-client"}, false);\n  }\n'),
}


def toml_list(key, items, indent=""):
    if not items:
        return f"{key} = []\n"
    body = "".join(f'    "{item}",\n' for item in items)
    return f"{key} = [\n{body}]\n"


def deps_table(deps):
    return "".join(f'cgpui.{dep} = {{ path = "../../packages/{dep}" }}\n' for dep in deps)


def write_suite(suite, files):
    desc, deps, win_deps, lin_deps, incs, win_incs, lin_incs, programs = SUITES[suite]
    support, win_support, lin_support = SUPPORT.get(suite, ([], [], []))
    common = sorted(f.split("/")[-1] for f, p in files.items() if p == {"windows", "linux"})
    win = sorted(f.split("/")[-1] for f, p in files.items() if p == {"windows"})
    lin = sorted(f.split("/")[-1] for f, p in files.items() if p == {"linux"})
    rel = lambda d: "../../" + d if not d.startswith("tests/" + suite) else "."
    out = [f"# tests/{suite}: {desc}\n",
           "# Every test program is named in [test] discover (registration is explicit,\n",
           "# as it was with xmake targets; structure tests check these entries).\n",
           "# build.mcpp defines CGPUI_SOURCE_ROOT for tests that read repository files.\n",
           "[package]\n", 'namespace = "cgpui"\n', f'name = "tests-{suite.replace("_", "-")}"\n',
           'version = "0.1.0"\n\n']
    if support or win_support or lin_support:
        out += [f"[targets.cgpui_tests_{suite}_support]\n", 'kind = "lib"\n\n']
    out += ["[dependencies]\n", deps_table(deps)]
    if win_deps:
        out += ["\n[target.'cfg(os = \"windows\")'.dependencies]\n", deps_table(win_deps)]
    if lin_deps:
        out += ["\n[target.'cfg(os = \"linux\")'.dependencies]\n", deps_table(lin_deps)]
    out += ["\n[build-dependencies]\n", 'cgpui.build-support = { path = "../../tools/mcpp" }\n',
            "\n[build]\n", toml_list("sources", support)]
    if incs:
        out.append(toml_list("include_dirs", [rel(d) for d in incs]))
    if suite == "ui":
        pass
    for os_name, extra_src, extra_inc, extra_def in (
            ("windows", win_support, win_incs, []),
            ("linux", lin_support, lin_incs, ["CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND"] if suite == "ui" else [])):
        if extra_src or extra_inc or extra_def:
            out.append(f"\n[target.'cfg(os = \"{os_name}\")'.build]\n")
            if extra_src:
                out.append(toml_list("sources", extra_src))
            if extra_inc:
                out.append(toml_list("include_dirs", [rel(d) for d in extra_inc]))
            if extra_def:
                out.append(toml_list("defines", extra_def))
    out += ["\n[test]\n", toml_list("discover", common)]
    if win:
        out += ["\n[target.'cfg(os = \"windows\")'.test]\n", toml_list("discover", win)]
    if lin:
        out += ["\n[target.'cfg(os = \"linux\")'.test]\n", toml_list("discover", lin)]
    directory = ROOT / "tests" / suite
    (directory / "mcpp.toml").write_text("".join(out), encoding="utf-8", newline="\n")
    body = "".join(PROGRAMS[p] for p in programs)
    program = ("// Configures this test suite; see tools/mcpp/cgpui_build.hpp.\n"
               "#include <cgpui_build.hpp>\n\nint main() {\n"
               "  cgpui_build::source_root_define(2);\n" + body + "}\n")
    (directory / "build.mcpp").write_text(program, encoding="utf-8", newline="\n")
    return len(common), len(win), len(lin)


def main():
    tests = collect()
    assert set(tests) == set(SUITES), sorted(set(tests) ^ set(SUITES))
    totals = [0, 0]
    for suite in sorted(tests):
        common, win, lin = write_suite(suite, tests[suite])
        totals[0] += common + win
        totals[1] += common + lin
        print(f"tests/{suite}: {common} both, {win} windows only, {lin} linux only")
    print(f"test programs: windows {totals[0]}, linux {totals[1]}")


if __name__ == "__main__":
    main()
