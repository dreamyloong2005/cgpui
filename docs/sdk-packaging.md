# SDK packages (C10)

CGPUI ships as a static-library SDK per platform and build mode, built
only from a clean committed tree. CodeStudio Next consumes it (C++23, MSVC
on Windows).

| Package | Platform | Runtime / toolchain |
|---|---|---|
| `cgpui-sdk-windows-x64-debug` | Windows x64, Win32 + Vulkan | MSVC, `/MD` (no optimization, `/Z7`) |
| `cgpui-sdk-windows-x64-release` | Windows x64, Win32 + Vulkan | MSVC, `/MD` |
| `cgpui-sdk-linux-x86_64-debug` | Linux x86_64, Wayland (+ X11) + Vulkan | GCC, libstdc++ |
| `cgpui-sdk-linux-x86_64-release` | Linux x86_64, Wayland (+ X11) + Vulkan | GCC, libstdc++ |

## Building the packages

Windows (PowerShell, from the repository root):

```powershell
scripts/ci/windows-sdk-package.ps1 -Mode Debug
scripts/ci/windows-sdk-package.ps1 -Mode Release
```

Linux (from the repository root of a clean clone):

```sh
scripts/ci/linux-sdk-package.sh debug
scripts/ci/linux-sdk-package.sh release
```

Each run writes, under the ignored `build/release/ide-workbench/`:

- `<platform>-<mode>/`: the extracted package;
- `cgpui-sdk-<platform>-<arch>-<mode>.zip` (Windows) or `.tar.gz` (Linux);
- the same name plus `.sha256`, in `sha256sum` format with an LF line
  ending on both platforms (`sha256sum -c` reads it unchanged).

Steps:

1. **Clean tree.** `sdk-package.py check-clean` reads
   `git status --porcelain --untracked-files=all`. On Windows, tracked
   changes are refused. Untracked files (here `.vscode/`, `artifacts/`) are
   tolerated because the build runs in a temporary
   `git worktree add --detach` of HEAD under the output root, which only
   holds committed files. The worktree is checked again, with untracked
   files refused too, and is removed afterwards (`-KeepWorktree` keeps it).
   On Linux the clone is built in place and must be fully clean.
