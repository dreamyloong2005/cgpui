# Frame outlives renderer facade

2026-11-24. Baseline `d893fdee`. Overall DX12 goal active; root-only engineering
execution, local commits/no push. Preserve .vscode and other checkouts.

Authority: original DX12 plan explicitly names vulkan_frame_lifetime_test for
dual-backend acceptance; existing parity ledger preserves frame-outlives-facade.
Close that compatibility gap without treating facade deletion as permission for
ownerless reconstruction. Native window ownership remains the caller's duty.

Boundary: already acquired unfinished frame can record, request capture and
complete once on its healthy existing native epoch/live surface. Shared state
continues to own native objects/alpha lease until the final frame owner dies.
owner_alive still gates reconstruction. Focused orphan preflight and late failure
paths reject removed/purged epochs without retry/rebuild. Proven removal permits
safe local retirement; live incomplete GPU work remains conservatively owned.

No image batching/capture history policy change in this slice. Preserve accepted
retention completion and success-only candidate publication. User-approved
capture/image semantics follow separately, with exact common tests.

External candidate:
`D:/Dev/shots/cgpui-dx12-public-audit-20261124/orphan-candidates/revised-full/`.
Independent source Standards/native-safety/Spec reviews0; not runtime evidence.

1. Apply only tests-first A; actual native named-DX12 and common selected DX12
   orphan Present must produce semantic exit40 after healthy setup. Shared Vulkan
   oracle should pass; do not call build/setup failure a RED.
2. Apply B focused orphan module plus private nullable borrowed synchronization
   seam (facade forwards, state constructor stores). Preserve all existing module
   caps; lifetime of test API extends through every state/frame cleanup.
3. Run A GREEN and expanded tests: preflight removal, unique late debug error,
   actual post-Present/Signal same-device RemoveDevice and real fence sentinel,
   first-Commit cold Popup desktop pixels, managed-alpha lease and desktop color,
   migrated five old rejection tests retaining unrelated assertions.
4. Architecture/native/shared regressions; actual-source independent reviews.
   Freeze and full Windows/WSL/hardware/WARP/debug/GPU-validation/ASan/SDK matrix,
   exact inventories/source matching, then acceptance/local commit.

The late native fault proves ordering after real submission, not work necessarily
in flight or hardware TDR. IsWindow detects current numeric HWND validity, not
a new guarantee against application-destroyed-and-reused handles. Existing alpha
instance/cookie/thread protocol remains intact; no new thread-safety promise.

## Observed progress

- Stage A built successfully. WARP/debug native actual orphan test fails exit40
  after healthy warmup in both destruction-order cases. The exact shared contract
  passes Vulkan/Intel, fails DX12/WARP exit40 with renderer-destroyed error.
  Logs in `D:/Dev/shots/cgpui-dx12-orphan-frame-20261124`; no setup failure counted.
- Applied B after fresh check: all5 native orphan tests GREEN under WARP/debug,
  including true late native removal and cold first-Commit desktop bands. The
  exact shared Vulkan/Intel and DX12/WARP contract both pass. Native warmed
  dual-destruction-order cases now report success1. No final acceptance claim.
- Full structure228/228 and native136/136 WARP-debug passed. Actual-source
  independent final Standards/Spec-native-safety reviews0.
- Frozen2997path matrix passed first run: Windows846/WSL671, all builds,
  native136 per adapter, common96 per selected backend, ASan136/96+96 and
  installed SDK including orphan and retention contracts. Exact inventories and
  source matching pass. Slice accepted; user image/capture work remains separate.
