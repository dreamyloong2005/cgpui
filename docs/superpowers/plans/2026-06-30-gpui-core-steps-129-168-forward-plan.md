# GPUI Core Steps 129-168 Forward Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Plan the next 40 Windows/Linux GPUI-core slices after Step 128, moving from near-core API coverage toward a practical GPUI-like application framework.

**Architecture:** Keep the active Step 128 remainder inside the current
89-128 execution queue, then use Steps 129-168 to deepen context/entity
ergonomics, keyed element reconciliation, reusable widgets, text/font
rendering, diagnostics, and platform-backed Win32/Wayland behavior.
Windows and Linux continue to use Vulkan, Linux stays Wayland-first, and
macOS/Cocoa + Metal remains a readiness boundary for a later parity run.

**Tech Stack:** C++23, xmake, public header-cleanliness tests, deterministic UI/runtime tests, Win32/Vulkan, Wayland/Vulkan, WSL Arch Linux verification, and feature-worktree RED/GREEN execution.

---

## Current State

- Steps 89-124 are complete on `master`; the Step 124 behavior commit is
  `74ad787 feat: apply platform cursors`.
- Step 120 post-merge verification passed before the docs closeout: targeted
  tests 4/4, Windows full debug 29/29, and WSL Arch Linux full debug 26/26.
- Step 121 post-merge verification passed: targeted tests 3/3, Windows full
  debug 29/29, and WSL Arch Linux full debug 26/26. Its docs closeout is also
  committed on `master`.
- Step 122 post-merge verification passed: targeted tests 3/3, Windows full
  debug 29/29, and WSL Arch Linux full debug 26/26.
- Step 123 post-merge verification passed: targeted tests 3/3, Windows full
  debug 29/29, and WSL Arch Linux full debug 26/26.
- Step 124 post-merge verification passed: targeted Windows tests for built
  targets 2/2, Windows full debug 29/29, and WSL Arch Linux full debug 26/26.
- Step 125 is merged on `master` at
  `389b9fb feat: add win32 system clipboard`; post-merge targeted tests passed
  1/1, Windows full debug 29/29, and WSL Arch Linux full debug 26/26.
- Step 126 is merged on `master` at
  `ab464d5 feat: add wayland clipboard skeleton`; post-merge targeted tests
  passed 1/1, Windows full debug 29/29, and WSL Arch Linux full debug 26/26.
- Step 127 is merged and post-merge verified on Windows and WSL Arch Linux;
  Step 128 remains the active implementation gate before this follow-on plan.
  They are covered by the detailed execution plan in
  `docs/superpowers/plans/2026-06-30-gpui-core-steps-89-128-execution-plan.md`.
- This document is the follow-on plan for the next 40 steps after Step 128.
  Do not execute Step 129 until Step 128 is merged and verified unless the
  roadmap is explicitly reprioritized.
- The main worktree is on `master`; the known local-only untracked item is
  `.vscode/`.

## 2026-07-01 Back-40 Planning After Step 124 Merge

- Step 124, platform cursor application for Win32 and Wayland, is merged on
  `master` at `74ad787 feat: apply platform cursors`. RED failed on missing
  `PlatformWindow::set_cursor(...)`; GREEN connects runtime hover cursor state
  to a platform cursor API, maps Win32 system cursors, and adds a testable
  Wayland `wl_pointer.set_cursor` skeleton without cursor theme loading.
- Feature-worktree verification passed: targeted Windows and WSL tests,
  Windows full debug 29/29, and WSL Arch Linux full debug 26/26.
- Post-merge verification passed: targeted Windows tests for built targets 2/2,
  Windows full debug 29/29, and WSL Arch Linux full debug 26/26.
- The gate to Step 129 is now 4 implementation slices:
  Steps 125-128, followed by post-Step-128 targeted verification, Windows full
  debug, WSL Arch full debug, and clean `master` status.
- The effective distance through Step 168 is 44
  implementation slices plus the post-Step-128 verification gate and four band
  checkpoint reviews.

## 2026-07-01 Back-40 Planning After Step 125 Merge

- Step 125, Win32 system clipboard backend for text copy, cut, and paste, is
  merged on `master` at `389b9fb feat: add win32 system clipboard`.
- RED failed on Win32 system clipboard interop after a forced test rebuild;
  GREEN adds a `CF_UNICODETEXT`-backed UTF-8 Win32 clipboard while preserving
  deterministic `MemoryClipboard` behavior for runtime tests and non-Windows
  fallback.
- Post-merge verification passed: targeted Windows clipboard test 1/1,
  Windows full debug 29/29, and WSL Arch Linux full debug 26/26.
- The gate to Step 129 is now 3 implementation slices: Steps 126-128,
  followed by post-Step-128 targeted verification, Windows full debug, WSL Arch
  full debug, and clean `master` status.
- The effective distance through Step 168 is 43 implementation slices plus the
  post-Step-128 verification gate and four band checkpoint reviews.

## 2026-07-01 Back-40 Planning After Step 126 GREEN

Historical snapshot, superseded by the Step 126 merge refresh below.

- Step 126, Wayland system clipboard backend skeleton for text copy, cut, and
  paste, is implemented and feature-worktree verified in
  `.worktrees/wayland-clipboard-skeleton`.
- RED failed on missing Linux `WaylandClipboard`, `WaylandClipboardSupport`,
  and `WaylandClipboardOptions` APIs. GREEN adds a Linux-only Wayland
  clipboard skeleton with `unsupported`, `no_seat`, and `available` support
  states, keeps memory fallback read/write behavior for graceful unsupported
  and no-seat runtime copy/cut/paste, and makes Linux
  `create_platform_clipboard()` return the Wayland skeleton.
- Feature-worktree verification passed: targeted clipboard test 1/1 on Windows
  and WSL Arch Linux, Windows full debug 29/29, and WSL Arch Linux full debug
  26/26.
- Step 126 still needs feature commit, fast-forward merge to `master`,
  post-merge targeted/Windows/WSL verification, docs closeout, and cleanup.
- After the Step 126 merge, the gate to Step 129 is 2 implementation slices:
  Steps 127-128, followed by post-Step-128 targeted verification, Windows full
  debug, WSL Arch full debug, and clean `master` status.
- The effective distance through Step 168 after the Step 126 merge is 42
  implementation slices plus the post-Step-128 verification gate and four band
  checkpoint reviews.

## 2026-07-01 Back-40 Planning After Step 126 Merge

- Step 126, Wayland system clipboard backend skeleton for text copy, cut, and
  paste, is merged on `master` at
  `ab464d5 feat: add wayland clipboard skeleton`.
- RED failed on missing Linux `WaylandClipboard`, `WaylandClipboardSupport`,
  and `WaylandClipboardOptions` APIs. GREEN adds a Linux-only Wayland
  clipboard skeleton with `unsupported`, `no_seat`, and `available` support
  states, keeps memory fallback read/write behavior for graceful unsupported
  and no-seat runtime copy/cut/paste, and makes Linux
  `create_platform_clipboard()` return the Wayland skeleton.
- Post-merge verification passed: targeted clipboard test 1/1, Windows full
  debug 29/29, and WSL Arch Linux full debug 26/26.
- The gate to Step 129 is now 2 implementation slices: Steps 127-128,
  followed by post-Step-128 targeted verification, Windows full debug, WSL Arch
  full debug, and clean `master` status.
- The effective distance through Step 168 is 42 implementation slices plus the
  post-Step-128 verification gate and four band checkpoint reviews.

## 2026-07-01 Back-40 Planning After Step 127 GREEN

Historical snapshot, superseded by the Step 127 merge refresh below.

- Step 127, IME composition/candidate rectangle data from the focused text
  element, is implemented and feature-worktree verified in
  `.worktrees/ime-candidate-rect`.
- RED failed on missing public `ImeCandidateRect` and
  `focused_text_ime_rect()` APIs. GREEN adds `ImeCandidateRect`,
  `WindowRuntime::focused_text_ime_rect()`, and
  `WindowRuntimeContext::focused_text_ime_rect()`, deriving the candidate rect
  from the focused `TextElement` layout bounds, cursor byte offset,
  font-size-derived glyph width, and caret height.
- Feature-worktree verification passed: targeted tests 3/3, Windows full debug
  29/29, and WSL Arch Linux full debug 26/26.
- Step 127 still needs feature commit, fast-forward merge to `master`,
  post-merge targeted/Windows/WSL verification, docs closeout, and cleanup.
- After the Step 127 merge, the gate to Step 129 will be 1 implementation
  slice: Step 128, followed by post-Step-128 targeted verification, Windows
  full debug, WSL Arch full debug, and clean `master` status.
- The effective distance through Step 168 after the Step 127 merge will be 41
  implementation slices plus the post-Step-128 verification gate and four band
  checkpoint reviews.

## 2026-07-01 Back-40 Planning After Step 127 Merge

- Step 127, IME composition/candidate rectangle data from the focused text
  element, is merged on `master` at
  `80aadae feat: add focused text ime rect`.
