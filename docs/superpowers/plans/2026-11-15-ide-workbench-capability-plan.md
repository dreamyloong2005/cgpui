# IDE Workbench Capability Plan (Windows first)

Date: 2026-11-15
Consumer: CodeStudio Next (dual-pane editor + agent workbench, glass
material, light/dark following the system, mixed-DPI monitors).

These are framework capabilities. Product logic (document buffers, rope
storage, syntax highlighting rules, agent protocol) stays in the consumer.

## Build conventions for this plan

- `xmake.lua` is held at <= 4340 lines by older structure tests. New targets,
  file additions to existing targets, and tests go in focused modules under
  `build/xmake/capabilities/*.lua`, which the root includes with one glob.
  The architecture/header matrix test scans that directory.
- Every new `tests/architecture` or `tests/header_cleanliness` source must be
  listed in `scripts/ci/architecture-header-targets.txt`.
- Follow AGENTS.md: leaf headers, private `*_internal.hpp`, focused `.cpp`
  files, and structure tests with line limits for each new module.
- Gate per capability: full Windows debug `xmake build -a` and `xmake test`
  green, `git diff --check` clean, then one commit.
- Behavior that needs a real window, GPU, or human observation is recorded as
  unverified until it has been observed.

## Capabilities

| ID | Capability | Status |
|---|---|---|
| C0 | Commit DirectWrite glyph rasterization; capability build module | done (`17fdc168`, this commit) |
| C1 | Per-Monitor V2 DPI awareness declared by the framework at startup | planned |
| C2 | System appearance: light/dark, accent color, transparency-effects and reduced-motion settings, change notifications; immersive dark title bar | planned |
| C3 | Window backdrop material: DWM Mica / Acrylic / tabbed, transparent Vulkan composition, graceful opaque fallback | planned |
| C4 | Custom title bar: client-area extension, hit-test regions (caption, min/max/close, resize borders), Snap Layout on maximize button | planned |
| C5 | Box shadow rendering in Vulkan (blurred rounded-rect shadow, spread, offset, inset excluded) | planned |
| C6 | In-app backdrop blur for overlays (offscreen copy + separable blur, clipped to rounded rect) | planned |
| C7 | DirectWrite shaping and real metrics: glyph ids, advances, clusters, font fallback runs, line metrics; caret/hit-test/selection consume shaped advances | planned |
| C8 | IME composition detail: cursor position and clause attributes (IMM32 `GCS_CURSORPOS`, `GCS_COMPATTR`) | planned |
| C9 | Workbench widgets: splitter, tabs, tree view, tooltip, context menu / popover | planned |
| C10 | SDK package refresh from a committed tree, with manifest and consumer instructions | planned |

## Order and rationale

C1 and C2 are small and unblock correct scaling and theming. C3 and C4 shape
the window itself. C5 and C6 complete in-app material. C7 is the largest item
and the main text-correctness blocker for an editor; it follows the window
work so visual checks can use the final chrome. C8 builds on C7 metrics. C9
uses all of the above. C10 packages the result.
