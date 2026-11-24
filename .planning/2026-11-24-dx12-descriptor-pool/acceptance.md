# SRV pool acceptance

Baseline1775cb6d; per-state native-epoch pool, stable4096-slot pages and move-only
one/three-slot leases. New focused private allocator/page/native-factory owners;
no public configuration, backend/default, Popup, pacing, sampler/RTV or shader
policy change. Three blur SRVs remain on one page for alpha composition. Unknown
GPU completion retains owners. Empty high-water pages persist until retirement;
this is not a total-memory bound or a binding/draw-count optimization.

## Actual semantic and focused gates

External evidence: D:/Dev/shots/cgpui-dx12-descriptor-pool-20261124/.
- A3068path source matched reviewed overlay. Two real scenes on Intel73219,
  NVIDIA74481 and WARP74333: exact pixels/diagnostics first, then actual native
 8views/8heaps or14views/10heaps failed the required1heap predicate, exit2.
  Debug all; GPU validation also WARP. Six native REDs and binaries retained.
- Incremental B preserves both scenes/oracles; same six native runs exit0 with
  8/14 unique descriptors sharing1actual heap. Initial B build caught one absent
  private declaration include; fixed in recovery test only, original log kept.
- WARP/debug/GPU-validation exact16new tests pass, including4097growth, atomic
  fragmentation/failed growth, immutable history, blur/pending/resize/removal/
  orphan lifetime and transaction failure. Synthetic completion controls are
  not represented as natural hardware failures. Full architecture233 passed.
- Independent Spec review found two coverage gaps (fragmented2+1 free slots and
  invalidated cache with both historical frame owners). Added focused assertions,
  real empty-frame captures and retained-payload8/4/0 evidence; rebuilt16pass.
  Original review retained; repair addendum0. Independent Standards0, separate
  from candidate author's own0review. No production changes were needed for these
  coverage repairs; candidate/source review never substitutes for compilation.

## Frozen full regression, with explicit fixture supplement

Final3093source manifest SHA256:
7eef66d4efa6bbb2db74e2637b4bf08f5ce6296b5fc54b5e85bff28fb5388cdc.
Full bounded driver exit0,2326s. Windows886/WSL683 and both all-member builds;
native156 per hardware0/hardware1/WARP and ASan; common107 per selected backend,
ASan107Vulkan+107DX12. Native/common exact inventories, no skip, six-phase Vulkan
missing-image native command order/reuse trace, source consistency, SDK graph9/
headers15, runner3 and installed consumer all passed. WARP GPU validation is
separate from ASan, which disables GPU validation. WSL is neither ASan nor Mac.

The initial audit deliberately failed: C: was exhausted, so root redirected
temporary build/fixture files to D:. D: lacked distinct short aliases, causing
the two real-short-path subtests to skip (PS5reported4skips;PS7reported2). This
was NOT accepted. After OS-released space became available, restored actual
original system TEMP and reran BOTH complete17-case PowerShell suites on the
same freeze: PS7all17,PS5exact15+2previously allowed PS7-only skips. No cache/SDK/
user-file deletion. Strict combined audit passed; its record explicitly says
initial_matrix_all_green=false. Old logs/audit remain. A rejected supplemental
source-check CLI argument attempt preceded all fixture reruns and is preserved;
corrected before/after source checks bind the actual rerun.

The external audit itself also had a preparation defect (initially rejecting
the established two PS5-only skips). Fixed before the full execution; original
version retained, historical positive plus five wrong-skip/name/duplicate/summary
negative controls passed. No broader fixture/native skip permission was added.

Installed SDK uses refreshed actual /MT libraries and copied headers: real
opaque/Popup/client-alpha/orphan/history/batched/missing-image scenes and owned
invalid app-local Vulkan loader all pass. CMake remains configure-only. SDK
hardlinks are mutable, not an immutable or Release distribution.

## New optimized build and measured comparison

Real post-B Release build succeeded;371effective TUs all /O2 /DNDEBUG /MT,
no/Od/sanitizer, all three new SRV objects in the final probe link closure.
Unchanged21harness/runner files; before/after/final full-source checks pass.
Old baseline executable was preserved before rebuilding.

New executable SHA96bba9090f87d36e49078177b2789f5792140f12cbcaf46c09da5d39dd6d2d7e;
manifest SHA0c48c46b77e893d0d80109284d69aa109b965bbb402e6fca1a4e606f5b9563a5.
The revised external provenance verifier passed all371TU flags/graph/source
checks. Its original preparation gaps (dash-prefixed conflicting options and
duplicated run folders) were independently found and repaired before use;
historical368TU/text/row-identity fixtures passed. The unchanged raw auditor
still correctly calls this artifact consistency, not execution attestation.

All18fresh runs passed:3scenes x2backends x3repeats, IntelLUID73219,60Hz,
warmup120/samples600,10800steady+2160warmup. No reruns, discarded rows or outliers.
Pre/post actual pixels, stable per-run display and zero steady uploads pass.
Run identities/raw hashes and original launch evidence are distinct and bound.
Six actual negative controls pass: wrong LUID rejected at preflight, hide/move
rejected during measured_no_capture for both backends; no native timeout. New
optimized NVIDIA/WARP20sample mixed smokes audited separately, not pooled with
Intel timing statistics.

Frame-API distributions remain close to the pre-SRV baseline: DX12p50 roughly
15.5ms/p95roughly30ms; Vulkan roughly16.6ms. Heap topology reduction is established,
but no material FPS/GPU-time/latency improvement is demonstrated. These are
wall-clock API envelopes with waits, not GPU timestamps. The cohorts are temporally
separate, and the pre-series paused after4runs for an external auditor correction.
Historical power-scheme GUID was not retained; current observed scheme is Silent,
AC/local-session. Power/thermal/background confounds cannot be excluded. Toolchain
resolution reports ucrt10.0.26100.0; stored metadata omits SDK-header root, so no
header-root identity claim is made.

## Remaining goal

Independent offline comparison: post-release-actual/comparison.md SHA256
2e4b4ab532b9619ac839c9199088290268500c03020c6fcb3f9ed6683e549708;
comparison.json SHA256
79a5043a626cd89cac311f96c2f5b9a1e8d860dc1004014d25fb253c28c6c35b.
All36runs/21600steady samples and180raw-file hashes checked, no trimming.
Local Present reductions coincide with increased acquisition waits in two scenes;
neither stage quantiles nor different backend envelopes are additive/equivalent.
Precommit independent full-source check still matches all3093frozen paths.

This slice is accepted on tested configurations. Full DX12 is still ACTIVE:
glyph dirty native transport, command reuse, window/recovery/DRED/CI/release/
default readiness gates remain. External candidates are not implementations.
No push or release publication authorized/performed.
