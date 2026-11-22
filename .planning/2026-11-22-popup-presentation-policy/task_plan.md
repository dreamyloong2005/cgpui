# Popup presentation policy revision

## Goal
Implement the corrected DX12/window design: public window intent, private presentation mechanisms. The final user decision retains independent Windows Vulkan popups through the existing private composition path; do not force overlays by backend name. Use overlays only for unavailable capability or actual popup creation/placement failure. DX12 remains a subsequent implementation, not a completed backend in this slice.

## Constraints
- No public native/composition window mode. No removal of DComp until its replacement is verified.
- Preserve current Vulkan adapter selection, main-window transparency/backdrops, native menus, non-Windows behavior and .vscode.
- Keep DComp/Vulkan interop modules and regression tests. Runtime intersects real platform/renderer capabilities and attempts popups rather than imposing a Windows Vulkan ban.
- mcpp >=2026.11.21.1, scripts/dev entry points only. Serial build/test processes. No push or merge.
- Baseline phase-v HEAD 11f7eb76; design worktree dx12-design HEAD b513dc0c. Only .vscode was untracked in main checkout; prior research note in design checkout is ours.

## Phases
1. [complete] Update all owning plans and widget docs for the final user decision: Vulkan popups retained, failure-only fallback, no public presentation modes.
2. [complete] Add neutral renderer popup capability and compose it with platform support in focused runtime modules; protect direct popup entry points.
3. [complete] Final policy: Windows capability RED-to-GREEN, popup 9/9 and serial full 598/598; WSL popup 8/8, architecture 188/188, Vulkan surfaces 4/4. Default-parallel full-run focus failure is recorded separately, not concealed by weakening assertions.
4. [complete] Independent Standards/Spec reviews found no actionable issues. Exact evidence and validation limits are recorded in acceptance.md; verified changes are prepared for local commits. No merge or push.

## Test seams
Renderer capability; WindowRuntime popup query/open and direct window creation; Tooltip/PopoverMenu overlay behavior; architecture module ownership/public window options guard. Preserve low-level presenter tests, which do not claim app runtime support.

## Errors
Initial exploratory guessed paths were missing; use repository file inventory. mcpp host build-program cache referenced missing binaries; retry regenerated them without manual cleanup. A new test initially included renderer_frame.hpp directly, exposing existing non-self-contained report declarations; use the supported renderer.hpp aggregate like existing tests. Clean RED then confirmed only missing popup capability API.
