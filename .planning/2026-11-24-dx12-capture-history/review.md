# Capture history review

Baseline `58e16f71`. Human-approved Vulkan compatibility, not an inferred policy.
Actual tree includes one new shared exact test and narrowly scoped owning edits.

## Standards

`/root/capture_standards_review`: 0 confirmed actual-source findings. Production
only drops frame pre-clear and conditions successful assignment on engaged
candidate. Leaf comments document optional support and stable-surface history.
Caps retained: presentation96/110, renderer86/100, late test128/140, shared89/120.
Orphan/cache/retention/completion logic unchanged; no frame-local aging restored.

## Spec/native safety

`/root/capture_boundary_review`: 0 confirmed actual-source findings. Readback,
debug, Commit and successful diagnostic completion still precede nonthrowing
pixel publication. Cold-null and warm deep previous-success checks, exact single
InfoQueue error scan, raw failure and live same-generation assertions remain.
No-capture success keeps history; new captured success replaces it. Resize,
recovery and pending-peer invalidation remain explicit and unchanged. No generic
custom-renderer or native-Mac runtime validation claim.

## Root execution

A actual Vulkan/Intel passes; DX12/WARP succeeds in the preceding captures and
uncaptured native frame, then exits8 on lost history. B exact shared both passes;
actual late InfoQueue cold/warm both pass. Logs retained independently. Whole
structure228/native136 WARP-debug pass; full frozen matrix pending acceptance.

External SDK verifier adds owned deep-copy comparison over uncaptured success
and requested invalid-stride failure, with same generation and subsequent real
blur/capture. Independent source check0: no borrowed pointer reuse, failed frame
dies before next acquire, all errors propagate. Actual installed execution is
pending the full driver; no orphan pixel getter or native Mac claim introduced.

## First full run: preserved native-menu timeout

Windows workspace846pass/1fail: ordinary_menu_cancel_test exit4 in its opaque
reference before any DX12 renderer construction, BEGIN then timeout without OPEN.
The exact isolated command passed once plus all5 bounded unchanged replays.
No source/timeout/oracle change or skip. Root cause is unestablished; current
logging cannot distinguish popup discovery from highlight/pixel-stability wait.
This is recorded as a remaining menu-fixture reliability gap, not declared fixed
and not evidence that capture history caused the timeout. Original final/ logs
remain intact. A fresh complete run uses verified/, with same source fingerprint.

## Final execution

Second frozen full driver exit0, exact native136/common97 inventories and2998
source paths verified. Windows847, WSL671, adapter/debug/GPU/ASan and installed
SDK checks pass as recorded in acceptance.md. This does not resolve or erase the
first native-menu timeout. No new source changes were needed for the second run.