- RED failed on missing public `ImeCandidateRect` and
  `focused_text_ime_rect()` APIs. GREEN adds `ImeCandidateRect`,
  `WindowRuntime::focused_text_ime_rect()`, and
  `WindowRuntimeContext::focused_text_ime_rect()`, deriving the candidate rect
  from the focused `TextElement` layout bounds, cursor byte offset,
  font-size-derived glyph width, and caret height.
- Post-merge verification passed: targeted tests 3/3, Windows full debug
  29/29, and WSL Arch Linux full debug 26/26.
- The gate to Step 129 is now 1 implementation slice: Step 128, followed by
  post-Step-128 targeted verification, Windows full debug, WSL Arch full debug,
  and clean `master` status.
- The effective distance through Step 168 is 41 implementation slices plus the
  post-Step-128 verification gate and four band checkpoint reviews.

## 2026-07-01 Back-40 Planning After Step 125 GREEN

Historical snapshot, superseded by the Step 125 merge refresh above.

- Step 125, Win32 system clipboard backend for text copy, cut, and paste, is
  implemented and feature-worktree verified in `.worktrees/win32-system-clipboard`.
  RED failed on Win32 system clipboard interop after a forced test rebuild;
  GREEN adds a `CF_UNICODETEXT`-backed UTF-8 Win32 clipboard while preserving
  deterministic `MemoryClipboard` behavior for runtime tests and non-Windows
  fallback.
- Feature-worktree verification passed: targeted Windows clipboard test 1/1,
  Windows full debug 29/29, and WSL Arch Linux full debug 26/26.
- Step 125 still needs feature commit, fast-forward merge to `master`,
  post-merge targeted/Windows/WSL verification, docs closeout, and cleanup.
- After the Step 125 merge, the gate to Step 129 is 3 implementation slices:
  Steps 126-128, followed by post-Step-128 targeted verification, Windows full
  debug, WSL Arch full debug, and clean `master` status.
- The effective distance through Step 168 after the Step 125 merge is 43
  implementation slices plus the post-Step-128 verification gate and four band
  checkpoint reviews.

## 2026-06-30 Back-40 Planning After Step 122 Merge

Historical snapshot, superseded by the Step 123 feature-worktree verification
refresh above.

This planning refresh follows the Step 122 merge at
`58561b1 feat: add font size style`. Step 122 is merged and post-merge verified
on Windows and WSL Arch Linux, so the active pre-back-40 path is now Steps
123-128 plus the post-Step-128 exit verification.

- Step 122 added a metadata-only `FontDescriptor`, `Style::font`,
  `Style::font_size`, style-overlay and builder shortcuts, deterministic
  font-size-derived `TextElement` metrics, and `TextPaint` font metadata.
- Step 122 intentionally did not add shaping, glyph cache ownership, platform
  font discovery, Vulkan text drawing, caret/selection metadata, clipboard,
  cursor, or IME behavior.
- The gate to Step 129 is 6 implementation slices: Steps 123-128, followed by
  post-Step-128 targeted verification, Windows full debug, WSL Arch full debug,
  and clean `master` status.
- The effective distance through Step 168 is 46 implementation slices plus the
  post-Step-128 verification gate and four band checkpoint reviews.

## 2026-06-30 Back-40 Planning After Step 121 Docs Closeout

Historical snapshot, superseded by the Step 122 merge refresh
above.

This planning refresh is anchored at `master` HEAD
`dc010b6 docs: mark step 121 merged`. It is a plan-only closeout: no Step 129
implementation work starts here, and the active implementation path remains
Step 122 through Step 128.

- The immediate execution branch is already open:
  `.worktrees/font-descriptor-font-size` on
  `codex/font-descriptor-font-size`. The next implementation action is to add
  the Step 122 RED tests for `FontDescriptor`, `font_size`, text layout metrics,
  and public header coverage.
- The gate to Step 129 is now exactly 7 implementation slices: Steps 122-128,
  followed by the post-Step-128 targeted verification, Windows full debug, WSL
  Arch full debug, and clean `master` status check.
- The后 40 步 are Steps 129-168. They are planned as four sequential 10-step
  bands, each with one product-shaped exit:
  context/entity/async, keyed widgets/style cascade, text/font/renderer
  maturity, and Windows/Wayland platform completion plus parity audit.
- The effective distance through Step 168 is 47 implementation slices: the 7
  remaining pre-back-40 slices plus the 40 follow-on slices, with four band
  checkpoint reviews after Steps 138, 148, 158, and 168.
- Step 168 remains an audit milestone. It should say what is implemented,
  partial, missing, and Mac/Metal-deferred; it should not claim full upstream
  GPUI parity.

Planning-level outcome for these 40 steps:

- After Step 138, public app code should be able to use context-shaped APIs for
  entities, globals, scoped actions, subscriptions, deferred callbacks, timers,
  async completions, batching, and diagnostics without direct runtime plumbing.
- After Step 148, common UI should be expressible through keyed elements,
  lifecycle/state, style classes/themes, cascade resolution, focus handles, and
  reusable button, label, text input, and scrollable list primitives.
- After Step 158, text and rendering should have backend-ready command
  metadata for fonts, shaping runs, glyph caches, Vulkan text handling,
  opacity, transforms, batching, HiDPI scale, snapshots, and unsupported-command
  diagnostics.
- After Step 168, Win32 and Wayland should have the platform hooks needed by
  the active Windows/Linux core API, and the parity document should become the
  truth source for the next milestone.

## 2026-06-30 Back-40 Planning After Step 121 Merge

Historical snapshot, superseded by the Step 121 docs-closeout refresh above.

This refresh is anchored at `master` HEAD
`cf180f4 feat: add text paint command`. Step 121 is now merged and post-merge
verified; the active gate before Step 129 is exactly Steps 122-128 plus the
post-Step-128 targeted, Windows, and WSL verification. The next implementation
action is Step 122, font descriptor and basic font-size style primitives.

- Finish the current gate first: Steps 122-128 close font-size metadata,
  caret/selection command metadata, cursor/clipboard/IME hooks, and the
  public-prelude demo rewrite.
- The effective distance to Step 129 is 7 implementation steps, Steps 122-128,
  plus post-Step-128 targeted, Windows, and WSL verification.
- The effective distance through Step 168 is 47 implementation steps plus the
  post-Step-128 verification gate and four band checkpoint reviews.
- Step 168 remains a milestone audit, not a parity claim.

## 2026-06-30 Back-40 Planning Refresh With Step 121 Active

Historical snapshot, superseded by the Step 121 merge refresh above.

This refresh is anchored at `master` HEAD
`45a8dad docs: mark step 120 merged`. The back 40 remain Steps 129-168 and
must not preempt the active Step 121 worktree or the rest of the Step 121-128
gate.

- Step 121 is already implemented and feature-worktree verified in
  `.worktrees/text-paint-command`; continue that branch through final targeted
  verification after docs edits, commit, merge, post-merge Windows/WSL
  verification, docs closeout, and cleanup before opening Step 122.
- Until Step 121 merges, the effective distance to Step 129 remains 8
  implementation steps, Steps 121-128, plus the post-Step-128 targeted,
  Windows full debug, and WSL Arch Linux verification on `master`.
- After Step 121 merges, the distance becomes 7 implementation steps,
  Steps 122-128, plus that same post-Step-128 verification gate.
- The back-40 planning output for this refresh is the execution contract below:
  branch slugs, first RED tests, targeted commands, step exit artifacts,
  explicit non-goals, and four band checkpoints after Steps 138, 148, 158, and
  168.
- No Step 129 worktree should be created until Step 128 has landed and the
  Step 129 start packet passes on a clean `master`.

## 2026-06-30 Back-40 Planning After Step 120 Merge

Historical snapshot, superseded by the Step 121-active refresh above.

This refresh is anchored at `master` HEAD
`9aba0e6 feat: honor vulkan solid rect clips`. Step 120 is now merged and
post-merge verified; the active gate before Step 129 is exactly Steps 121-128
plus the post-Step-128 targeted, Windows, and WSL verification. The next
implementation action is Step 121, text paint command metadata, not Step 129.

- Finish the current gate first: Steps 121-128 close text/caret/selection
  command metadata, cursor/clipboard/IME hooks, and the public-prelude demo
  rewrite.
- Then run Steps 129-168 as four 10-step bands: context/entity/async, keyed
  widgets/style cascade, text/font/renderer diagnostics, and Windows/Wayland
  platform closure plus the parity audit.
- Keep the same cadence for the back 40: feature worktree, RED test, GREEN
  implementation, targeted tests, Windows full debug, WSL Arch full debug,
  docs/progress update, commit, fast-forward merge, post-merge verification,
  and cleanup.
- The effective distance to Step 129 is 8 implementation steps, Steps 121-128,
  plus post-Step-128 targeted, Windows, and WSL verification.
- The effective distance through Step 168 is 48 implementation steps plus the
  post-Step-128 verification gate and four band checkpoint reviews.
- Step 168 is a milestone audit, not a parity victory lap. It should document
  implemented, partial, missing, and Mac/Metal-deferred areas with a
  Windows/Linux completion lens.

## 2026-06-30 Back-40 Planning After Step 119 Merge

Historical snapshot, superseded by the Step 120 merge refresh above.

