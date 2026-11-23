# DX12 milestone: same-UI-thread coordinated device recovery

Baseline `fd125c63`, branch `phase-v`; continuation requested. Preserve user
`.vscode/`, separate design checkout and user's Linux checkout. Local commit
only; no push. Vulkan/default and Popup fallback policies remain unchanged.

## Contract

On actual native device-removal evidence, retire eligible peers on the same
adapter LUID and HWND-owning thread before rebuilding the triggering renderer.
Peers remain recovery_pending with unchanged generation until their own next
operation completes one bounded rebuild. Cached removal evidence or pending/null
native state must never trigger another purge of already healthy peers.

The private registry holds weak metadata only, never a native device reference.
Its lock covers metadata snapshots, not native calls or state destruction. Each
state has a private recursive operation gate. Public Renderer/RenderFrame entry
points serialize state access and track active operation depth (including
same-thread reentry); coordination uses try_lock for peers and never
waits for a busy peer. Cross-window-thread peers, busy peers and external native
holders remain actual bounded replacement blockers, not fake typed retry.
Confirmed/pending recovery nested inside the triggering renderer's own operation
returns ERROR_BUSY before mutating recovery state or native ownership. Peer
retirement also holds the activity guard throughout native release.
This adds same-thread multi-window recovery, not arbitrary cross-thread rendering
or atomic recovery of every window. No creation-thread restriction is added to
individual renderer calls, and no lock is held globally across GPU/DXGI waits.

Native-only peer retirement preserves CPU caches, dimensions, frame_active and
capture storage. Pending peers report no current capture, but borrowed pre-loss
CPU storage remains alive until that peer's own next mutating operation. The
existing pointer interface is still not a concurrent-use guarantee.

An outstanding CPU frame remains outstanding until presented or abandoned; do
not forcibly clear frame_active on peer retirement. Its present preflight
rebuilds if needed and requests a new frame, without submitting across generation.
Finished-frame destruction remains idempotent. owner_alive=false states held by
orphan frames participate in native retirement but are never rebuilt for peers.
Each renderer increments generation only after its own full successful rebuild.

## Ownership and validation seams

- gpui_windows_design: private weak registry, cohort cleanup, recovery integration,
  state gate/thread metadata and native cleanup preserving CPU capture.
- popup_structure: frame entry gates, captured generation and pre-submit preflight.
- repo_progress: public native two-renderer tracer and later lifetime/blocker tests.
- root: Renderer gates/factory registration, preflight helper, structure tests,
  all registrations/docs/builds/tests, source freeze and commit.

Use public Renderer/RenderFrame/real RemoveDevice/pixel capture seams plus private
operation-gate tests only if required for deterministic busy-peer verification.
All builds serial: MCPP_JOBS=4 no-run,1 execution; use mcpp wrappers. No cap raises.
Evidence: `D:/Dev/shots/cgpui-dx12-cohort-20261123/`.

## Sequence

1. [done] Same-thread two-renderer public tracer RED.
2. [done] Focused coordination and frame invalidation GREEN.
3. [done] Orphan/capture/lazy/foreign-thread/blocker/reentrancy regressions and reviews.
4. [done] Frozen complete Windows/WSL, native adapters, SDK and ASan checks.
5. [done] Acceptance recorded; local commit closes this milestone (no push).