2. **Build.** `windows-package.ps1` / `linux-package.sh` write a packaging
   workspace under the output root (`sdk-package.py workspace`: the root
   `mcpp.toml`'s library and example members, read from there, with the
   package's C++ runtime: the CRT is one setting for the whole graph) and
   build every member with mcpp, in parallel, with the `dev` (debug) or
   `release` profile. On Windows the SDK passes `-Runtimes MD`
   (`cxx_runtime = "toolchain-coupled"`) for both modes: mcpp has no debug
   CRT. On Linux the system libstdc++ is used (`host-coupled`).
3. **Finalize.** `sdk-package.py finalize` takes the link order from the
   `example_workbench` dependency graph (each library before those it
   depends on, read from the members' `mcpp.toml`), copies the CGPUI and
   vendored (lunasvg, plutovg) static libraries from the build directory,
   and takes the system libraries from `example_workbench`'s link line in
   mcpp's `build.ninja`. On Windows it reads the `/DEFAULTLIB` directives of
   every library and refuses one compiled for a different CRT. It writes
   `manifest.json`, `lib/cmake/cgpui/cgpuiConfig.cmake`,
   `cgpuiConfigVersion.cmake`, `README.md` and copies the consumer sample.
   The tree must still be clean afterwards.
4. **Archive.** `create-release-artifact.py --mode <mode>` (the Phase J
   archiver) writes `release-manifest.json` and a deterministic archive
   (sorted paths, fixed mtime `SOURCE_DATE_EPOCH=1704067200`, modes 0644 /
   0755 for `bin/`). The Linux intermediate build output is deleted
   (`CGPUI_SDK_KEEP_BUILD=1` keeps it).

## Package layout

```text
include/cgpui/...                 public headers
src/ui/*_internal.hpp             private headers the public ones include
lib/*.lib | lib/*.a               CGPUI + bundled static libraries
lib/cmake/cgpui/cgpuiConfig.cmake, cgpuiConfigVersion.cmake
bin/hello_window, bin/example_workbench (.exe on Windows)
share/cgpui/sdk_consumer/         sample: main.cpp, CMakeLists.txt, xmake.lua
README.md, manifest.json, release-manifest.json
```

`include/cgpui/ui/window_runtime.hpp` declares `WindowRuntime`'s private
members by including `../../../src/ui/window_runtime_internal.hpp`, which
pulls in 15 more `src/ui/*_internal.hpp` headers. A package with only
`include/` therefore fails to compile any consumer (found by the consumer
smoke test: C1083 on `window_runtime_internal.hpp`). `sdk-package.py`
copies that header closure to the same relative path and lists it in
`private_headers`. Moving those members behind a pimpl would remove the
need; it is not done here.

Shaders are compiled to SPIR-V at development time and embedded in
`cgpui_renderer_vulkan`; there are no separate shader files.

## manifest.json

`schema_version` 3 (the mcpp build; 2 was the xmake build) extends the Phase G package manifest. Fields:

| Field | Meaning |
|---|---|
| `package`, `version` | package name; `version` of `packages/core/mcpp.toml` |
| `git_commit`, `dirty`, `source_check` | HEAD of the packaged tree; `false` only after the clean-tree checks |
| `build_system` | mcpp `version` and `path`, the workspace `toolchain`, `cxx_runtime` and `profile` |
| `platform`, `architecture`, `mode`, `cxx_standard` | `windows`/`linux`, `x64`/`x86_64`, `debug`/`release`, `c++23` |
| `compiler` | id, path, version (Windows: `cl` version and MSVC toolset) |
| `msvc_runtime`, `vulkan_sdk_version` | Windows only |
| `system_packages` | Linux only: `pacman -Q` versions of harfbuzz, freetype2, fontconfig, wayland, libxkbcommon, dbus, vulkan-icd-loader, libxml2 |
| `libraries`, `link_names`, `library_origins` | packaged libraries in link order and where each came from (`cgpui` or `vendored <name> <version>`) |
| `vendored_dependencies` | per row of `third_party/VENDORED.md`: name, directory, version, upstream commit, license, source archive SHA-256, `tree_sha256` (SHA-256 over `<sha256>  <path>` lines of the directory's files as committed at HEAD, so checkout line endings cannot change it) and `bundled` (copied into `lib/`) |
| `system_libraries` | libraries the consumer links from the system, in order |
| `private_headers` | `src/ui` headers shipped because public headers include them |
| `executables`, `cmake_config`, `consumer_sample`, `shaders`, `documents` | package contents |
| `files` | path, size and SHA-256 of every file except `manifest.json` and `release-manifest.json` |

`release-manifest.json`, written by the archiver, lists every file
including `manifest.json` with its SHA-256 and archive mode.

## Consuming with CMake

```cmake
find_package(cgpui 0.1 REQUIRED CONFIG)
add_executable(app main.cpp)
target_link_libraries(app PRIVATE cgpui::cgpui)
```

Point CMake at the package with `-Dcgpui_DIR=<sdk>/lib/cmake/cgpui` or
`CMAKE_PREFIX_PATH=<sdk>`. Each library is an imported static target
(`cgpui::cgpui_core`, ..., `cgpui::lunasvg`); `cgpui::cgpui` carries the
include directory, `cxx_std_23` and `INTERFACE_LINK_LIBRARIES` with all of
them in link order followed by the system libraries. On Windows,
`vulkan-1` becomes `Vulkan::Vulkan` from `find_dependency(Vulkan)` (set
`VULKAN_SDK`). See [C++ runtime (Windows)](#c-runtime-windows) for the
CRT the config enforces. The version file accepts requests up to the package version
with the same major version (same minor while the major is 0), 64-bit
only.

## C++ runtime (Windows)

Both Windows packages, the debug one included, are built with the release
DLL CRT (`/MD`, `msvc_runtime` in `manifest.json`). mcpp's CRT model has no
debug axis: `/MTd` and `/MDd` are refused and the standard library module
and the link use the release CRT (mcpp docs, *20 — Toolchains*, "A
free-form CRT word is always a second statement"). A consumer must build
every configuration, Debug too, with the same CRT:

- CMake: `CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`. `cgpuiConfig.cmake`
  sets it when the consumer has not and stops with an error on any other
  value (CMake's Debug default is `MultiThreadedDebugDLL`).
- xmake: `set_runtimes("MD")` (the sample reads `msvc_runtime` from the
  manifest).

What a consumer's Debug build loses against a `/MDd` one: the
`_ITERATOR_DEBUG_LEVEL=2` checked iterators of the MSVC STL (it is 0
here), the debug heap (`_malloc_dbg`, guard bytes, fill patterns) and the
`_CrtSetDbgFlag` / `_CrtDumpMemoryLeaks` leak report. CGPUI covers those
classes of bugs by running its test suite under AddressSanitizer
([Getting Started](getting-started.md#7-addresssanitizer)); a consumer
can do the same with `/fsanitize=address`, which works with `/MD`.

## Debug information (Windows)

mcpp compiles `debug = true` with `/Z7` (it refuses `/Zi`), so the debug
package's `.lib` files carry their debug information in each object and
the package ships no `.pdb` files (the xmake package had 7, one per
library). For a static library this is complete: the consumer's linker
copies the information into the consumer's PDB. Checked on the debug
package of `0006db52`: the CMake sample built in Debug links
`sdk_consumer.pdb` (82 MB) whose module list (`llvm-pdbutil dump
--modules`) holds 374 objects of the 7 `cgpui_*.lib`, with line tables for
`src/**/*.cpp` (`dump --l`), functions (`S_GPROC32`
`cgpui::vulkan_image_texture_format`, ...), locals (`S_REGREL32`) and
27,613 `cgpui::` type records. Source paths point at the packaging
worktree (`build/release/ide-workbench/worktree-debug/`); map them to a
checkout in the debugger (Visual Studio: *Debug Source Files*).

## Consuming with xmake

Consumers may use xmake; CGPUI itself builds with mcpp.

`share/cgpui/sdk_consumer/xmake.lua` reads `manifest.json`, so it follows
the package without edits:

```lua
target("app")
    set_kind("binary")
    set_languages("c++23")
    add_files("main.cpp")
    on_load(function (target)
        import("core.base.json")
        local sdk = os.getenv("CGPUI_SDK")
        local manifest = json.loadfile(path.join(sdk, "manifest.json"))
        target:add("includedirs", path.join(sdk, "include"))
        target:add("linkdirs", path.join(sdk, "lib"))
        for _, name in ipairs(manifest.link_names) do
            target:add("links", name)
        end
        for _, name in ipairs(manifest.system_libraries) do
            target:add("syslinks", name)
        end
        if manifest.platform == "windows" then
            target:set("runtimes", manifest.msvc_runtime)
            target:add("linkdirs", path.join(os.getenv("VULKAN_SDK"), "Lib"))
        end
    end)
```

The package `README.md` also has a static snippet with the libraries
spelled out.

## Replaces

`build/native-text/windows/package` (Phase C7 era) was built from a tree
with uncommitted native-text changes
(`source_has_uncommitted_native_text_changes: true`), had no bundled
lunasvg / plutovg, no private headers and no CMake config. Use the
packages above instead.

## Limits

- Builds are parallel: a cold Windows package takes about 2 minutes.
- The Windows debug package is 1.4-5x larger than the xmake one: its
  debug information is in the libraries (`/Z7`), see
  [Debug information](#debug-information-windows).
- Linux packages link the system libraries of the build machine (Arch
  Linux, versions in `system_packages`); other distributions are not
  tested.
- The consumer sample is verified on Windows 11 (NVIDIA) and WSLg (Mesa
  dzn, a non-conformant Vulkan driver); a native Wayland compositor is not
  tested.
- No signing; integrity is the per-file and archive SHA-256.
- Nothing is downloaded: lunasvg and plutovg are vendored, the toolchain
  and system libraries are the build machine's.
