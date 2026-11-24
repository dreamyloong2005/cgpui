# DX12 glyph dirty transport — acceptance record

Date: 2026-11-24. **Accepted after the second frozen matrix and reviewed audit repair.**
Overall DX12 completion remains ACTIVE. This slice does not change the default
backend, public API, Popup/window policy, shader/gutter semantics or scheduling.

## Implemented boundary

- Private CPU lineage seals immutable page-local prefixes against weak snapshot
  identity. Validation uses fresh planner state for every suffix record.
- A compatible prior-completed native prefix with `0 < D < P` creates a new
  destination, clones the complete page on the GPU and uploads only dirty regions.
  Both retained and released predecessors use copy-on-write, not in-place mutation.
- Exact native hits perform no transport. Cold, unavailable optional lineage,
  incompatible/backward and valid dense updates initialize the complete page.
  Invalid source or inconsistent sealed metadata remains an error.
- GPU completion (G) and parent command-list reset/release (P) are independent.
  Completed publication never frees recorded dependencies. Detached map nodes
  keep their unique descriptor leases until P; failed Reset/unknown completion
  retain ownership.
- Full CPU snapshots/history remain. GPU clone work and copy barriers are not
  omitted from the accounting. No FPS, GPU-time, total-memory or RSS gain claim.

## Original focused evidence

External evidence root:
`D:/Dev/shots/cgpui-dx12-glyph-dirty-20261124/`.

- CPU suffix-state mutation compiled, failed for the intended second-region
  regression, then exact-byte restoration passed all four CPU tests and structure.
- Original ungated readback rejected a real WARP InfoQueue injection (exit6).
  Corrected unchanged oracle then checked Intel73219, NVIDIA74481 and WARP74333:
  injection controls passed; held and separately released predecessor REDs first
  proved pixels/full-page behavior, then returned the specific transport exit40.
  Incremental implementation passed all nine corresponding native rows.
- Deliberately skipping only the second real CopyTextureRegion produced the
  expected native pixel mismatch, exit7, despite untouched accounting. Exact
  restoration/rebuild passed all nine native rows again. The original inaccurate
  wrapper expectation and corrected audit of the same run are both preserved.
- Latest24glyph tests pass on WARP/debug/GPU-validation. Final238architecture
  tests pass without weakening existing caps; presentation is98lines under110.
  Earlier235/237structure results and preliminary179native run are retained,
  not substituted for verification of the later source.
- Lineage metadata allocation RED:4096records/4096ordinary allocation requests/
  67,265,063requested bytes. Checked geometric reservation GREEN:4096records/
  13requests/65,684bytes, with fault/retry and prefix/mapping assertions intact.
  These are cumulative allocation requests, not retained bytes or measured copies.
- Missing-lineage/family/invalid-seal controls exercise private synthetic input
  boundaries. They do not establish normal public-frame packing behavior.

## Independent review and final source

- Actual-source Standards0/Spec0 across81paths. Author-owned fallback controls and
  growth deltas were excluded from self-approval and separately independently
  reviewed. Reports and exact inputs are in `stage-b-actual/`.
- Freeze:3158source files at HEAD91d9f10eb38d41743169dd2cf116ab8b3966114b.
  `final-freeze/freeze.json` SHA256:
  `3bcb084890fd5f5c7f4e9668389d0e1fcac0e49f969d701abed574bdaabc090c`.
- Frozen inventory: Windows915, WSL688, native180, shared107, architecture238;
  all39workspace member identities/order preserved. Root resumed-source check
  confirmed every source byte and registration against the freeze.
- Final runner bundle SHA256:
  `a1a0077ccff2c222fa3a368b6afc5606182aaeb1ca8102d5d62ee4f63115b86a`.
  Independent static review0; actual root syntax checks and26log-parser plus
  10final-production-graph synthetic controls pass. These are tool controls,
  not native matrix results.

## Final matrix — accepted; original failures preserved

Initial evidence: `final-attempt-1/`. Root exclusively owns serial builds/tests/GPU.
The actual original system TEMP directory/8.3 alias probe passed while reporting
0free bytes; it imposes no arbitrary free-space threshold and is not a substitute
for the complete17case PS5/PS7 fixture suites. No cleanup preceded that attempt.

Full Windows no-run build, all39member build and915tests passed. The pipeline
then failed before WSL execution: backslashes in Windows paths were stripped at
the WSL command boundary. A minimal actual path-conversion RED/GREEN proves the
forward-slash repair. Original bundle and failed attempt are retained unchanged;
the separate pathfix bundle changes only the three wslpath arguments and its
manifest. Fresh full-matrix rerun is pending. This is not an accepted matrix or a
product-source failure, and no successful early phase is relabelled as full pass.

