# Prepared follow-up work (not implemented)

## Current status correction (2026-11-24 follow-up)

The original candidate inventory below is historical, not the current status of
every item. Capture publication (`140cde6b`), retained-image diagnostics
(`d893fdee`) and healthy acquired orphan-frame completion (`58e16f71`) are now
implemented and accepted in their dedicated planning directories. Capture
history is applied and undergoing its second frozen full matrix; the first
native-menu fixture timeout and unchanged successful replays are retained.
Image batching has current reviewed rebased A/B candidates, not yet applied.

DRED's original candidate is **not safe to integrate as-is**: the later
`D:/Dev/shots/cgpui-dx12-dred-20261124/candidates/retirement-boundary-review.md`
establishes device-wide borrowed-node release-ordering gaps. Per-state gates
and moving one origin hook do not establish the required lifetime. This is an
ordering finding, not a demonstrated UAF. A correct device-epoch capture-once
transaction or bounded no-traversal initial scope is still needed.

Performance work has a source-reviewed revised manual benchmark, still unrun:
`D:/Dev/shots/cgpui-dx12-performance-20261124/candidates/revised/`.
Recorded-command reuse, shared SRV pool and dirty glyph native-copy work remain
missing; staging/pacing evidence does not measure FPS or input latency. Shared
SRV candidates are being prepared outside the frozen repository, with atomic
same-page blur tuple leases and idle/removal-safe recycling.

The native/common counts below describe the old candidate baseline only; derive
current inventory from manifests before executing CI or final matrix scripts.

2026-11-24. Root continues the active complete-DX12 goal. These external candidates
were prepared while the composition source was frozen; no build/GPU/application
claim follows from their existence or static review. Do not overwrite current
manifests/docs with candidate copies; apply reviewed increments after acceptance.

## Diagnostics and successful-frame publication

- `D:/Dev/shots/cgpui-dx12-dred-20261124/candidates/README.md`:
  private bounded DRED summaries, settings-before-device and capture-before-release,
  per-native-lease rather than public-generation deduplication. Independent
  Standards/Spec0 confirmed findings; no actual execution. Device-wide borrowed
  nodes across other-renderer teardown remain an explicit unverified boundary.
- `D:/Dev/shots/cgpui-dx12-image-retention-20261124/candidates/README.md`:
  agreed optional retained-image CPU payload plus separate saturation; all-live
  immutable RGBA snapshots, weak identity tracking, post-aging success publication,
  sticky unavailable on tracker allocation failure. Independent source review0.
  StageA/B patches prepared but not run; OOM branch is source-reviewed only.
- `D:/Dev/shots/cgpui-dx12-public-audit-20261124/candidates/README.md`:
  late debug/Commit failure can currently expose a capture candidate already
  assigned by readback. Proposed native InfoQueue-message regression and staged
  pixels until success are unexecuted. Public headers do not already promise
  success-only pixels: make this DX12 safety strengthening explicit, not a
  fabricated neutral historical contract. Consider alongside the diagnostic
  completion owner, retaining strict separate RED/GREEN evidence.

## CI

`D:/Dev/shots/cgpui-dx12-warp-ci-20261124/candidates/README.md` and `review.md`.
Exact inventory/log/identity/evidence checking, read-only prerequisites and two
manual experimental lanes. Reviewer found prefix-log false-green and insufficient
self-hosted authorization assumptions; both source/document fixes re-reviewed0.
No fixtures/native/remote execution. Actual server-side fixed workflow/ref runner
group restrictions, provisioning and host acceptance are mandatory and unverified.
Current inventory122 native/94 shared must be updated after reviewed test additions.

## Public-operation contract audit

`D:/Dev/shots/cgpui-dx12-public-audit-20261124/audit.md` inventories every current
Renderer/RenderFrame operation against both owning backends and tests. The common
thirteen pixel scenes do not cover image upload/invalidation ordering, facade/frame
lifetime or capture-history differences. Existing explicit tests differ between
backends; do not mechanically declare DX12 wrong or silently rewrite Vulkan.
The neutral contracts and cross-backend regression obligations still need closing.

## Window route and cross-thread recovery

- `D:/Dev/shots/cgpui-dx12-window-route-20261124/candidates/README.md`:
  manual fresh managed system-caption/HMENU opaque HWND vs premultiplied/IGNORE
  composition probe. GPU/desktop alpha and real native-menu oracles prepared;
  results all UNMEASURED. No repeat of rejected HWND-flip-to-DComp recipe. IGNORE
  uses a test-only manual Commit after diagnostics: not production transaction,
  recovery, default route, or dynamic transparency proof.
- `D:/Dev/shots/cgpui-dx12-cross-thread-recovery-20261124/plan.md`:
  official DComp facts say not thread-bound; the alpha token protocol and DXGI
  message-pump/deadlock concerns are separate. Proposes tests-only idle foreign
  native retirement before any guard change; busy/asynchronous coordination needs
  an explicit pending protocol. Existing begin-only frame_not_ready and completed
  recovery retry semantics must not be repurposed. Root/child resize error handling
  differs and needs its own contract/tests before asynchronous extension.

Default/auto remains Vulkan; explicit DX12 never secretly falls back. No new
application-facing window mode, hidden native-menu replacement or backend Popup
blacklist. Preserve native-first Popup, failure-only Overlay, strict module caps,
source-consistent acceptance, user's .vscode and the separate user/design checkouts.
