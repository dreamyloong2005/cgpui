# SDK packages (C10)

CGPUI ships as a static-library SDK per platform and build mode, built
only from a clean committed tree. CodeStudio Next consumes it (C++23, MSVC
on Windows).

| Package | Platform | Runtime / toolchain |
|---|---|---|
| `cgpui-sdk-windows-x64-debug` | Windows x64, Win32 + DX12 / Vulkan | MSVC, `/MD` (no optimization, `/Z7`) |
| `cgpui-sdk-windows-x64-release` | Windows x64, Win32 + DX12 / Vulkan | MSVC, `/MD` |
| `cgpui-sdk-linux-x86_64-debug` | Linux x86_64, Wayland (+ X11) + Vulkan | GCC, libstdc++ |
| `cgpui-sdk-linux-x86_64-release` | Linux x86_64, Wayland (+ X11) + Vulkan | GCC, libstdc++ |

macOS uses the separate **Phase J** archive format, not the C10 clean-tree
release pipeline above. On an Apple Silicon Mac with a native arm64 shell, mcpp,
Python 3.12+, CMake, and a Metal-capable graphical session, run
`MACOSX_DEPLOYMENT_TARGET=26.0 scripts/ci/macos-phase-j-release.sh arm64`.
It verifies the release archive and builds and runs its CMake consumer outside
the checkout. The archive, SHA-256 sidecar and content manifest are under
`build/phase-j-release/macos-arm64/artifacts/`. See
[using the macOS SDK](macos-sdk.md) for the consumer commands.

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
3. **Finalize.** `sdk-package.py finalize` first validates the staged public
   header closure and rejects any stale package-root `src/` directory. It
   takes the link order from the
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
lib/*.lib | lib/*.a               CGPUI + bundled static libraries
lib/cmake/cgpui/cgpuiConfig.cmake, cgpuiConfigVersion.cmake
bin/hello_window, bin/example_workbench (.exe on Windows)
share/cgpui/sdk_consumer/         sample: main.cpp, CMakeLists.txt, xmake.lua
README.md, manifest.json, release-manifest.json
```

Public headers close over `include/`; no private source headers are shipped.
`WindowRuntime` owns opaque state implemented in the library, rather than
including its storage definitions from `src/ui/`. This changes its binary
layout: rebuild the libraries and consumers together, even though existing
public method signatures remain source-compatible.

Before copying libraries, both SDK `finalize` and macOS `stage-macos` check
every quoted include in the staged headers, including conditional platform
branches. Resolution uses the current header's directory and then the SDK's
`include/` root. Missing headers, paths escaping that root, absolute include
paths and header symlinks are rejected. A leftover package-root `src/` from
an older SDK is also rejected, not silently reused or deleted; choose a fresh
output directory. System angle includes remain toolchain dependencies.

The fast packaging regression suite is
`python -B tests/ci/sdk_package_headers_test.py`. It does not replace the
external include-only compile and packaged-library link smoke tests. Current
working-tree validation is not a new clean committed SDK release; native
macOS validation must be performed on a Mac.

Shaders are written in Slang and compiled to SPIR-V at development time
(`scripts/dev/compile_shaders.py`); the SPIR-V is embedded in
`cgpui_renderer_vulkan`, and the package has no shader files. The Vulkan
renderer requires Vulkan 1.1 (SPIR-V 1.3).

Windows packages include the backend dispatcher and DX12 library, whose
rectangle/text/image/shadow/backdrop DXIL is embedded (no runtime shader compiler).
Automatic selection tries DX12 first; only a classified unavailable-backend
initialization failure with checked same-window cleanup permits Vulkan fallback.
Explicit selections never fall back and runtime failures never switch APIs. See
[DX12 selection and scope](dx12-backend.md#selection-and-testing).
`python -B tests/ci/dx12_package_graph_test.py` checks both the SDK graph and the
base Windows package's copied libraries so neither can omit the factory owner.

Newly built Windows SDKs do not statically import `vulkan-1.dll`. Only selecting
Vulkan initializes its private runtime loader; explicit DX12 selection and
successful automatic DX12 selection do not load Vulkan. Loading uses the safe
default search (application directory, System32 and directories registered with
`AddDllDirectory`), not the current
directory or ambient `PATH`. All 88 required exports must bind before the typed
table is published; existing optional version/device-extension queries remain
optional under their previous checks. Missing/damaged DLLs and missing required
exports return `renderer_initialization_failed`, with the DLL/Win32 error or
missing symbol. A failed explicit Vulkan selection never tries DX12; a failed
automatic Vulkan fallback retains both initialization causes in the returned
error and stops. Failed bindings are retryable; a successful binding retains the
DLL for process lifetime.

Building the repository or packaging these libraries still requires the Vulkan
SDK headers on Windows. An installed consumer needs no Vulkan SDK, Vulkan import
library or delay-load flags/hooks; a Vulkan runtime is needed only if Vulkan is
explicitly selected or reached through the checked automatic fallback. Linux
keeps its direct system Vulkan linkage. This describes the current packaging
code, not a claim that a new clean committed SDK release has been published.

## manifest.json

`schema_version` 4 removes the obsolete `private_headers` and
`private_headers_reason` fields. Version 3 was the first mcpp SDK schema;
version 2 was the xmake build. The separate macOS Phase J manifest advances
from version 1 to version 2 for the same public-header-only contract; its
build-metadata schema is unchanged. Windows/Linux SDK fields:

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
| `header_root` | `include/cgpui`; quoted dependencies must remain inside the SDK's `include/` tree |
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
them in link order followed by the system libraries. Current Windows packages
omit `vulkan-1` from `system_libraries`, so the config does not call
`find_dependency(Vulkan)` or require `VULKAN_SDK`. For legacy packages whose
manifest still lists `vulkan-1`, that entry becomes `Vulkan::Vulkan` and retains
`find_dependency(Vulkan)` (set `VULKAN_SDK`). See [C++ runtime (Windows)](#c-runtime-windows) for the
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
        local needs_vulkan_sdk = false
        for _, name in ipairs(manifest.system_libraries) do
            target:add("syslinks", name)
            if name == "vulkan-1" then needs_vulkan_sdk = true end
        end
        if manifest.platform == "windows" then
            target:set("runtimes", manifest.msvc_runtime)
            if needs_vulkan_sdk then
                local vulkan_sdk = os.getenv("VULKAN_SDK")
                assert(vulkan_sdk, "legacy SDK links vulkan-1; set VULKAN_SDK")
                target:add("linkdirs", path.join(vulkan_sdk, "Lib"))
            end
        end
    end)
```

The package `README.md` also has a static snippet with the libraries
spelled out. The Vulkan SDK link directory is added only for a legacy manifest
that explicitly requires `vulkan-1`, not merely because the platform is Windows.

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