Pathfix independent review0; new bundle SHA256
`72fb1d29aec85079619b80a5973b05f557d06cd2ad6a68efadcc3e0cbeb71f72`.
At the next checkpoint in fresh `final-attempt-2/`, Windows915 and both Windows
builds had passed; owned WSL sync/source check passed and WSL build was underway.

During attempt2 the user explicitly authorized clearing unused old compilation
temporary caches. Root removed172old `preamble-*.pch` files directly under the
actual user TEMP, using an exclusive nonshared file handle with delete-on-close;
three in-use files were preserved. Deleted logical bytes2,545,245,892; observed
C: free space rose from0 to2,532,376,576. No recursion, source/SDK/chat/database
deletion or cache redirection. Exact inventory and per-file outcomes are retained
in external `authorized-temp-pch-*.json*`. This is environment maintenance, not a
product change or a fixture-skip exemption.

D: later approached full during ASan linkage. Root applied reversible NTFS
compression to inactive generated normal-build objects and `.exe`/`.lib` files
under the exact `target/x86_64-windows-msvc/bf1926899b8acd1e/{obj,bin}` trees,
not to the active ASan tree, sources or evidence. No files were deleted and
logical contents are unchanged. compact reported1810objects (2,596,302,450logical
bytes ->1,052,122,467stored) and967binaries/libraries (4,061,883,334logical bytes
->2,140,476,164stored); observed D: free space then3,465,682,944bytes. These are
filesystem storage figures, not renderer memory/performance results. Logs are
`normal-object-ntfs-compression.log` and `normal-binary-ntfs-compression.log`.

At that checkpoint remaining results and strict exact-inventory, source-pre/post,
adapter, fixture and fresh SDK-graph audit were pending. Final results follow;
prepared scripts, prior milestones or focused runs are not substituted for them.

Required: full Windows/WSL builds/tests; native0/1/WARP debug with WARP GPU
validation; both shared backends for each adapter; WARP ASan native/shared;
original-system-TEMP driver fixtures; SDK graph/header/runner controls; fresh
production `/MT` graph bound to the actual installed SDK library and consumer.
Installed headers/static libraries and CMake configure-only do not certify a
release package. WSL does not certify native Linux desktop/GPU or macOS behavior.

Final `final-attempt-2/` pipeline exited0 in2382.39seconds. Full Windows915 and
WSL688 tests, both all39member builds, native180 on Intel/NVIDIA/WARP debug
(WARP GPU validation), shared107 on both backends for each adapter, ASan180 and
ASan shared107+107 all pass exact inventories with no suite skips. The original
system-TEMP driver fixtures pass17onPS7 and15+exact2PS7-only skips onPS5;
no short-path skip is accepted. SDK graph9/header15/runner3 controls pass.
Fresh production `/MT` graph contains all11glyph production objects in the actual
DX12 archive and matches the installed consumer's library. Source checks match
3158files before/after Windows and normalized WSL execution.

The first final audit failed, honestly retained in `matrix-audit.log`: it required
the SRV identity syntax even for ASan, whose unflushed SRV stdout did not survive
termination. This is not evidence of a wrong adapter. Independent owning-source
review verified two existing flushed FIRST_BEGIN records from real DX12
RendererBackendInfo, including explicit selection/no fallback/debug checks.
The separate audit-only revision requires exact visible/hidden test-output blocks,
their complete lifecycle traces, expected LUID/software/generation and all SRV
records if present; non-ASan retains the original SRV gate. All other original
audit logic is unchanged. Producer bundle, binding, source and logs remain intact.
18pure positive/negative controls and independent review0 preceded the complete
corrected audit. This is same-run evidence interpretation, not a supplementary
native execution or a per-test adapter claim.

Final report: `final-attempt-2/matrix-audit-identity-revised.json`, SHA256
`c2db20bb618b2e930c4eceb3b7288045ed44ffc25dae9099801a9b367190ca96`.
Audit-only revision SHA256:
`d9ff427d9b925767f54062155d9b05d785da06afd9e1a7a78826227067e18fa5`.
Run binding SHA256:
`11266c43fbaf021e19c72cf35591678fad8cd8c6f7f9ab11adc44655bd99cf63`.

Slice accepted for local commit; no push. Overall DX12 goal and command reuse,
window/recovery/diagnostic/CI/release/default-readiness gates remain open.
