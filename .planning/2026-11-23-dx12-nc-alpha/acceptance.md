# Acceptance: initial ordinary DX12 alpha with native menus

Date: 2026-11-23 (Asia/Shanghai). Baseline `c40dd0c4`, branch `phase-v`.
Evidence root: `D:/Dev/shots/cgpui-dx12-nc-alpha-20261123/`.

## Delivered

Initial transparent ordinary windows are now supported for CGPUI-owned HWNDs,
with decorations and a visible system title bar, on Windows >=26100 when the
actual redirection-bitmap-alpha attribute succeeds. Existing native HMENU is
preserved: a focused platform leaf copies the OS-drawn menu RGB, flushes GDI,
sets only alpha to255, and updates the nonclient buffer after documented window
and menu events. No replacement menu, text self-drawing, undocumented UAH hook,
per-frame pixel repair or new public presentation mode.

A platform-owned instance/cookie lease controls alpha independently of material
selection. The private renderer lease uses scalar HWND messages only. Window
instance changes, stale owners, competing factories and malformed queued tokens
cannot revoke a newer lease or dereference a caller address. GPU recovery keeps
the window lease; final GPU retirement precedes its release. Alpha operations
require the owning UI thread. Foreign renderer destruction posts a release token;
it is asynchronous and requires UI dispatch. Posting/native release failure can
retain platform ownership until retry or window detach, not promise arbitrary
thread migration or unconditional release.

Opaque renderers keep their HWND flip-discard path and existing clear behavior.
Managed opaque worker factories retain support via a bounded same-process
history handshake while the owner pumps messages. Failed handshakes fail closed
before creating a flip chain. Each window instance conservatively retains prior
flip history, because the experiment proved stale pixels survive retiring an
opaque chain before attempting composition. Dynamic opaque-to-transparent intent
and alpha recreation on a prior opaque HWND remain unsupported. Initially alpha
renderers retain their fixed actual premultiplied route when intent toggles.

Native fullscreen/incompatible chrome requests reject without changing an active
alpha topology. Raw ordinary HWNDs, client/hidden captions, undecorated ordinary
windows and pre-26100 positive alpha cases remain outside support. Vulkan remains
default/auto, explicit DX12 never falls back, and Vulkan/DX12 Popup policy stays
native-first with Overlay only on actual failure/unsupported capability. Internal
DComp remains private; this is not complete DX12 parity or a release/default switch.

## Feedback loop and review

- Stable references require three equal menu captures 100ms apart. Startup
  animation is not permission to relax RGB equality.
- Uncorrected strict probe: all10962 menu pixels differ despite correct client
  alpha. Corrected: zero differences and correct desktop alpha0/.5/1. An opaque
  negative control preserves menus but fails alpha. Automatic event repair
  enters a real native menu loop and generates genuine WM_COMMAND.
- Public initial-alpha tracer was RED before implementation; 11 ordinary native
  cases are now GREEN, including genuine command/Escape loops, changed menus,
  resize/renderer DPI, two idle-device-loss epochs, Mica/none, failed factory,
  duplicate/lifetime and foreign-thread behavior.
- Opaque worker regression was observed as exit3 before the dedicated history
  handshake fix, then passed without narrowing the old opaque contract.
- Nine alpha positive tests check actual OS build. <26100 must reject via the
  real public factory and preserve the HWND/menu/styles before explicit SKIP.
  This host is build26300; no such skip is accepted in its alpha tests.
- First complete Windows run:727 passed/5 structure failures. A136-line window
  header exceeded five existing134-line guards. Two short declarations were
  normalized to existing formatting; no cap changed. New alpha structure guard
  explicitly covers the original134 cap; all205 architecture tests then passed.
- Independent Standards/Spec reviews are recorded in `review.md`: no remaining
  hard violation or actionable correctness issue; one nonblocking test assertion
  duplication heuristic remains.

## Frozen-source verification

Root owns all builds/tests; no-run jobs4 and execution jobs1. Full driver:
`verify-final.ps1` exited 0. Final source snapshot2758 files, excluding `.planning/` and
user `.vscode/`. Source matching is required after all checks.

| Check | Result | Evidence |
| --- | --- | --- |
| Initial public tracer RED | expected unsupported rejection | ordinary-initial-red.log |
| Opaque worker RED | expected exit3 | opaque-thread-red.log |
| Final ordinary WARP+Debug |11 passed,0 failed | ordinary-final-test.log |
| Pure-token platform lease |1 passed | platform-token-test.log |
| Complete architecture |205 passed,0 failed | architecture-final-test.log |
| Windows complete suite |732 passed,0 failed | windows-final-test.log |
| Windows all-member libraries/examples |PASS | windows-all-build.log |
| Dedicated WSL complete suite |640 passed,0 failed | wsl-full-test.log |
| Dedicated WSL all-member libraries/examples |PASS | wsl-all-build.log |
| Intel Graphics adapter0 +Debug |64 passed,0 failed | native-0.log |
| NVIDIA RTX5070Ti Laptop adapter1 +Debug |64 passed,0 failed | native-1.log |
| WARP +Debug +GPU validation |64 passed,0 failed | native-warp.log |
| Native DX12 ASan, WARP+Debug |64 passed,0 failed | asan-test.log |
| SDK graph/header fixtures |4+15 passed | sdk-graph-fixtures.log, sdk-headers-fixtures.log |
| Installed Windows SDK compile/link/run |PASS | installed-sdk-windows.log |
| Windows raw-byte source match |2758 matched; no changed/new/removed paths | source-match.json |
| WSL normalized source match |2758 matched; no errors | wsl-source-match.json |

The SDK consumer explicitly creates a public ordinary transparent window with a
real native menu and DX12 captured alpha, in addition to prior opaque/Popup
cases. Its external installed headers and static library link must close through
the platform-owned uxtheme dependency; the renderer package does not own it.

## Archived research / limitations

Four throwaway probe files are absent from the production suite. They are saved
on local `codex/prototype-dx12-native-menu-alpha`, commit
`65fc087703985e7070e32cade39d8b42b5482035`, with probe-only registration. The
prior-flip transition remains a strict negative, not a passing production test.
The all-composition-from-first-opaque-frame toggle experiment is not adopted.
(That branch was deleted on 2026-11-25 without merging; its source is gone.)

No native old-Windows or macOS verification, physical mouse hover, real monitor
migration, HDR/remote-session coverage, arbitrary TDR, full workbench acceptance,
performance parity, release or push. DPI tests update the renderer/fixture scale,
not physically move across monitors. Native menu tests control owned windows and
compare active/focus states, which need not mean foreground. Device loss is
explicit idle-device removal. Mica application was actual on this host; it is not
a universal policy guarantee. WSL validation used only
`/root/cgpui-logic-cleanup-20261123`, never the user's Linux checkout.


Validation retained its source snapshot, logs and external SDK fixture. D: free
space at final validation was about341MiB; the SDK stage alone contains temporary
library copies. A scoped cleanup attempt was denied before execution, so those
copies remain. No pre-existing files, user data or build caches were cleaned.
The user's untracked `.vscode/` remains untouched. Local commit only; no push.
