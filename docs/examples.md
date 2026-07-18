# Examples

## Canonical Targets

`scripts/ci/example-targets.txt` is the build/run inventory for the 21 public
C++ example targets. Build one target with:

```bash
xmake build -j 1 api_parity_hello_world
xmake run api_parity_hello_world
```

All examples use public headers. Architecture guards reject private `src/`,
runtime-internal, renderer-internal, and platform-internal includes.

## Official GPUI Mapping

`docs/gpui-official-example-matrix.json` maps every example in the pinned GPUI
revision to one or more canonical CGPUI targets. It records Windows, Wayland,
X11, and macOS compile/smoke status plus focused pixel and input evidence.

Some mappings are intentional C++ adaptations:

- `move_entity_between_windows`: runtime-owned entities and explicit shared
  state handles.
- `pattern`: the public SVG source/raster pipeline.
- `grid_layout`: nested horizontal and vertical flex containers.
- `mouse_pressure`: pointer movement, buttons, capture, and drag; no native
  pressure claim.

## Desktop Demo

`hello_window` is the native application smoke target. Environment-driven
first-frame, resize, close, and complete interaction modes are documented in
[Getting Started](getting-started.md).

## Example Matrices

- Windows: `scripts/ci/windows-example-smoke.ps1`
- Wayland: `scripts/ci/linux-example-smoke.sh`
- X11: `scripts/ci/linux-x11-example-smoke.sh`
- Isolated Xvfb wrapper: `scripts/ci/linux-x11-xvfb-example-smoke.sh`
- macOS: `scripts/ci/macos-example-smoke.sh`

The scripts build targets serially, run noninteractive public entrypoints, and
execute registered/native smoke modes. macOS arm64 and x86_64 execution is
required and remains pending final native CI verification.