This refresh is anchored at `master` HEAD
`0893600 feat: add rounded rect paint command`. Step 119 is now merged and
post-merge verified; the active gate before Step 129 is exactly Steps 120-128
plus the post-Step-128 targeted, Windows, and WSL verification. The next
implementation action is Step 120, Vulkan clip rect metadata for solid
rectangles, not Step 129.

Step 120 is now implemented and feature-worktree verified. Until it is merged
and post-merge verified, the current active gate is Step 120 merge/post-merge
verification plus Steps 121-128 and the post-Step-128 exit verification.

- Finish the current gate first: Steps 120-128 close Vulkan clip handling,
  text/caret/selection command metadata, cursor/clipboard/IME hooks, and the
  public-prelude demo rewrite.
- Then run Steps 129-168 as four 10-step bands: context/entity/async,
  keyed widgets/style cascade, text/font/renderer diagnostics, and
  Windows/Wayland platform closure plus the parity audit.
- Keep the same cadence for the back 40: feature worktree, RED test, GREEN
  implementation, targeted tests, Windows full debug, WSL Arch full debug,
  docs/progress update, commit, fast-forward merge, post-merge verification,
  and cleanup.
- From the Step 119 merge snapshot, the effective distance to Step 129 was 9
  implementation steps, Steps 120-128, plus post-Step-128 targeted, Windows,
  and WSL verification.
- After Step 120 merges, the effective distance to Step 129 becomes 8
  implementation steps, Steps 121-128, plus post-Step-128 targeted, Windows,
  and WSL verification. The effective distance through Step 168 becomes 48
  implementation steps plus those verification gates and the four band
  checkpoint reviews.
- Step 168 is a milestone audit, not a parity victory lap. It should document
  implemented, partial, missing, and Mac/Metal-deferred areas with a
  Windows/Linux completion lens.

## 2026-06-30 Back-40 Planning After Step 118 Docs Closeout

Historical snapshot, superseded by the Step 119 merge refresh above.

This refresh is anchored at `master` HEAD
`c0d38c7 docs: mark step 118 merged`. The Step 118 behavior commit is
`9dfc2e7 feat: add layer elevation z order`. This is the execution plan for
Steps 129-168 after the Step 128 gate, not a new active branch queue. The next
implementation action after Step 119 is merged and post-merge verified is Step
120, Vulkan clip rect metadata, unless the roadmap is explicitly reprioritized.

- Finish the current gate first: Steps 119-128 close render command depth,
  render command metadata, cursor/clipboard/IME hooks, and the public-prelude
  demo rewrite.
- Then run Steps 129-168 as four 10-step bands: context/entity/async,
  keyed widgets/style cascade, text/font/renderer diagnostics, and
  Windows/Wayland platform closure plus the parity audit.
- Keep the same cadence for the back 40: feature worktree, RED test, GREEN
  implementation, targeted tests, Windows full debug, WSL Arch full debug,
  docs/progress update, commit, fast-forward merge, post-merge verification,
  and cleanup.
- From that Step 118 docs-closeout snapshot, after Step 119 merged the
  effective distance to Step 129 would become 9 implementation steps, Steps
  120-128, plus post-Step-128 targeted, Windows, and WSL verification.
- The main worktree currently has no tracked or staged changes; `.vscode/`
  remains the known local-only untracked item.
- Step 168 is a milestone audit, not a parity victory lap. It should document
  implemented, partial, missing, and Mac/Metal-deferred areas with a
  Windows/Linux completion lens.

## Back-40 Delivery Shape

The后 40 步 should be treated as a product hardening queue, not merely an API
inventory. Each 10-step band should leave the Windows/Linux core more usable
than it was at entry:

- Steps 129-138 make application state and asynchronous work authorable through
  context-shaped APIs instead of direct runtime plumbing.
- Steps 139-148 make reusable UI realistic by pairing keyed reconciliation,
  element lifecycle/state, style cascade, focus handles, and first widgets.
- Steps 149-158 make rendering contracts backend-ready by turning text, glyph,
  opacity, transform, HiDPI, batching, and fallback diagnostics into stable
  command metadata.
- Steps 159-168 connect those shared contracts to Win32 and Wayland behavior,
  then close with a written parity audit that separates implemented, partial,
  missing, and Mac/Metal-deferred areas.

## Planning Snapshot

These 40 steps are the post-Step-128 push from "close to GPUI core API" toward a
usable Windows/Linux GPUI-core layer. The plan deliberately stays in four
10-step bands so each band has one coherent product outcome while each
individual step remains small enough for the current RED/GREEN/merge cadence.

- Steps 129-138 finish the context side: authoring aliases, entity handles,
  globals, scoped actions, subscription lifetime, deferred work, timers, async
  completions, update batching, and runtime diagnostics.
- Steps 139-148 make elements reusable: keyed reconciliation, lifecycle hooks,
  per-element state, classes/theme tokens, cascade resolution, focus handles,
  and first-class button, label, text-input, and scroll-list widgets.
- Steps 149-158 deepen text and rendering: font discovery skeletons,
  deterministic shaping metadata, glyph cache contracts, Vulkan text command
  handling, opacity/transform metadata, batching, HiDPI propagation, command
  snapshots, and unsupported-command diagnostics.
- Steps 159-168 close the Windows/Linux runtime loop: multi-window ownership,
  lifecycle events, Win32 and Wayland IME/DnD hooks, event-loop wakeups,
  accessibility snapshots, demo smoke tests, and a GPUI-core parity audit with
  Mac/Metal explicitly handed off.

The intended outcome after Step 168 is not full GPUI parity. It is a practical
Windows/Linux foundation where application code can be written against a
GPUI-like public prelude, common widgets are reusable, text/render command
streams are backend-ready, platform hooks exist for the active targets, and the
remaining GPUI gaps are visible enough to plan the next milestone honestly.

## Back-40 Scope Summary

The "back 40 steps" are Steps 129-168. They intentionally begin only after the
Step 128 demo rewrite proves the current core can express a real Windows/Linux
application through the public prelude.

- Steps 129-138: close context-side API gaps. This gives author code typed
  context aliases, entity handles, globals, scoped actions, subscription
  ownership, deferred callbacks, timers, async completions, update batching,
  and diagnostics.
- Steps 139-148: make reusable UI possible. This adds keyed reconciliation,
  lifecycle hooks, element state, style vocabulary/cascade, focus handles, and
  the first widget primitives: button, label, text input, and scrollable list.
- Steps 149-158: mature text and renderer contracts. This adds font discovery
  skeletons, shaping metadata, glyph cache interfaces, Vulkan text handling,
  opacity/transform metadata, batching, frame stats, HiDPI propagation,
  snapshot tests, and unsupported-command diagnostics.
- Steps 159-168: close Windows/Linux platform behavior. This adds multi-window
  runtime storage, window lifecycle events, Win32 and Wayland IME/DnD hooks,
  platform wakeups, accessibility snapshots, demo smoke tests, and the final
  GPUI-core API parity audit.

## Back-40 Completion Target

After Step 168, the Windows/Linux track should be able to support a small
GPUI-like app without reaching through private runtime plumbing:

- App code uses `cgpui/cgpui.hpp`, `Context<T>`-shaped APIs, entities,
  globals, scoped actions, subscriptions, deferred work, timers, async
  completions, and diagnostics from public headers.
- UI code composes keyed elements and reusable button, label, text input, and
  scrollable list widgets with style classes, theme tokens, style states, and
  focus handles.
- Paint output carries enough deterministic metadata for text, glyphs,
  opacity, transforms, clips, HiDPI scale, batching, and unsupported-command
  diagnostics that the Vulkan backend can evolve without changing public UI
  APIs again.
- Win32 and Wayland expose the platform hooks needed by the active API:
  multi-window records, lifecycle events, IME geometry, drag/drop skeletons,
  wakeups, accessibility snapshots, clipboard/cursor continuity, and demo smoke
  coverage.
- `docs/gpui-core-api-parity.md` is the truth source for what is implemented,
  partial, missing, and Mac/Metal-deferred. Step 168 closes this milestone by
  making the remaining gaps explicit, not by claiming full upstream GPUI
  parity.

## Back-40 Non-Goals

Keep these out of Steps 129-168 unless the roadmap is explicitly rewritten:

- Do not start macOS/Cocoa + Metal parity work. Only document Mac boundaries in
  the Step 168 audit.
- Do not replace Vulkan, the current xmake setup, or the existing Win32/Wayland
  platform split.
- Do not pull in a full text shaping stack or OS font fallback policy before
  the deterministic shaping/glyph-cache contracts exist.
- Do not turn widget primitives into a separate framework. They should remain
  examples of what the public element, style, context, and action APIs can
  express.
- Do not combine adjacent steps to save commits. The small RED/GREEN cadence is
  part of the safety model for keeping Windows and WSL verification green.

## Back-40 Planning Commitments

These commitments make the back-40 plan executable without turning it into a
second active branch while Steps 121-128 are still incomplete:

- Step 129 is a gate transition, not today's next branch. It starts only after
  Step 128 is merged, Windows full debug passes, WSL Arch full debug passes,
  and the main worktree has no tracked or staged changes.
