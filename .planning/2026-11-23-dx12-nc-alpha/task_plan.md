# Native-menu-safe ordinary alpha investigation

Baseline `c40dd0c4`, branch `phase-v`. Continue ordinary DX12 transparency without
public presentation modes, backend fallback, hidden/replaced native menus or
changes to Vulkan/default/auto. Preserve `.vscode/`, design checkout and the
user's Linux checkout. Root owns all builds/tests, serial execution; local commit
only when the delivered slice is verified, no push.

## Current feedback loop

Throwaway `dx12_menu_alpha_probe_test` has a strict oracle: a real normal HWND's
entire native menu bar must match its opaque reference, while desktop alpha0 and
alpha0.5 match controlled green backing. The initial uncorrected candidate fails
all 10962 menu pixels despite correct client alpha. Success is not just HRESULT.

Reference acquisition itself must stabilize: native/DWM startup animation caused
1843 changed pixels in an early candidate. Require three equal 100ms captures
before recording reference, and check a later repeat. No tolerance was increased.
With this control, opaque copy+SetAlpha255 changes zero pixels; no-alpha negative
control preserves the menu but fails desktop alpha.

## Ranked hypotheses and observed slice

1. The native menu's RGB is intact but GDI leaves alpha unsuitable for DWM.
   Copy the real menu rectangle through BufferedPaint (NONCLIENT), set alpha255,
   preserve all RGB, and copy it back. Stable initial experiments match all 10962
   pixels and preserve desktop alpha, for both old blur and attr39.
2. If RGB readback were polluted, default WM_PRINT/PRF_NONCLIENT could ask Windows
   to redraw into the buffer without framework self-drawing. Current stable copy
   evidence does not require this alternative.
3. NC policy/alternate documented alpha routing may isolate the problem. The
   newer DWMWA_REDIRECTIONBITMAP_ALPHA (Windows 11 build26100+) avoids coupling the
   alpha lease to old blur/material-extension settings; its uncorrected menu
   still needs alpha repair. Do not silently use a negative colorkey workaround.

## Gates before production support

- [done] Event-driven repair, no per-frame polling or manual hidden fix:
  native menu open/highlight/close and real WM_COMMAND using owned-window input.
  No undocumented UAH menu drawing hooks.
- [done] Fresh composition vs retired same-HWND flip content, with backing
  color changes. GPU/CPU release alone does not prove no stale desktop pixels.
- [done] Choose a bounded implementation based on these outcomes; no broad
  feature claim if interactive menu updates or safe transitions remain unsolved.
- [done] Focused platform alpha/paint leaves and private renderer lease if
  viable. No renderer dependency on concrete platform-window objects; versioned,
  instance/cookie-checked HWND protocol rather than public window mode.
- [done] Public native/runtime regressions, ownership/lifetime/recovery,
  structure guards, independent Standards/Spec review, appropriate Windows/WSL
  validation, archived prototype and local commit.

Evidence: `D:/Dev/shots/cgpui-dx12-nc-alpha-20261123/`.

## Chosen production boundary and source freeze

Only initially transparent CGPUI-owned ordinary HWNDs with decorated visible
system captions on Windows build >=26100 are enabled, contingent on actual DWM
success. Opaque HWND flip semantics remain unchanged. Opaque-to-transparent
intent changes and same-HWND recreation after a flip attempt remain unsupported.
No all-composition opaque default, client chrome or fullscreen alpha support.

Implementation ownership: private platform alpha lease and token protocol,
nonclient menu RGB-preserving repair leaf, private DX12 RAII token lease. No
public presentation mode, renderer dependency on concrete platform windows, or
raised module caps. Native GPU retirement precedes lease release; recovery retains
the lease; stale tokens cannot affect a newer owner. Posted payloads carry no
borrowed address. Alpha operations remain UI-thread-bound; opaque history alone
uses a bounded cross-thread handshake while preserving old opaque factory use.

Prototype archived on local `codex/prototype-dx12-native-menu-alpha` at
`65fc087703985e7070e32cade39d8b42b5482035`. Four probe files and temporary suite
registration/uxtheme test-link workaround removed from production. The production
platform package owns uxtheme link closure.
(That branch was deleted on 2026-11-25 without merging; its source is gone.)

Review found and fixed the pointer-message lifetime vulnerability, accidental
opaque-worker factory restriction (test RED exit3 then GREEN), and older-Windows
positive-test assumptions. Nine alpha positive tests now require actual
unsupported rejection before skipping on <26100; this host is build26300 and
never takes that skip. Opaque history tests run without an OS skip.

Focused final checks: 11 ordinary native tests passed with WARP+Debug, plus
platform lease and structure tests. Native menu comparisons remained exact.
Source snapshot: 2758 files, excludes `.planning/` and user `.vscode/`.
Full verification passed: Windows732, WSL640, architecture205; Intel/NVIDIA/WARP
native64 each, native ASan64, installed Windows SDK compile/link/run, SDK fixtures
4+15. Windows raw and WSL normalized source matches cover2758 files. See
acceptance.md for boundaries and evidence. No push; user `.vscode/` preserved.
