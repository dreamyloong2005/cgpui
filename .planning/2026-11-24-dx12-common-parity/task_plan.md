# DX12 common parity and real runtime retry

Date: 2026-11-24. Baseline: `36b3cd40` (`perf(dx12): reuse bounded texture upload pages`).
Status: IN PROGRESS. This is the next bounded slice, not full DX12 completion.

## Scope: three focused modules

1. **Common real-pixel harness and scenes.** Migrate the same 13 Windows pixel
   scenarios to a selected named-backend entry with strict actual backend,
   selection/fallback and WARP-software checks. Preserve pixel oracles/tolerances,
   public-neutral planning ownership and narrow test-support cpp modules.
   Vulkan 0/5/9 and DX12 0/3/6 backdrop topology remain separate exact native
   regressions; their counts are not equated. Ordinary-window compatibility,
   full device recovery and production backend default changes are out of scope.
2. **PowerShell test-runner selection.** Explicit `-Renderer both` runs both
   requested backends without changing no-selector/auto defaults, argument/ASan
   handling or failure propagation. Parser/process regressions must cover real
   native nonzero exits and restoration; neither backend may silently skip.
3. **Real DX12 WindowRuntime retry acceptance.** Integrate the independently
   prepared native runtime retry tests and focused ownership guard. Observe
   actual device removal and later runtime-rendered pixels, not just a mocked
   retry or manual renderer loop. Keep runtime source/API changes outside this
   test slice unless independently diagnosed and explicitly scoped.

## Ownership and sequencing

- Root owns candidate application, builds/test execution, final integration,
  evidence recording and any commit. No parallel build/test processes.
- Common candidate A centralizes the original actual Vulkan factory while
  imposing the real strict identity gate. Root must first establish a meaningful
  DX12-request/Vulkan-actual RED through that same scene entry, not a test stub.
- Candidate B changes only the same creator's factory expression. Apply only
  after RED evidence; then strengthen the common ownership guard to require both
  named factories and validate both requested-backend runs.
- The common guard owner migrates existing structure references, including the
  native glyph-shape helper's header/cpp split, without raising original caps or
  weakening assertions. It registers the common guard first in architecture
  mcpp/CI, then hands those shared registries to the runtime guard owner.
- Preserve `.vscode/` and all unrelated work. No remote push/publication.

## Acceptance checklist

- [done] Apply common candidate A and migrate/refine ownership guards.
- [done] Root native baseline: common identity/pixel test under Vulkan.
- [done] Root real RED: requested DX12, actual Vulkan rejected by common gate.
- [done] Apply B and require both factories in the focused common guard.
- [done] Post-B renderer suite passed 92/92 under Vulkan and 92/92 under
  DX12 WARP + debug, including the same 13 common scenes and actual identity gate.
- [done] Exact native backdrop diagnostic wrappers: Vulkan 0/5/9, DX12 0/3/6,
  included in the two successful renderer-suite runs.
- [done] Explicit-both/parser/native-exit/ASan-forwarding runner regressions.
- [done] Actual native WindowRuntime retry/recovery acceptance: root-first and
  child-first cases passed 2/2 under WARP + DX12 debug. Final matrix coverage of
  its ownership guard remains part of the regression gate below.
- [pending] Root structure/header/full Windows regression and appropriate
  WARP/debug/GPU-validation/ASan/WSL/SDK validation; record exact executed scope.
- [pending] Independent Standards/Spec review and final source/evidence consistency.

## Evidence limits

Root reported the following actual Stage A execution on 2026-11-24:

- Identity Vulkan control passed and reported the actual Intel Vulkan backend.
- The same identity test with a DX12 request rejected actual Vulkan, exit 2.
- Original clip-scene Vulkan control passed; its DX12 request hit the same real
  common-creator mismatch and failed, exit 1.
- Root evidence logs are named `identity-stage-a-*` and `clip-stage-a-*`.
- Stage A architecture suite passed 212/212.

Root then applied B's single factory-expression hunk. The common guard now
requires the explicit DX12/Vulkan named-factory branches in that same owner.
Root then reported post-B renderer acceptance: Vulkan **92/92 PASS** in
`renderer-vulkan-corrected.log`, and DX12 WARP + debug **92/92 PASS** in
`renderer-dx12-corrected.log`. Both include the 13 common scenes and the focused
native backdrop diagnostic wrappers. This is bounded suite evidence, not the
final full matrix or proof that Vulkan-private tests became DX12 tests.
The remaining unfinished checklist items stay pending.

Additional root-reported integration evidence:

- `renderer_frame_pixels_test` exposed a stale file pointer (RED exit 50), then
  passed after root updated the current capture-scene path.
- `phase_e_pixel_output_integration_closeout_test` exposed old target references
  (RED exit 11). Root migrated eleven references and retained capture coverage
  through the resize `begin_pixel_capture` path and common request oracle; GREEN.
- The two native runtime cases built with `--no-run`, then root-first and
  child-first real recovery both passed under WARP + DX12 debug (2/2). Evidence:
  `runtime-first-build.log` / `runtime-first-test.log` in root's current evidence
  directory. This added acceptance without production code changes.
