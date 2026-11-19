# SDK packages (C10)

CGPUI ships as a static-library SDK per platform and build mode, built
only from a clean committed tree. CodeStudio Next consumes it (C++23, MSVC
on Windows).

| Package | Platform | Runtime / toolchain |
|---|---|---|
| `cgpui-sdk-windows-x64-debug` | Windows x64, Win32 + Vulkan | MSVC, `/MDd` |
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
- the same name plus `.sha256`, in `sha256sum` format.

Steps:

1. **Clean tree.** `sdk-package.py check-clean` reads
   `git status --porcelain --untracked-files=all`. On Windows, tracked
   changes are refused. Untracked files (here `.vscode/`, `artifacts/`) are
   tolerated because the build runs in a temporary
   `git worktree add --detach` of HEAD under the output root, which only
   holds committed files. The worktree is checked again, with untracked
   files refused too, and is removed afterwards (`-KeepWorktree` keeps it).
   On Linux the clone is built in place and must be fully clean.
2. **Build.** The existing `windows-package.ps1` / `linux-package.sh`
   configure and build `cgpui_app` and `hello_window` in a hermetic xmake
   dependency directory. On Windows the SDK passes `-Runtimes MDd` or `MD`
   and the platform explicitly (`-p windows -a x64`). The wrapper then
   builds `example_workbench` verbosely; its link line is the consumer link
   order.
3. **Finalize.** `sdk-package.py finalize` classifies each library on that
   link line: a CGPUI library from the build directory, a static library
   from the xmake package store (copied into `lib/`), or a system library.
   On Windows it reads the `/DEFAULTLIB` directives of every library and
   refuses one compiled for a different CRT. It writes `manifest.json`,
   `lib/cmake/cgpui/cgpuiConfig.cmake`, `cgpuiConfigVersion.cmake`,
   `README.md` and copies the consumer sample. After the build only
   `xmake-requires.lock` may differ from HEAD; on Linux the committed file
   is restored.
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
lib/*.pdb                         Windows debug symbols of the libraries
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

`schema_version` 2 extends the Phase G package manifest. Fields:

| Field | Meaning |
|---|---|
| `package`, `version` | package name; `set_version` from `xmake.lua` |
| `git_commit`, `dirty`, `source_check` | HEAD of the packaged tree; `false` only after the clean-tree checks |
| `xmake_requires_lock_rewritten_by_build` | xmake rewrote the lock file while resolving packages |
| `platform`, `architecture`, `mode`, `cxx_standard` | `windows`/`linux`, `x64`/`x86_64`, `debug`/`release`, `c++23` |
| `compiler` | id, path, version (Windows: `cl` version and MSVC toolset) |
| `msvc_runtime`, `vulkan_sdk_version` | Windows only |
| `system_packages` | Linux only: `pacman -Q` versions of harfbuzz, freetype2, fontconfig, wayland, libxkbcommon, dbus, vulkan-icd-loader, libxml2 |
| `libraries`, `link_names`, `library_origins` | packaged libraries in link order and where each came from (`cgpui` or `xmake package <name> <version>`) |
| `xmake_dependencies` | from `xmake-requires.lock` as committed at HEAD (`git show HEAD:xmake-requires.lock`, never the working copy or the effective git / xmake configuration, so a package mirror cannot leak in): `lock_sha256`, `lock_section` and, per package pinned for the platform, name, version, `lock_key`, `package_hash`, `repo_url`, `repo_branch`, `repo_commit`, `bundled` (copied into `lib/`) |
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
`VULKAN_SDK`), and a static CRT (`CMAKE_MSVC_RUNTIME_LIBRARY`
`MultiThreaded` / `MultiThreadedDebug`) is rejected at configure time. Use
the debug package for Debug builds (`/MDd`, `_ITERATOR_DEBUG_LEVEL=2`) and
the release package for Release builds; mixing them fails at link time
(LNK2038). The version file accepts requests up to the package version
with the same major version (same minor while the major is 0), 64-bit
only.

## Consuming with xmake

`share/cgpui/sdk_consumer/xmake.lua` reads `manifest.json`, so it follows
the package without edits:

```lua
target("app")
    set_kind("binary")
    set_languages("c++23")
    add_files("main.cpp")
    if is_plat("windows") then
        set_runtimes(is_mode("debug") and "MDd" or "MD")
    end
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

- `-j 1` (kept from `windows-package.ps1` / `linux-package.sh`): a Windows
  package takes about 10 minutes, a Linux one about 4.
- `xmake-requires.lock` is rewritten by the xmake package resolution
  during the Windows build in the worktree (recorded in
  `xmake_requires_lock_rewritten_by_build`); sources are unaffected.
- Debug and release packages are not interchangeable on Windows (CRT and
  `_ITERATOR_DEBUG_LEVEL`).
- Linux packages link the system libraries of the build machine (Arch
  Linux, versions in `system_packages`); other distributions are not
  tested.
- The consumer sample is verified on Windows 11 (NVIDIA) and WSLg (Mesa
  dzn, a non-conformant Vulkan driver); a native Wayland compositor is not
  tested.
- No signing; integrity is the per-file and archive SHA-256.
- Package resolution clones xmake-repo and (on Windows) build-artifacts in
  a fresh xmake global directory each run. On a connection that drops long
  transfers, point xmake at local mirrors with `XMAKE_MAIN_REPO` /
  `XMAKE_BINARY_REPO`. The manifest still records the URLs and commits
  from the committed lock, so they match a package built without mirrors.
