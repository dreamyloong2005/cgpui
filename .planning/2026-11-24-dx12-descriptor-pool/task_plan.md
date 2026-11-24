# DX12 SRV descriptor pool

Baseline: 1775cb6d. Overall DX12 goal remains ACTIVE. Local commits only.

## Contract and ownership

Implement a per-Dx12State, native-epoch-local shader-visible SRV pool with
stable 4096-slot pages, move-only leases and allocation-free slot return.
Images/glyph snapshots own one slot; a blur group atomically owns three slots
on the same page. No CPU/public snapshot owns native leases. Existing
idle/removal/abandonment proofs govern native retirement; unknown GPU completion
retains owners. No extra frame rewind, public option, backend/default change,
Popup policy change, sampler/RTV policy change or bind-elision optimization.
Empty high-water pages remain until native retirement; 4096 is not a total cap.

## Acceptance sequence

- [complete] Preserve and hash the accepted pre-SRV optimized baseline binary.
- [complete] Apply test-only A, build and run both exact native scenes on
  hardware0/hardware1/WARP with debug (WARP GPU validation). Pixel/diagnostic
  success must precede real topology8/10 heaps, then the intended exit2.
- [complete] Apply only incremental B, rebuild, same-oracle GREEN and all16
  named SRV tests; architecture233 remains green with all existing caps.
- [complete] Independent actual-source Standards and Spec review.
- [complete] Freeze sources; full Windows886/WSL683/native156/common107 matrix,
  adapters/debug/ASan, fixtures and installed-consumer SDK/source audit.
- [complete] New optimized post-B build/provenance and comparable18-run cohort,
  actual negative controls, raw-output audit and honest comparison.
- [complete] Acceptance records and precommit frozen-source check, no push.

## Evidence

External design, reviewed candidate and detailed execution plan:
`D:/Dev/shots/cgpui-dx12-descriptor-pool-20261124/`.
Candidate `candidates/rebased-baseline/`: A SHA99190b102876c504df2658aa6f2fe7c43a1efd6dc7ea7c8db9ec13ba70b795e4;
incremental B SHA25c1ccf7a3db435eecba48afa30e487a231174a6c5b7ffb69ffd8de321cdb8c5.
No source-only review, candidate existence or heap-count result is an FPS,
GPU-time, peak-memory bound, all-platform certification or full DX12 completion.
Old baseline binary was verified SHA9745a3bb54239698c51901d484f8e9a19286a89fc58fd6d219f8def05211a6e5
and copied to the prior performance evidence's release-provenance/preserved-binary.

## Executed progress (not final acceptance)

- A compiled, source3068 matched the reviewed overlay. Both scenes on Intel
  LUID73219, NVIDIA74481 and WARP74333 passed pixels/diagnostics then failed only
  topology:8views/8heaps and14views/10heaps, each expected1/exit2. Debug enabled
  for all, GPU validation additionally for WARP. Binary hashes stable per run.
- B's first build caught a missing direct private declaration include in
  dx12_srv_recovery_test.cpp. Added dx12_frame_recovery_internal.hpp, not a broad
  umbrella include; no production/oracle change. Original failure retained.
- Rebuilt B: both identical scenes pass on the same three adapters with one
  heap and8/14 unique handles; WARP/debug/GPU-validation16 focused tests pass.
  Complete architecture233 passed. Independent Spec review then identified two
  required coverage gaps (not production defects): cross-page2+1 fragmentation
  and cache invalidation while both historical frames survive. Added focused
  assertions, real empty captures and payload8/4/0 checks, rebuilt and reran all16
  under WARP/debug/GPU validation successfully. Original review and initial
  pass are preserved, not relabelled as covering these additional assertions.
  Full matrix and post-SRV performance are still pending.

## Frozen matrix and environment repair

Final3093source freeze: manifest7eef66d4efa6bbb2db74e2637b4bf08f5ce6296b5fc54b5e85bff28fb5388cdc.
Full driver exited0 in2326s: Windows886/WSL683, all builds, native156 per
hardware0/hardware1/WARP plus ASan, common107 per selection/ASan both, SDK.
Strict initial audit correctly rejected extra driver-fixture skips: to keep
temporary build files off exhausted C:, root redirected TEMP to D:, whose volume
did not expose a distinct8.3 alias. Two real-short-path subtests were skipped.
No production failure or oracle change was inferred from that environment issue.

After the OS independently freed a small amount of C: space, restored the actual
original system TEMP and reran both full17-case PowerShell fixture suites on the
same frozen source. PS7:17pass; PS5:15pass+the two established PS7-only skips.
No extra caches/SDK/user files were deleted. Original skipped logs, failed audit
and a rejected supplemental source-check CLI argument attempt are preserved.
The latter preceded fixture execution; corrected before/after checks bind the
real supplemental rerun. Combined strict audit passed with exact39member/name
inventories and native/common/reuse/SDK checks unchanged. Evidence explicitly
states initial_matrix_all_green=false, not a fictitious clean first attempt.

New optimized Release succeeded in21.39s,371 effective TUs all /O2 /DNDEBUG /MT,
three new SRV objects reach the actual probe. Binary96bba9090f87d36e49078177b2789f5792140f12cbcaf46c09da5d39dd6d2d7e;
manifest0c48c46b77e893d0d80109284d69aa109b965bbb402e6fca1a4e606f5b9563a5.
Source and graph verification passed. Independent current desktop observation
confirms Intel cohort display2560x1600/144DPI/60Hz, local AC. Historical power
scheme GUID was not independently retained; temporal/power confounds remain.
The stored toolchain resolution identifies ucrt10.0.26100.0 but does not export
the SDK-header root; no unsupported SDK-root identity is claimed. All18fresh runs
passed, with no retry/discard; six native negative controls and separate optimized
NVIDIA/WARP smokes passed. Independent offline comparison checked36pre/post runs
and21600steady samples,180raw-file hashes and18distinct post identities. Present's
local reduction in two scenes coincides with higher acquisition wait; whole-frame
distributions remain close. No causal speedup or full-DX12-completion claim.
See acceptance.md and external post-release-actual/comparison.md/json.
