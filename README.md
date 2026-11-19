# CGPUI

CGPUI is a C++23-native desktop UI framework inspired by GPUI. It provides a
declarative public authoring API, runtime-owned entities and views, deterministic
test support, production desktop platform adapters, and GPU rendering backends
without a Rust FFI layer.

## Supported Desktop Targets

- Windows: Win32 + Vulkan
- Linux: Wayland + Vulkan
- Linux: X11/XCB + Vulkan
- macOS: Cocoa + Metal

The current Phase J task excludes new macOS execution. The completed Phase H
macOS evidence remains recorded in the parity ledger.

## Quick Start

CGPUI builds with [mcpp](https://github.com/mcpp-community/mcpp); the
wrappers in `scripts/dev/` set its environment:

```bash
scripts/dev/build.sh -p examples/hello_window      # Windows: scripts/dev/build.ps1
mcpp run -p examples/hello_window hello_window     # builds, then starts it
```

On Windows run `mcpp` with `VSLANG=1033` (see
[Getting Started](docs/getting-started.md)). On
Linux, select a backend explicitly when both display environments exist:

```bash
CGPUI_LINUX_BACKEND=wayland mcpp run -p examples/hello_window hello_window
CGPUI_LINUX_BACKEND=x11 mcpp run -p examples/hello_window hello_window
```

See [Getting Started](docs/getting-started.md) for toolchain setup, smoke modes,
and test commands.

## Documentation

- [Getting Started](docs/getting-started.md)
- [Architecture](docs/architecture.md)
- [Platform Dependencies](docs/platform-dependencies.md)
- [Examples](docs/examples.md)
- [GPUI to C++23 Migration](docs/gpui-to-cpp23-migration.md)
- [Non-goals](docs/non-goals.md)
- [Public Authoring Vocabulary](docs/gpui-public-authoring-vocabulary.md)
- [Parity Ledger](docs/gpui-complete-parity-ledger.md)

The machine-readable authority files are
`docs/gpui-complete-parity-ledger.json`,
`docs/gpui-upstream-snapshot.json`, and
`docs/gpui-official-example-matrix.json`.

## Verification

Build every member and run every test program (in parallel):

```bash
scripts/dev/build.sh        # Windows: scripts/dev/build.ps1
scripts/dev/test.sh         # Windows: scripts/dev/test.ps1
```

Platform packaging, example smoke, performance, and stress entrypoints live in
`scripts/ci/`. Generated packages and reports stay under repository-local
build or artifact directories.