- Steps 129-168 stay sequential by default. If a step proves too large, split
  it into a prep step and a behavior step only after updating this plan and
  `task_plan.md`; do not silently combine neighboring steps.
- Every step keeps one branch slug, one first RED test intent, one targeted
  verification command, one Windows full debug run, and one WSL Arch full debug
  run before and after merge.
- Every 10-step band has a checkpoint review. At Steps 138, 148, 158, and 168,
  confirm the band exit check below before opening the next band.
- Platform-specific work remains isolated under Win32 and Wayland files.
  Shared headers and runtime contracts must stay platform-neutral so macOS can
  receive a later Metal/Cocoa parity pass without undoing Windows/Linux API
  work.

## Back-40 Entry Contract

Step 129 starts only from a clean `master` after Step 128. The handoff state
must have these properties:

- `View::render(ViewContext&)` is the normal public-prelude authoring path,
  and the runtime installs the rendered element tree before layout, hit
  testing, paint, and event routing.
- Typed model helpers, weak handles, observation, model-driven invalidation,
  `AppContext`, `WindowOptions`, root-view storage, the view registry, and
  child-view placeholders are all present from Steps 99-108.
- Event route ancestry, bubbling, focus traversal, scroll routing,
  overflow-aware hit testing, flex controls, absolute positioning, and
  deterministic layer ordering are all present from Steps 109-118.
- Rounded/text/caret/selection command metadata, Vulkan clip handling,
  cursor/clipboard hooks, IME geometry, and the public-prelude demo are all
  present from Steps 119-128.
- Windows full debug and WSL Arch full debug both pass on `master`, and
  `git status --short --branch` has no tracked or staged changes.

## Execution Gate From The Current State

Do not begin Step 129 until all of these are true:

- [x] Steps 96-98 have landed `View::render(ViewContext&)`, runtime render-tree installation, and render invalidation observability.
- [x] Steps 99-108 have landed model aliases/helpers, weak handles, observation, `AppContext`, `WindowOptions`, root-view lifecycle storage, the view registry, and child-view placeholders.
- [x] Step 109 has landed element and view route ancestry metadata, giving Step 110 a target-to-root route for propagation.
- [x] Step 110 has landed target handling, ancestor bubbling, disabled-ancestor skipping, and root view fallback using route ancestry on `master`.
- [x] Step 111 focus traversal has landed on `master` and is post-merge verified on Windows and WSL Arch Linux.
- [x] Step 112 scroll element binding has landed on `master` and is post-merge verified on Windows and WSL Arch Linux.
- [x] Step 113 wheel/trackpad scroll routing has landed on `master` and is post-merge verified on Windows and WSL Arch Linux.
- [x] Step 114 overflow-aware hit testing has landed on `master` and is post-merge verified on Windows and WSL Arch Linux.
- [x] Step 115 flex alignment/justification has landed on `master` and is post-merge verified on Windows and WSL Arch Linux.
- [x] Step 116 flex grow/shrink has landed on `master` and is post-merge verified on Windows and WSL Arch Linux.
- [x] Step 117 absolute positioning/insets has landed on `master` and is
  post-merge verified on Windows and WSL Arch Linux.
- [x] Step 118 layer/elevation ordering has landed on `master` and is
  post-merge verified on Windows and WSL Arch Linux.
- [x] Step 119 rounded-rect command metadata has landed on `master` and is
  post-merge verified on Windows and WSL Arch Linux.
- [x] Step 120 Vulkan clip handling has landed on `master` and is
  post-merge verified on Windows and WSL Arch Linux.
- [x] Step 121 text paint command metadata has landed on `master` and is
  post-merge verified on Windows and WSL Arch Linux.
- [x] Step 122 font metadata has landed on `master` and is post-merge verified
  on Windows and WSL Arch Linux.
- [x] Step 123 caret/selection command metadata has landed on `master` and is
  post-merge verified on Windows and WSL Arch Linux.
- [x] Step 124 platform cursor application has landed on `master` and is
  post-merge verified on Windows and WSL Arch Linux.
- [x] Step 125 Win32 system clipboard backend has landed on `master` and is
  post-merge verified on Windows and WSL Arch Linux.
- [x] Step 126 Wayland clipboard skeleton has landed on `master` and is
  post-merge verified on Windows and WSL Arch Linux.
- [x] Step 127 IME geometry has landed on `master` and is post-merge verified
  on Windows and WSL Arch Linux.
- [ ] Step 128 has landed the public-prelude demo rewrite.
- [ ] Windows full debug and WSL Arch full debug verification pass on `master` after Step 128, with no tracked/staged changes left behind.

Remaining pre-back-40 implementation count:
1 implementation step remains: Step 128, plus post-Step-128 Windows/WSL
verification.

## Back-40 Execution Strategy

The follow-on 40 steps should be run as a single continuation of the current
discipline, not as a broad rewrite. The shape is:

- Steps 129-138: make context-facing application code pleasant and observable
  before widgets depend on it.
- Steps 139-148: add keyed identity, lifecycle, state, style cascade, focus
  handles, and first reusable widgets on top of that context layer.
- Steps 149-158: deepen text, font, renderer command, batching, HiDPI, and
  diagnostics contracts after widgets can produce representative output.
- Steps 159-168: wire Windows/Wayland lifecycle, IME, drag/drop, wakeups,
  accessibility, smoke tests, and parity documentation after the shared
  renderer/text surfaces are stable.

At each band boundary, do a short review before opening the next branch:

- After Step 138, public app code should not need direct `WindowRuntime`
  plumbing for entities, globals, scoped actions, subscriptions, deferred work,
  timers, async completions, batching, or diagnostics.
- After Step 148, the public prelude should express reusable button, label,
  text input, and scrollable list UI with keyed state and style cascade.
- After Step 158, widget/demo paint streams should expose text, glyph metadata,
  opacity/transform, HiDPI scale, batching diagnostics, and unsupported-command
  diagnostics.
- After Step 168, `docs/gpui-core-api-parity.md` should separate implemented,
  partial, missing, and Mac/Metal-deferred GPUI areas.

## File Map

- `include/cgpui/ui/ui.hpp`: context aliases, app/window/runtime APIs, scoped actions, timers, async completion dispatch, diagnostics, multi-window registry, lifecycle events.
- `src/ui/ui.cpp`: runtime behavior for deferred callbacks, timers, batching, diagnostics, multi-window storage, event-loop wakeups, lifecycle dispatch.
- `include/cgpui/core/entity.hpp`: entity handles, weak handles, global handles, subscription tokens.
- `include/cgpui/ui/element.hpp`: keyed reconciliation, lifecycle hooks, element state storage, widget primitives, style cascade wiring, scrollable list behavior.
- `include/cgpui/ui/style.hpp`: style classes, theme tokens, cascade inputs, opacity, transform, text/font style fields.
- `include/cgpui/ui/text.hpp`: text input widget support, shaping runs, IME/caret geometry, selection metadata.
- `include/cgpui/renderer/renderer.hpp`: text/glyph commands, opacity/transform metadata, batching keys, diagnostics records.
- `src/renderer/vulkan/vulkan_renderer.cpp`: glyph cache consumption, text drawing, batching, HiDPI resource handling, unsupported-command diagnostics.
- `include/cgpui/platform/platform.hpp`: event-loop wakeup, lifecycle events, drag/drop and accessibility skeleton hooks.
- `src/platform/win32/win32_application.cpp`: Win32 IME placement, drag/drop skeleton, lifecycle events, wakeups, accessibility skeleton.
- `src/platform/linux/wayland_application.cpp`: Wayland text-input skeleton, data-device drag/drop skeleton, lifecycle events, wakeups, accessibility skeleton.
- `examples/hello_window/main.cpp`: post-Step-128 demo smoke surface for public API parity checks.
- `tests/ui/*.cpp`, `tests/platform/*.cpp`, `tests/renderer/*.cpp`, `tests/architecture/*.cpp`, `tests/header_cleanliness/*.cpp`: RED/GREEN coverage for each slice.

## Execution Rules

Use one isolated worktree and one branch per step:

```powershell
git worktree add .worktrees/<slug> -b codex/<slug> master
```

For every step:

- [ ] Add the RED test first and confirm the expected failure.
- [ ] Implement the smallest GREEN change.
- [ ] Run targeted tests in the feature worktree.
- [ ] Run Windows full debug verification: `xmake f -c -m debug -P .; xmake test -P .`.
- [ ] Run WSL Arch full debug verification for shared UI/runtime/renderer/platform/build changes.
- [ ] Update `task_plan.md`, `progress.md`, and `findings.md`.
- [ ] Commit, fast-forward merge to `master`, re-run targeted and full verification on `master`.
- [ ] Remove the feature worktree and delete the branch.

For the current pre-back-40 state, start Step 128 before starting Step 129:

```powershell
git checkout master
git status --short --branch
git worktree add .worktrees/public-prelude-demo-rewrite -b codex/public-prelude-demo-rewrite master
cd .worktrees/public-prelude-demo-rewrite
xmake test -P . hello_window_lifetime_test/default prelude_header_cleanliness/default
```

## Post-Step-128 Planning Contract

When Step 128 is merged, do this before starting Step 129:

