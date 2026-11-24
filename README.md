# CGPUI

CGPUI is a C++23-native desktop UI framework inspired by GPUI. It provides a
declarative public authoring API, runtime-owned entities and views, deterministic
test support, production desktop platform adapters, and GPU rendering backends
without a Rust FFI layer.

## Supported Desktop Targets

- Windows: Win32 + DX12, with checked initialization-only Vulkan fallback
- Linux: Wayland + Vulkan
- Linux: X11/XCB + Vulkan
- macOS: Cocoa + Metal on Apple Silicon (arm64) only

Windows automatic selection tries [DX12](docs/dx12-backend.md) first. Only a
classified unavailable-backend initialization failure with proven same-window
cleanup permits Vulkan fallback. Explicit backend selection never falls back,
and runtime failures never switch APIs. Ordinary-window transparency remains
[capability-bounded](docs/dx12-backend.md#bounded-ordinary-window-transparency);
this local default change is not a published SDK or all-driver certification.

The [macOS SDK](docs/macos-sdk.md) targets arm64; no Intel (x86_64) or
Universal SDK is provided. The minimum macOS is 26.0 and CI runs on
`macos-26` runners. See [macOS validation](docs/macos-validation.md) for
recorded configurations and the remaining revalidation boundary.

## Quick Start

CGPUI builds with [mcpp](https://github.com/mcpp-community/mcpp); the
wrappers in `scripts/dev/` set its environment:

```bash
scripts/dev/build.sh -p examples/hello_window      # Windows: scripts/dev/build.ps1
mcpp run -p examples/hello_window hello_window     # builds, then starts it
```

See [Getting Started](docs/getting-started.md) for the environment. On
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

Build every member and run every test program:

```bash
scripts/dev/build.sh        # Windows: scripts/dev/build.ps1
scripts/dev/test.sh         # Windows: scripts/dev/test.ps1
```

On Windows, run native window tests serially (`$env:MCPP_JOBS = '1'` before
`scripts/dev/test.ps1`); no-run builds may use multiple workers.

Platform packaging, example smoke, performance, and stress entrypoints live in
`scripts/ci/`. Generated packages and reports stay under repository-local
build or artifact directories.
