# Orphan-frame review

Baseline `d893fdee`; actual tracked and newly added source, not only a patch.
Root alone runs engineering build/test/GPU. All reviewers were read-only.

## Standards

`/root/capture_standards_review`: 0 confirmed actual-source findings. All29
production/test/doc files match the reviewed StageB with normalized newlines.
Focused private orphan owner and borrowed synchronization constructor seam;
facade only forwards. Renderer86/100, orphan header11/cpp45, existing caps not
relaxed. Five prior rejection tests retain other assertions; discovery is exact.
Capture/presentation/completion and retention order unchanged, no old aging call.

## Spec/native lifetime

`/root/capture_boundary_review`: 0 confirmed actual-source findings. Both
preflight/late failure dispatch orphan first; core ownerless guard blocks any
release-and-rebuild path. Local retirement requires actual/trusted removal and
operation depth<=1, while live incomplete work remains owned. Alpha token,
thread protocol and native-before-alpha final teardown are retained.

Private borrowed synchronization interface outlives every frame/state and
teardown in the test. Late fault uses genuine Signal, queue/fence canonical
device identity, real RemoveDevice, real UINT64_MAX completion sentinel and
failed device query; no fabricated success/skip or persistent COM reference.
The failed orphan remains alive while its handle is already closed and a live
peer reconstructs and captures green. This does not claim hardware TDR or that
GPU work was necessarily still executing when the removal was requested.

Cold Popup observes production-only first Commit and new desktop bands; managed
alpha observes new red-over-green content with unchanged occupied lease/topology.
Five old tests migrate only healthy orphan rejection; removed/purged cases still
reject, and unrelated overlap/resize/pixels/pacing/alpha assertions remain.

## Root execution before freeze

Actual StageA native WARP/debug setup succeeds then both destruction-order cases
exit40; common identical Vulkan test passes while DX12 exits40. After B all5
native orphan tests pass, including cold desktop/late native removal, and common
Vulkan/DX12 both pass. Full structure228/native136 WARP-debug pass. Full frozen
matrix subsequently passed first run exit0: Windows846/WSL671, native136 per
adapter, shared96 per selection, ASan136/96+96 and SDK. Exact inventories/source
matching pass; see acceptance for limits.

## Installed consumer review

The external verifier's installed_orphan_frame creates an independent opaque
HWND, verifies requested actual DX12, acquires then deletes the facade, records,
requests capture and submits once. Repeated Present/capture fails. Lambda-scoped
frame/state cleanup precedes HWND destruction on every ordinary return; errors
propagate. Independent source check0. No dead getter or pixel claim. Actual
installed /MT compile/link/run now passed both ordinary and owned-invalid-Vulkan
DLL runs, each logging exactly-one successful orphan completion and capture request.