- [ ] Confirm `master` contains the Step 128 merge and that `git status --short --branch` reports no tracked or staged changes.
- [ ] Run the Step 128 exit verification on `master`:
  `xmake test -P . hello_window_lifetime_test/default prelude_header_cleanliness/default`.
- [ ] Run Windows full debug on `master`:
  `xmake f -c -m debug -P .; xmake test -P .`.
- [ ] Run WSL Arch full debug on `master`:
  `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`.
- [ ] Update `task_plan.md` so Step 128 is complete and Step 129 is the active step.
- [ ] Start Step 129 from a fresh worktree:
  `git worktree add .worktrees/context-authoring-alias -b codex/context-authoring-alias master`.

## Back-40 Review Checkpoints

Run these checkpoint reviews in addition to the per-step verification matrix:

- After Step 138: confirm context/entity/global/action/subscription/defer/timer/async/batching/diagnostics APIs can be used from public headers without direct `WindowRuntime` plumbing.
- After Step 148: confirm button, label, text input, and scrollable list are implemented from public element/context/style APIs and survive keyed reconciliation.
- After Step 158: confirm demo/widget paint command streams include text, glyph metadata, opacity/transform, HiDPI scale, batching diagnostics, and unsupported-command diagnostics.
- After Step 168: confirm `docs/gpui-core-api-parity.md` separates implemented, partial, missing, and Mac/Metal-deferred areas, and that Windows plus WSL verification passed after the final merge.

Each checkpoint should leave a short entry in `progress.md` and `findings.md`
before the next band starts. If a checkpoint finds an API shape that is too
thin for the next band, add a small follow-up step to this document rather than
silently expanding the next implementation branch.

Keep these 40 steps sequential by default. A later step may be split if its RED
test reveals a larger dependency, but do not combine adjacent steps just
because they touch the same files. The point of this queue is to preserve the
small RED/GREEN/merge cadence that has kept the Windows and WSL suites stable.

## Back-40 Execution Profile

The 40-step follow-on should run as one disciplined continuation, not as a
parallel rewrite:

- Steps 129-138 are mostly public API and runtime orchestration. Expect frequent
  `ui.hpp` and `ui.cpp` edits with `window_runtime_test` as the main safety net.
- Steps 139-148 are element/widget work. Keep widget primitives built from
  public element APIs so they stay useful as examples for application code.
- Steps 149-158 are renderer/text maturity work. Keep metadata and command
  snapshots deterministic before depending on full platform font discovery.
- Steps 159-168 are platform and closure work. Keep Win32 and Wayland behavior
  isolated behind platform files, and use Step 168 to document what remains
  partial or Mac/Metal-deferred.

The expected execution rhythm remains one step per feature branch, one focused
RED test, one minimal GREEN implementation, targeted tests, Windows full debug,
WSL Arch full debug, docs/progress update, commit, fast-forward merge, and
post-merge verification.

## Step Exit Contracts

Use this table as the planning-level definition of done for each follow-on
step. The detailed step notes below still own the file-level scope and test
targets.

| Step | Exit artifact | Keep out of this step |
| --- | --- | --- |
| 129 | `Context<T>` is public and source-compatible with existing `ViewContext` helper usage. | New context storage, model ownership rewrites, or async behavior. |
| 130 | Entity handles can read/update/downgrade through existing runtime/context APIs. | A second entity store or lifetime model. |
| 131 | Typed globals can be set, read, and updated with soft-fail missing lookups. | Persistence, serialization, or cross-process global state. |
| 132 | Actions resolve deterministically by app, window, view, and focused-element scope. | Menu systems, command palettes, or platform accelerator integration. |
| 133 | Subscription tokens disconnect observers on release/removal. | Full reactive dependency tracking or automatic lifetime inference. |
| 134 | `cx.defer(...)` queues FIFO post-event callbacks before the next redraw flush. | Threads, timers, or async task spawning. |
| 135 | One-shot and repeating timers are deterministic under runtime test ticks. | Real OS timer backends beyond the minimal wakeup contract. |
| 136 | Async task handles deliver completions through the runtime main-thread queue. | A thread pool, coroutine framework, or cancellation semantics beyond skeleton handles. |
| 137 | Batched model/global changes coalesce redraw scheduling. | Changing model notification order or observer callback semantics. |
| 138 | Public diagnostics expose entity, subscription, invalidation, and frame state. | Profiling UI, tracing export, or platform-specific diagnostics. |
| 139 | Keyed children preserve ids across reorder/insert/remove. | Lifecycle hooks or element state storage. |
| 140 | Mount/update/unmount hooks fire during reconciliation. | Widget state bags or style cascade. |
| 141 | Typed element state survives keyed reconciliation by element id. | General app globals or persistent storage. |
| 142 | Style classes and theme tokens are public vocabulary objects. | Cascade application or runtime theme switching. |
| 143 | Base, class, state, and inline style resolution has a deterministic order. | A CSS parser, selectors, or inherited typography beyond planned fields. |
| 144 | `FocusHandle` wraps request/release/query behavior over current focus ownership. | Platform focus rings or accessibility focus adapters. |
| 145 | Button widget composes public element/focus/style/action APIs. | A private widget framework or platform-native controls. |
| 146 | Label widget emits styled text paint command metadata. | Text shaping, glyph atlas ownership, or Vulkan text drawing. |
| 147 | Text input widget integrates focus, text editing, selection, clipboard, and IME geometry surfaces. | Full rich text, multi-line layout, or OS text services beyond current hooks. |
| 148 | Scrollable list uses stable item keys, scroll offset, and viewport clip metadata. | Virtualization, recycling, or async data loading. |
| 149 | Font database and platform discovery skeletons expose deterministic descriptors. | Full fallback policy or mandatory system font enumeration in tests. |
| 150 | Shaping runs carry deterministic fallback metrics and glyph advances. | HarfBuzz integration or complex-script correctness claims. |
| 151 | Glyph cache/atlas interfaces define lookup, miss, and metadata contracts. | Concrete Vulkan atlas allocation. |
| 152 | Vulkan accepts text draw commands through cached glyph metadata. | Production-quality glyph rasterization or font discovery coupling. |
| 153 | Opacity and transform metadata reach paint/renderer command streams. | GPU transform pipelines or animation systems. |
| 154 | Renderer batching diagnostics group by clip, opacity, transform, and primitive kind. | Performance optimization beyond observable grouping. |
| 155 | Frame diagnostics report layout, paint, render, command counts, and timing fields. | External profilers or timeline visualization. |
| 156 | HiDPI scale flows through layout, text metrics, and renderer resize data. | Per-monitor migration behavior or fractional-layout redesign. |
| 157 | Demo and widget paint command streams have stable snapshot coverage. | Golden image rendering or pixel-perfect text assertions. |
| 158 | Unsupported commands produce explicit diagnostics instead of silent drops. | Implementing every unsupported command. |
| 159 | Multi-window registry owns independent per-window root and renderer records. | Full platform multi-window event-loop rewrite. |
| 160 | Window lifecycle events update dispatch records and callbacks. | Native titlebar/menu policy or platform-specific lifecycle quirks. |
| 161 | Win32 IME placement consumes focused text geometry. | Wayland IME protocol work or rich text composition UI. |
| 162 | Wayland text-input skeleton consumes focused text geometry with graceful unsupported behavior. | Full compositor protocol coverage or Win32 changes. |
| 163 | Win32 drag/drop text/file event shapes and deterministic hooks exist. | Real shell integration beyond skeleton test hooks. |
| 164 | Wayland data-device drag/drop event shapes soft-fail without data. | Full MIME negotiation or file-manager integration. |
| 165 | Timers, async completions, and deferred callbacks can request platform wakeups. | Replacing the platform event loop. |
| 166 | Accessibility snapshots expose roles, names, text, and focus state. | OS-specific accessibility adapters. |
| 167 | Demo smoke tests cover window, input, text, clipboard, redraw, and close flows. | Broad end-to-end automation or Mac coverage. |
| 168 | `docs/gpui-core-api-parity.md` audits implemented, partial, missing, and Mac-deferred areas. | Claiming full upstream GPUI parity. |

## Step-by-Step Execution Matrix

This matrix is the operational queue for the follow-on 40 steps. The detailed
notes below remain the source of file-level scope; this table pins down the
branch names, first RED test intent, and targeted verification command.

