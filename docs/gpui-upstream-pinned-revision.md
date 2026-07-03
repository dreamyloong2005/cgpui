# GPUI Upstream Pinned Revision

CGPUI complete-replication work is pinned to this upstream GPUI snapshot:

- Repository: `https://github.com/zed-industries/zed`
- Commit: `5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0`
- GPUI source tree:
  `https://github.com/zed-industries/zed/tree/5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0/crates/gpui`
- GPUI raw crate root:
  `https://raw.githubusercontent.com/zed-industries/zed/5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0/crates/gpui/src/gpui.rs`
- GPUI README:
  `https://raw.githubusercontent.com/zed-industries/zed/5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0/crates/gpui/README.md`
- Context docs:
  `https://raw.githubusercontent.com/zed-industries/zed/5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0/crates/gpui/docs/contexts.md`
- Key dispatch docs:
  `https://raw.githubusercontent.com/zed-industries/zed/5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0/crates/gpui/docs/key_dispatch.md`

## Crate Versions

- `gpui = 0.2.2`
- `gpui_platform = 0.1.0`

## Snapshot Files

Phase A treats these upstream files as the initial parity source set:

- `README.md`
- `Cargo.toml`
- `src/gpui.rs`
- `docs/contexts.md`
- `docs/key_dispatch.md`
- `examples/README.md`
- `examples/hello_world.rs`
- `examples/animation.rs`
- `examples/gif_viewer.rs`
- `examples/image/image.rs`
- `examples/input.rs`
- `examples/opacity.rs`
- `examples/set_menus.rs`
- `examples/shadow.rs`
- `examples/svg/svg.rs`
- `examples/text_wrapper.rs`
- `examples/uniform_list.rs`
- `examples/window.rs`
- `examples/window_positioning.rs`
- `examples/window_shadow.rs`

## Platform Interpretation

- Windows parity target: Win32 + Vulkan in CGPUI.
- Linux parity target: Wayland + Vulkan in CGPUI.
- macOS parity target: Cocoa + Metal after Windows/Linux core stabilization.
- X11 is recorded in the ledger as `Deferred`, not active work. It only
  becomes required if the user explicitly chooses strict upstream Linux backend
  matrix parity.

## Refresh Procedure

To refresh this pin, update the commit hash first, then rerun:

```powershell
python tools\gpui_parity\extract_upstream_symbols.py --output build\gpui_parity_snapshot.json
```

After refreshing, update `docs/gpui-complete-parity-ledger.md` and
`docs/gpui-complete-parity-ledger.json` in the same change. Do not silently
move the pin without updating both files and the API parity tests.
