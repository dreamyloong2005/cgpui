# Acceptance: explicit-transparent clear and ordinary DX12 investigation

Date: 2026-11-23 (Asia/Shanghai). Baseline `65eb6ae6`, branch `phase-v`.

## Delivered

The backend-neutral runtime now preserves explicit ordinary-window transparency
at frame clearing, not only at renderer creation. `runtime_window_backdrop.cpp`
uses the record's explicit chrome intent and the renderer's current
`supported && alpha_composited()` report. The root draw path now consumes that
same record policy as secondary windows. No new public API or window mode.

Existing Popup clear and applied-material behavior are unchanged. A window
created only for a material clears opaque after `set_backdrop(none)`, even though
the renderer retains its alpha route and the descriptor retains its creation
request. Explicit transparency remains transparent only when its branch has an
actual supported pre-/post-alpha report. Inherit/opaque/unknown/unsupported do
not satisfy that branch. Live capability-change tests use backdrop `none`;
this is not new full material renegotiation after recovery.

Runtime regressions exercise public root/secondary creation, actual runtime
redraw dispatch and recording `RenderFrame::clear`/`present`, plus public
`Window::set_backdrop`. They are not native desktop-alpha tests. Module caps were
not increased: the runtime leaf is 53 lines and its private header 33.

Ordinary DX12 transparency/native HMENU coexistence remains **unfinished and
unsupported**, not silently degraded. Eleven native recipes across Intel, NVIDIA
and WARP yielded correct GPU captures, and eight recipes per adapter displayed
desktop alpha, but none preserved reference native menu drawing simultaneously.
Some menus disappear/are blank; others retain text but change the standard gray
glyphs `(160,160,160)` to `(160,255,160)` over the controlled green backing.
This rules out shipping the tested recipes unchanged, not every possible host
design. See `docs/dx12-ordinary-transparency-probe.md` for the detailed table.

The probe source/registration is only on the local throwaway branch
`codex/prototype-dx12-ordinary-20261123`, commit
`3cde7234d70ab341b10be8a7b4ae7f072e636f10`, and in the evidence directory. The
production suite contains no probe whose exit 0 merely records negative results.

## Verification

Evidence root: `D:/Dev/shots/cgpui-dx12-ordinary-20261123/`.
Root ran all builds/execution serially, MCPP_JOBS=4 for no-run builds and 1 for
execution. `verify-final.ps1` exited 0.

| Check | Result | Evidence |
| --- | --- | --- |
| Public runtime initial transparency RED | expected exit 1, four failing combinations | `runtime-transparent-red.log` |
| Public runtime dynamic backdrop RED | expected exit 1, four failing combinations | `runtime-dynamic-red.log` |
| Runtime GREEN incl. expanded live-capability checks | 2 passed | `runtime-clear-green.log` |
| Focused architecture GREEN | 1 passed | `structure-green.log` |
| Frozen Windows full suite | 719 passed, 0 failed | `windows-final-test.log` |
| Architecture / UI / native DX12 within that suite | 204 / 108 / 53 passed | same log |
| Windows all-member libraries/examples build | PASS | `windows-all-build.log` |
| Frozen WSL full suite | 639 passed, 0 failed | `wsl-full-test.log` |
| WSL all-member libraries/examples build | PASS | `wsl-all-build.log` |
| Scoped runtime ASan | 2 passed, 0 failed | `asan-runtime-test.log` |
| Native topology experiment, debug requested | 11 cases on each of 3 verified adapters; setup errors 0; combined candidates 0 | `probe-v3-{warp,0,1}.log` |
| Native menu RGB comparison | 553 reference glyph pixels become green-tinted in the sampled candidates on every adapter | `compare-menu-rgb.py`, `menu-rgb-comparison.json` |
| Windows source match | 2731 raw-byte SHA256 matches, no added/removed paths | `source-match.json` |
| WSL source match | 2731 manifest paths match after the documented text normalization | `wsl-source-match.json` |

The initial test harness had an ambiguous `View` name; it was corrected before
the recorded behavioral RED runs. The shader/backend implementation, SDK API and
packages did not change. This slice did not repeat installed-SDK checks or the
previous milestone's all-native ASan/GPU-validation matrix; the full Windows run
does include the existing 53 native DX12 programs. The topology experiment's
adapter matrix is not a complete backend-acceptance matrix.

The source manifest excludes build outputs, `.planning/` closeout notes and the
user's pre-existing `.vscode/`. WSL uses only the dedicated validation mirror
`/root/cgpui-logic-cleanup-20261123`; the user's Linux checkout and the design
checkout are untouched. No disk cleanup was performed despite low-space build
warnings. Only dated planning notes changed after source freeze.

## Review and remaining gates

### Standards

Zero hard violations; zero heuristic smell findings. Policy stays in its private
leaf, the root is only a caller, tests share a scoped fixture, and structure tests
protect intent/capability ownership and both runtime call sites.

### Spec

Zero actionable findings in the code or follow-up documentation/evidence review.
The independent review verified the prototype branch contains only the probe and
manifest changes, and that its source matches the captured v3 experiment.

Remaining: a native-menu-safe ordinary alpha host, safe opaque/transparent route
transition and rollback, dynamic menu/client geometry, native menu interaction,
input/DPI/backdrop/lifetime/recovery coverage for that new host, cross-thread
recovery and the overall parity/performance/release gates. No native macOS claim.
Vulkan remains default including auto, Vulkan Popup remains enabled with
failure-only fallback, and explicit DX12 never silently changes backend.

Local commit only, no push or release. This accepts the bounded runtime fix and
investigation, **not completion of the DX12 backend**.