| Step | Branch slug | First RED test intent | Targeted verification |
| --- | --- | --- | --- |
| 129 | `context-authoring-alias` | `test_context_alias_exposes_view_context_helpers` compiles `Context<T>` and calls existing context helpers. | `xmake test -P . window_runtime_test/default prelude_header_cleanliness/default` |
| 130 | `entity-handle-convenience` | `test_entity_handle_read_update_and_downgrade` covers handle read/update/weak conversion. | `xmake test -P . entity_store_test/default window_runtime_test/default` |
| 131 | `global-app-state-registry` | `test_app_context_global_state_helpers` stores, reads, and updates typed globals. | `xmake test -P . app_runner_test/default window_runtime_test/default` |
| 132 | `scoped-action-registry` | `test_scoped_actions_resolve_in_specificity_order` verifies app/window/view/focused-element action precedence. | `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` |
| 133 | `subscription-token-lifetime` | `test_subscription_token_disconnects_on_release` proves dropped tokens stop observer callbacks. | `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` |
| 134 | `deferred-callback-queue` | `test_defer_runs_after_current_event_before_redraw` verifies FIFO post-event work. | `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` |
| 135 | `runtime-timer-api` | `test_one_shot_and_repeating_timers_tick_deterministically` drives fake timer ticks. | `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` |
| 136 | `async-task-completion` | `test_async_task_completion_dispatches_on_runtime_queue` injects completion and observes main-thread callback. | `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` |
| 137 | `runtime-update-batching` | `test_model_and_global_updates_coalesce_one_redraw` counts redraw scheduling across a batch. | `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` |
| 138 | `runtime-diagnostics-snapshot` | `test_diagnostics_reports_entities_subscriptions_invalidations_and_frames` snapshots runtime state. | `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` |
| 139 | `keyed-element-identity` | `test_keyed_children_preserve_ids_across_reorder` proves keyed reconciliation beats index matching. | `xmake test -P . element_test/default ui_header_cleanliness/default` |
| 140 | `element-lifecycle-hooks` | `test_element_lifecycle_mount_update_unmount` records lifecycle callbacks during reconciliation. | `xmake test -P . element_test/default ui_header_cleanliness/default` |
| 141 | `element-state-storage` | `test_element_state_survives_keyed_reconcile` stores typed state by element id. | `xmake test -P . element_test/default window_runtime_test/default` |
| 142 | `style-classes-theme-tokens` | `test_style_classes_and_theme_tokens_are_public_vocabulary` covers ids and token lookup. | `xmake test -P . style_test/default prelude_header_cleanliness/default` |
| 143 | `style-cascade-resolution` | `test_style_cascade_merges_base_class_state_and_inline` fixes cascade order. | `xmake test -P . style_test/default element_test/default` |
| 144 | `focus-handle-primitive` | `test_focus_handle_requests_releases_and_queries_focus` wraps current focus APIs. | `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` |
| 145 | `button-widget-primitive` | `test_button_widget_focus_click_action_and_disabled_state` composes public primitives into a button. | `xmake test -P . element_test/default window_runtime_test/default` |
| 146 | `label-widget-primitive` | `test_label_widget_emits_text_paint_with_style` verifies label text command metadata. | `xmake test -P . element_test/default ui_header_cleanliness/default` |
| 147 | `text-input-widget-primitive` | `test_text_input_widget_integrates_focus_edit_clipboard_and_ime_geometry` exercises the full text input path. | `xmake test -P . element_test/default window_runtime_test/default text_model_test/default` |
| 148 | `scrollable-list-container` | `test_scrollable_list_uses_keys_scroll_offset_and_clip_metadata` covers list viewport behavior. | `xmake test -P . element_test/default scroll_test/default` |
| 149 | `font-database-skeleton` | `test_font_database_descriptor_and_fake_discovery` keeps discovery deterministic. | `xmake test -P . text_model_test/default win32_text_input_test/default` |
| 150 | `text-shaping-runs` | `test_text_shaping_run_fallback_metrics_are_deterministic` creates shaping metadata without HarfBuzz. | `xmake test -P . text_model_test/default element_test/default` |
| 151 | `glyph-cache-interface` | `test_glyph_cache_records_hits_misses_and_text_metadata` defines renderer-facing cache contracts. | `xmake test -P . element_test/default vulkan_solid_rect_test/default` |
| 152 | `vulkan-text-draw-path` | `test_vulkan_accepts_text_draw_commands_with_cached_glyphs` routes text commands through Vulkan test doubles. | `xmake test -P . vulkan_solid_rect_test/default` |
| 153 | `opacity-transform-metadata` | `test_opacity_and_transform_reach_paint_commands` propagates style metadata. | `xmake test -P . style_test/default element_test/default` |
| 154 | `renderer-command-batching` | `test_renderer_batches_by_clip_opacity_transform_and_kind` verifies stable batching keys. | `xmake test -P . vulkan_solid_rect_test/default` |
| 155 | `frame-statistics-diagnostics` | `test_frame_statistics_report_render_layout_paint_and_command_counts` extends diagnostics. | `xmake test -P . window_runtime_test/default` |
| 156 | `hidpi-scale-propagation` | `test_hidpi_scale_flows_to_layout_text_and_renderer_resize` keeps logical pixels stable. | `xmake test -P . layout_test/default window_runtime_test/default` |
| 157 | `paint-command-snapshots` | `test_demo_and_widgets_emit_stable_paint_snapshots` adds deterministic command serializers. | `xmake test -P . element_test/default render_view_test/default hello_window_lifetime_test/default` |
| 158 | `renderer-unsupported-diagnostics` | `test_unsupported_renderer_commands_record_explicit_diagnostics` prevents silent drops. | `xmake test -P . vulkan_solid_rect_test/default` |
| 159 | `multi-window-runtime-registry` | `test_multi_window_registry_owns_independent_runtime_records` covers per-window roots/renderers. | `xmake test -P . app_runner_test/default` |
| 160 | `window-lifecycle-events` | `test_window_lifecycle_events_update_dispatch_records` covers activate/focus/minimize/restore/close. | `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` |
| 161 | `win32-ime-placement` | `test_win32_ime_placement_uses_focused_text_geometry` wires candidate geometry to Win32 hooks. | `xmake test -P . win32_text_input_test/default window_runtime_test/default` |
| 162 | `wayland-text-input-ime` | `test_wayland_text_input_skeleton_consumes_focused_geometry` keeps unsupported behavior graceful. | `xmake test -P . wayland_keyboard_test/default window_runtime_test/default` |
| 163 | `win32-drag-drop-skeleton` | `test_win32_drag_drop_events_have_text_and_file_payload_shapes` defines Win32 DnD events. | `xmake test -P . win32_input_event_test/default` |
| 164 | `wayland-data-device-dnd` | `test_wayland_data_device_drag_drop_events_soft_fail_without_data` defines Wayland DnD events. | `xmake test -P . wayland_pointer_button_test/default` |
| 165 | `platform-event-loop-wakeup` | `test_runtime_async_timer_and_defer_request_platform_wakeup` bridges runtime queues to platforms. | `xmake test -P . window_runtime_test/default win32_input_event_test/default wayland_keyboard_test/default` |
| 166 | `accessibility-tree-skeleton` | `test_accessibility_tree_reports_roles_names_and_focus_state` exposes platform-neutral a11y snapshots. | `xmake test -P . element_test/default window_runtime_test/default` |
| 167 | `windows-linux-demo-smoke` | `test_demo_smoke_covers_window_input_text_clipboard_redraw_and_close` validates the public demo path. | `xmake test -P . hello_window_lifetime_test/default` |
| 168 | `gpui-core-api-parity-audit` | `test_gpui_core_api_parity_audit_lists_completed_partial_missing_and_deferred` closes the milestone. | `xmake test -P . desktop_target_readiness_test/default` |

## Verification Matrix

Use targeted tests before every commit. After each feature branch goes GREEN,
run the full platform matrix before merging and again on `master` after the
fast-forward merge:

- Windows full debug is mandatory for every step:
  `xmake f -c -m debug -P .; xmake test -P .`.
- WSL Arch full debug is mandatory for every step because all 40 follow-on
  steps touch shared public APIs, UI/runtime behavior, renderer command
  contracts, or Win32/Wayland platform boundaries.
- Platform-specific steps still run their opposite-platform smoke coverage when
  a shared header changes. For example, Step 161 is Win32 IME placement, but it
  still needs WSL full debug because the focused-text geometry API is shared.
- If a platform target is not available in the current xmake configuration, log
  the skipped target and reason in `progress.md`, then run the full suite that
  is available. Do not mark the step complete until the skipped target is
  either restored or explicitly accepted as a temporary environment issue.

## Dependency Notes

- Steps 129-138 must land before widget work because widgets need stable
  context, entity/global state, scoped action, subscription, deferred, timer,
  async, batching, and diagnostics APIs.
- Steps 139-148 must land before the renderer/text deepening band because text
  commands and snapshots need keyed state, lifecycle hooks, style cascade,
  focus handles, and real widgets to produce representative command streams.
- Steps 149-158 must land before platform completion because IME placement,
  accessibility, smoke tests, and API parity need text geometry, glyph metadata,
  renderer diagnostics, HiDPI scale, and unsupported-command reporting.
- Steps 159-168 close the Windows/Linux loop. Keep macOS/Metal as a handoff
  boundary in Step 168; do not pull it into the active verification definition.

## Step 129 Start Packet

When the Step 128 exit contract is satisfied, begin the back 40 with this
exact setup:

```powershell
git status --short --branch
xmake test -P . hello_window_lifetime_test/default prelude_header_cleanliness/default
xmake f -c -m debug -P .
xmake test -P .
wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'
git worktree add .worktrees/context-authoring-alias -b codex/context-authoring-alias master
```

Then add the Step 129 RED tests before touching implementation code:

- `tests/header_cleanliness/prelude_header_cleanliness.cpp` should compile a
  small authoring snippet that names `Context<SomeView>`.