- The current architecture run passed 211/212; `phase_j_documentation_structure_test`
  failed with exit 3 because the getting-started guide exceeded its 180-line cap.
  The renderer instructions were condensed back to 180 lines, without weakening
  the guard. The failed log `architecture-current-test.log` is retained; root's
  rerun is pending.
- Root applied the driver candidate. Its first integrated fixture attempt failed
  during setup because root's custom temporary parent directory did not exist,
  not because a driver assertion failed. `driver-integrated-ps7.log` is retained;
  root is rerunning with the parent directory created. No green integrated fixture
  count has yet been supplied to this record.

## Documentation and reference audit

- User-facing renderer testing instructions now describe the applied B helper's
  actual selector, strict identity gate and exact common scene set. They do not
  claim unreported results. After root applied the driver, executable examples
  include explicit `-Renderer both`, with separate-backend/focused examples and
  precise exit/default/environment semantics in the DX12 guide.
- The existing Windows Python-fixture CI step now invokes unittest discovery
  for `windows_test_driver*test.py` using both resolved `pwsh` and `powershell.exe`
  hosts via the fixture's `CGPUI_DRIVER_TEST_POWERSHELL` selector. Every Python
  nonzero exit fails the step. It uses the default temporary directory, not a
  nonexistent custom parent; Linux fixture registration is unchanged. This
  registration is not evidence that GitHub Actions has executed it successfully.
- Current effect/text documentation, JSON source pointers and example pixel
  target names move to the current neutral scene names. Historical phase counts,
  platform statuses and prior measurements are not upgraded to DX12 evidence.
- Whole-repository source-reference scan identified stale paths in
  `tests/renderer/renderer_frame_pixels_test.cpp` and
  `tests/api_parity/phase_e_pixel_output_integration_closeout_test.cpp`; root fixed
  these and reported the RED/GREEN results above. The documentation task did not
  edit those tests. A follow-up source/scripts scan found no remaining old pixel
  names outside intentionally historical records.
- Historical `docs/superpowers/plans`, prior `.planning` recommendations,
  `findings.md` and `progress.md` retain their original file-name context rather
  than pretending the renamed harness existed during those earlier runs.

Candidate source inspection and `git apply --check` alone do not establish C++
compilation, runtime recovery or pixel parity. This slice does not switch the
production default, solve all ordinary alpha/chrome/menu combinations, add WARP
CI by implication, or certify physical-monitor/TDR/native-macOS behavior.

## Root integration update before final matrix

- Applied final driver candidate; integrated PS7.6.9: 16 PASS, Windows PowerShell 5.1.26100.9818: 14 PASS + 2 explicit PS7-only SKIP. Logs: driver-integrated-ps7-green.log and driver-integrated-ps5-green.log. Setup failure log remains unchanged.
- Real explicit-both runner completed two native runs, Vulkan 92/92 and DX12 WARP+debug 92/92: both-integrated-green.log.
- Condensed documentation passes original phase_j_documentation_structure_test: documentation-cap-green.log. No cap was raised.
- Independent Standards and Spec reviews both report 0 actionable findings, in standards-review.md and spec-review.md; they did not execute tests.
- Frozen 2858 source paths (excluding .planning and user .vscode); final sequential Windows/WSL/adapters/WARP/ASan/SDK matrix is now running. Final results are not yet accepted.

## First full matrix and fixture normalization correction

- First frozen-source native matrix passed Windows 777, WSL 647, Intel/NVIDIA/WARP
  DX12 97 each; common renderer Intel/NVIDIA DX12 92 each and WARP explicit-both
  92 + 92; ASan DX12 97 and renderer explicit-both 92 + 92. The script then
  stopped at driver-final-ps7.log: four fixture assertions compared long and
  short (8.3) spellings of the same TEMP directory. SDK/final source check did
  not run in that first pipeline. Do not call it an entirely successful matrix.
- Tight driver-short-path-red.log reproduces the exact both/cwd failure.
  os.path.samefile proves the logged short/long directories identify the same
  path; this is a fixture expected-path defect, not a wrong working directory.
  Resolving the mkdtemp result before constructing paths preserves every exact
  command/environment assertion; driver-short-path-green.log passes with the
  same default TEMP. No production driver or rendering change was made.
- A dedicated real Windows short-alias fixture regression is being added; hosts
  without an actual 8.3 alias will report a skip rather than fabricate one.
  Source freeze is withdrawn for this test-only correction. A second full frozen
  matrix will write under evidence/verified/ so all first-pass logs remain intact.
- Dedicated real-short-alias test first failed both normal and ASan subcases on
  the pre-fix fixture (driver-short-alias-regression-red.log). With canonical
  fixture paths, the complete default-TEMP suites pass: PS7 17/17; PS5 15 PASS +
  2 PS7-only SKIP. The new alias regression actually ran on both local hosts;
  it was not the skipped case. Logs: driver-short-alias-ps7-green.log and ps5.
- Independent Standards follow-up accepted this two-file fixture correction with
  0 findings. Frozen source v2 now has 2859 paths; full rerun started in verified/.

## Final closeout

Accepted after the second complete frozen-source run: see acceptance.md for all
counts, first-run failures, final `verified/` logs and limitations. Final process
exit0; both source manifests match all2859 paths. This bounded slice is complete;
the overarching DX12 completion goal remains active.