- `tests/ui/window_runtime_test.cpp` should verify the alias can call existing
  context helpers without losing `ViewContext` behavior.
- Expected RED failure: `cgpui::Context<T>` is not declared or cannot call the
  existing context helper surface.

Do not reuse the Step 128 worktree for Step 129. The back-40 queue should start
from a verified `master` so later parity claims are anchored to the public
Step 128 demo state.

## Milestone Bands

### Band E: Context, Entity, Global State, and Async, Steps 129-138

Purpose: make the model/context side feel closer to GPUI authoring instead of direct runtime plumbing.

- [ ] Step 129: public `Context<T>` authoring alias over `ViewContext` for view/model code.
- [ ] Step 130: entity handle API with `read`, `update`, and `downgrade` convenience methods.
- [ ] Step 131: global app state registry with typed `set_global`, `global`, and `update_global` helpers.
- [ ] Step 132: scoped action registry for app, window, view, and focused element actions.
- [ ] Step 133: subscription ownership token that disconnects observers on drop/removal.
- [ ] Step 134: deferred callback queue for `cx.defer(...)` style post-event work.
- [ ] Step 135: timer API for one-shot and repeating callbacks through the runtime loop.
- [ ] Step 136: async task handle skeleton with main-thread completion dispatch.
- [ ] Step 137: runtime update batching so multiple model/global changes coalesce redraws.
- [ ] Step 138: public diagnostics snapshot for entities, subscriptions, invalidations, and frames.

Exit check: author code can use context-shaped APIs for entities, globals, actions, subscriptions, deferred work, timers, and async completions, with deterministic diagnostics and redraw batching.

### Band F: Reconciliation, Style Cascade, Focus Handles, and Widgets, Steps 139-148

Purpose: make elements reusable and widget-ready, not just one-off builder trees.

- [ ] Step 139: keyed element identity and keyed reconciliation beyond parent-local index matching.
- [ ] Step 140: element lifecycle hooks for mount, update, and unmount notifications.
- [ ] Step 141: element state storage keyed by element id for reusable widgets.
- [ ] Step 142: style class and theme token primitives for reusable design vocabulary.
- [ ] Step 143: style cascade resolution combining base, class, state, and inline styles.
- [ ] Step 144: `FocusHandle` primitive with request, release, contains, and focused queries.
- [ ] Step 145: button widget primitive built from public element, focus, style, and action APIs.
- [ ] Step 146: label widget primitive using text style and text paint commands.
- [ ] Step 147: text input widget primitive integrating focus, text model, selection, clipboard, and IME geometry.
- [ ] Step 148: scrollable list container with stable item keys and viewport clipping metadata.

Exit check: the public prelude can express common app UI controls through reusable widgets, keyed state, style classes/themes, and focus handles.

### Band G: Text, Font, Renderer, and Diagnostics Depth, Steps 149-158

Purpose: turn text and renderer output from metadata/placeholder paths into backend-ready command streams.

- [ ] Step 149: font database abstraction and platform font discovery skeleton for Win32 and Linux.
- [ ] Step 150: text shaping run abstraction with deterministic fallback metrics before full shaping.
- [ ] Step 151: glyph atlas/cache interface shared by text elements and Vulkan renderer.
- [ ] Step 152: Vulkan text draw path consumes text paint commands through cached glyph metadata.
- [ ] Step 153: opacity and transform paint metadata with deterministic command ordering.
- [ ] Step 154: renderer command batching by clip, opacity, transform, and primitive kind.
- [ ] Step 155: frame timing and paint/layout/render statistics exposed through diagnostics.
- [ ] Step 156: HiDPI scale propagation into layout, text metrics, and renderer resources.
- [ ] Step 157: snapshot tests for paint command streams emitted by the demo and widgets.
- [ ] Step 158: renderer fallback path for unsupported commands with explicit diagnostics.

Exit check: text, opacity, transform, batching, HiDPI scale, and renderer diagnostics are represented in command streams and Vulkan-facing code without regressing existing rectangle rendering.

### Band H: Platform Completion, Multi-Window, Accessibility, and Parity Audit, Steps 159-168

Purpose: connect the public API to Windows/Wayland runtime behavior and close the Windows/Linux parity loop.

- [ ] Step 159: multi-window runtime registry with per-window root view and renderer ownership.
- [ ] Step 160: window activation, focus, minimize, restore, and close lifecycle events.
- [ ] Step 161: Win32 IME composition window placement wired to focused text geometry.
- [ ] Step 162: Wayland text-input/IME protocol skeleton wired to focused text geometry.
- [ ] Step 163: Win32 drag-and-drop text/file event skeleton.
- [ ] Step 164: Wayland data-device drag-and-drop text/file event skeleton.
- [ ] Step 165: platform event loop wakeup API for timers, async completions, and deferred callbacks.
- [ ] Step 166: accessibility tree skeleton for labels, buttons, text inputs, and focus state.
- [ ] Step 167: Windows/Linux demo smoke tests covering window, input, text, clipboard, and redraw flows.
- [ ] Step 168: GPUI-core API parity audit document with remaining gaps and Mac parity handoff boundaries.

Exit check: Windows and Wayland have the platform hooks needed by the public core API, the demo is smoke-tested on both active targets, and the remaining GPUI parity gaps are explicitly documented.

## Step Details

### Step 129: Public `Context<T>` Authoring Alias

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `tests/header_cleanliness/prelude_header_cleanliness.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED compile coverage that lets a view/model author spell `Context<MyView>` and call existing `ViewContext` helpers through it.
- [ ] Implement the alias or wrapper as an additive API over `ViewContext`.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default prelude_header_cleanliness/default`.

### Step 130: Entity Handle Convenience API

**Files:**
- Modify: `include/cgpui/core/entity.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `tests/core/entity_store_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for an entity handle that stores a typed id and supports `read(cx)`, `update(cx, fn)`, and `downgrade()`.
- [ ] Implement handle helpers over the existing entity store/context APIs.
- [ ] Targeted test command: `xmake test -P . entity_store_test/default window_runtime_test/default`.

### Step 131: Global App State Registry

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/app_runner_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for typed `set_global`, `global`, and `update_global` helpers.
- [ ] Store globals by `std::type_index` and keep missing globals as soft-fail lookups.
- [ ] Targeted test command: `xmake test -P . app_runner_test/default window_runtime_test/default`.

### Step 132: Scoped Action Registry

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for app/window/view/focused-element action lookup order.
- [ ] Extend the existing action registry with explicit scope metadata while preserving global action behavior.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 133: Subscription Ownership Token

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for a subscription token that disconnects observer callbacks when released or removed.
- [ ] Implement token ids and soft-fail removal without changing existing subscription query behavior.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 134: Deferred Callback Queue

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for `ViewContext::defer(...)` running after the current event dispatch and before the next redraw flush.
- [ ] Implement a FIFO deferred callback queue owned by `WindowRuntime`.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 135: Timer API

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for deterministic one-shot and repeating timer callbacks driven by a fake runtime tick.
- [ ] Implement timer registration/cancellation without requiring real OS timers in unit tests.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 136: Async Task Handle Skeleton

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for spawning a task handle and delivering completion back through the runtime main-thread queue.
- [ ] Implement a minimal task/completion abstraction with deterministic test injection.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 137: Runtime Update Batching

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests proving multiple model/global updates coalesce into one scheduled redraw.
- [ ] Implement an update batch guard or queue flush around existing invalidation scheduling.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 138: Public Diagnostics Snapshot

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for diagnostics that report entity counts, subscription counts, invalidation state, frame index, and recent render status.
- [ ] Expose a read-only runtime/context diagnostics snapshot.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 139: Keyed Element Identity

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for stable ids across keyed child reorder, insert, and removal.
- [ ] Extend reconciliation with optional stable keys while preserving existing index-based behavior for unkeyed children.
- [ ] Targeted test command: `xmake test -P . element_test/default ui_header_cleanliness/default`.

### Step 140: Element Lifecycle Hooks

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for mount, update, and unmount callbacks during root and keyed child reconciliation.
- [ ] Implement lifecycle hooks with no-op defaults on `Element`.
- [ ] Targeted test command: `xmake test -P . element_test/default ui_header_cleanliness/default`.

### Step 141: Element State Storage

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for state storage keyed by element id and preserved across keyed reconciliation.
- [ ] Implement a type-indexed state bag with soft-fail lookup.
- [ ] Targeted test command: `xmake test -P . element_test/default window_runtime_test/default`.

### Step 142: Style Classes and Theme Tokens

**Files:**
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `tests/ui/style_test.cpp`
- Modify: `tests/header_cleanliness/prelude_header_cleanliness.cpp`

- [ ] Add RED tests for class ids, theme token ids, and typed color/spacing token lookup.
- [ ] Implement inert style vocabulary primitives before applying cascade behavior.
- [ ] Targeted test command: `xmake test -P . style_test/default prelude_header_cleanliness/default`.

### Step 143: Style Cascade Resolution

**Files:**
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/style_test.cpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for base, class, state, and inline style resolution order.
- [ ] Implement deterministic cascade merge rules using the Step 95 style-state primitives.
- [ ] Targeted test command: `xmake test -P . style_test/default element_test/default`.

### Step 144: FocusHandle Primitive

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for focus handles requesting/releasing focus and querying current focus.
- [ ] Implement handle storage over existing element focus owner data.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 145: Button Widget Primitive

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for a button widget invoking an action/click handler, exposing focusability, disabled state, and style states.
- [ ] Build the widget from existing element wrappers and public builder APIs.
- [ ] Targeted test command: `xmake test -P . element_test/default window_runtime_test/default`.

### Step 146: Label Widget Primitive

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for a label widget emitting text paint commands with foreground/font style.
- [ ] Implement the label as a thin text element helper with style support.
- [ ] Targeted test command: `xmake test -P . element_test/default ui_header_cleanliness/default`.

### Step 147: Text Input Widget Primitive

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for focused text input handling selection, clipboard, key edits, and IME geometry.
- [ ] Compose the widget from `TextModel`, focus handle, text element, and runtime text helpers.
- [ ] Targeted test command: `xmake test -P . element_test/default window_runtime_test/default text_model_test/default`.

### Step 148: Scrollable List Container

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `include/cgpui/ui/scroll.hpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/ui/scroll_test.cpp`

- [ ] Add RED tests for stable keyed list items, viewport clipping metadata, and scroll state offset.
- [ ] Implement a scrollable list container on top of existing scroll state and keyed elements.
- [ ] Targeted test command: `xmake test -P . element_test/default scroll_test/default`.

### Step 149: Font Database Skeleton

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/ui/text_model_test.cpp`
- Modify: `tests/platform/win32_text_input_test.cpp`

- [ ] Add RED tests for platform-neutral font descriptors and deterministic fake font discovery.
- [ ] Add Win32/Linux discovery skeletons with graceful empty-result behavior in tests.
- [ ] Targeted test command: `xmake test -P . text_model_test/default win32_text_input_test/default`.

### Step 150: Text Shaping Run Abstraction

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `tests/ui/text_model_test.cpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for shaping runs carrying text, font, glyph advances, and fallback metrics.
- [ ] Implement deterministic fallback shaping without requiring HarfBuzz yet.
- [ ] Targeted test command: `xmake test -P . text_model_test/default element_test/default`.

### Step 151: Glyph Atlas/Cache Interface

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for glyph cache lookup/miss records and text command glyph metadata.
- [ ] Define renderer-facing glyph atlas interfaces without forcing a concrete Vulkan allocation in this slice.
- [ ] Targeted test command: `xmake test -P . element_test/default vulkan_solid_rect_test/default`.

### Step 152: Vulkan Text Draw Path

**Files:**
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`

- [ ] Add RED renderer tests for text draw command acceptance with cached glyph metadata.
- [ ] Implement a minimal Vulkan text command path that is testable without real font discovery.
- [ ] Targeted test command: `xmake test -P . vulkan_solid_rect_test/default`.

### Step 153: Opacity and Transform Metadata

**Files:**
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `tests/ui/style_test.cpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for opacity and transform style fields reaching paint command metadata.
- [ ] Implement metadata propagation without changing default paint output.
- [ ] Targeted test command: `xmake test -P . style_test/default element_test/default`.

### Step 154: Renderer Command Batching

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`

- [ ] Add RED tests for stable batching keys by clip, opacity, transform, and primitive kind.
- [ ] Implement command grouping diagnostics before optimizing GPU paths.
- [ ] Targeted test command: `xmake test -P . vulkan_solid_rect_test/default`.

### Step 155: Frame Statistics Diagnostics

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for layout, paint, render, command count, and frame timing fields.
- [ ] Populate deterministic counters in tests and leave real timing optional.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default`.

### Step 156: HiDPI Scale Propagation

**Files:**
- Modify: `include/cgpui/ui/layout.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/layout_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests proving DPI scale affects text metrics, layout context, and renderer resize data consistently.
- [ ] Thread scale through layout/render context without changing authored logical pixel APIs.
- [ ] Targeted test command: `xmake test -P . layout_test/default window_runtime_test/default`.

### Step 157: Paint Command Snapshot Tests

**Files:**
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/ui/render_view_test.cpp`
- Modify: `tests/architecture/hello_window_lifetime_test.cpp`

- [ ] Add RED snapshot-style tests for demo/widget paint command streams.
- [ ] Add deterministic serializers for paint commands in tests only.
- [ ] Targeted test command: `xmake test -P . element_test/default render_view_test/default hello_window_lifetime_test/default`.

### Step 158: Renderer Unsupported-Command Diagnostics

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`

- [ ] Add RED tests proving unsupported renderer commands produce explicit diagnostics instead of silent drops.
- [ ] Implement fallback diagnostics while keeping supported solid/text paths unchanged.
- [ ] Targeted test command: `xmake test -P . vulkan_solid_rect_test/default`.

### Step 159: Multi-Window Runtime Registry

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/app_runner_test.cpp`

- [ ] Add RED tests for multiple runtime/window records with independent root views and renderers.
- [ ] Implement a registry that keeps ownership explicit and preserves the single-window path.
- [ ] Targeted test command: `xmake test -P . app_runner_test/default`.

### Step 160: Window Lifecycle Events

**Files:**
- Modify: `include/cgpui/core/events.hpp`
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for activation, focus, minimize, restore, and close lifecycle events.
- [ ] Route lifecycle events through existing dispatch records and callbacks.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 161: Win32 IME Placement

**Files:**
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/platform/win32_text_input_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for focused text geometry reaching Win32 IME candidate/composition placement data.
- [ ] Wire the existing IME geometry surface into the Win32 platform layer.
- [ ] Targeted test command: `xmake test -P . win32_text_input_test/default window_runtime_test/default`.

### Step 162: Wayland Text-Input IME Skeleton

**Files:**
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/platform/wayland_keyboard_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for a Wayland text-input skeleton consuming focused text geometry with graceful unsupported behavior.
- [ ] Keep protocol wiring isolated in the Wayland platform source.
- [ ] Targeted test command: `xmake test -P . wayland_keyboard_test/default window_runtime_test/default`.

### Step 163: Win32 Drag-and-Drop Skeleton

**Files:**
- Modify: `include/cgpui/core/events.hpp`
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `tests/platform/win32_input_event_test.cpp`

- [ ] Add RED tests for text/file drag-enter, drag-over, drop, and leave event shapes.
- [ ] Implement Win32 skeleton event translation with deterministic test hooks.
- [ ] Targeted test command: `xmake test -P . win32_input_event_test/default`.

### Step 164: Wayland Data-Device Drag-and-Drop Skeleton

**Files:**
- Modify: `include/cgpui/core/events.hpp`
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/platform/wayland_pointer_button_test.cpp`

- [ ] Add RED tests for Wayland data-device drag/drop event shapes with graceful no-data behavior.
- [ ] Implement isolated Wayland skeleton hooks.
- [ ] Targeted test command: `xmake test -P . wayland_pointer_button_test/default`.

### Step 165: Platform Event Loop Wakeup API

**Files:**
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests proving timers, async completions, and deferred callbacks request platform wakeups.
- [ ] Add a platform wakeup method with Win32/Wayland implementations and deterministic fakes.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default win32_input_event_test/default wayland_keyboard_test/default`.

### Step 166: Accessibility Tree Skeleton

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for accessibility roles/names/focus state on labels, buttons, and text inputs.
- [ ] Expose a platform-neutral accessibility tree snapshot before OS-specific adapters.
- [ ] Targeted test command: `xmake test -P . element_test/default window_runtime_test/default`.

### Step 167: Windows/Linux Demo Smoke Tests

**Files:**
- Modify: `examples/hello_window/main.cpp`
- Modify: `tests/architecture/hello_window_lifetime_test.cpp`
- Modify: `xmake.lua`

- [ ] Add RED smoke coverage for demo startup, input, text, clipboard, redraw, and close paths on Windows and Linux.
- [ ] Keep smoke tests bounded with existing first-frame and close-after-first-frame environment controls.
- [ ] Targeted test command: `xmake test -P . hello_window_lifetime_test/default`.

### Step 168: GPUI-Core API Parity Audit

**Files:**
- Create: `docs/gpui-core-api-parity.md`
- Modify: `task_plan.md`
- Modify: `findings.md`

- [ ] Add RED architecture coverage requiring the parity audit document to list implemented, partial, missing, and Mac-deferred API areas.
- [ ] Write the audit with a Windows/Linux-only completion lens and a separate Mac parity handoff section.
- [ ] Targeted test command: `xmake test -P . desktop_target_readiness_test/default`.

## Risk Controls

- Do not start Step 129 before Step 128 is actually merged and verified unless the user explicitly reprioritizes.
- Keep context/entity/global APIs additive and source-compatible with the Step 99-108 model work.
- Keep widgets implemented from public primitives; avoid special private runtime hooks unless a prior step exposes them generally.
- Keep renderer text work command-driven before depending on full platform font discovery.
- Keep Win32 and Wayland behavior behind platform sources; public headers should stay platform-neutral.
- Keep macOS/Metal out of this Windows/Linux completion track except for explicit readiness notes.
