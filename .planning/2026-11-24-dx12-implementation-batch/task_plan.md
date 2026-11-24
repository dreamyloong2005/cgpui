# Implementation-first completion batch

User direction: **加速开发，全部实现后再验证**. This supersedes the previous
slice-by-slice full-matrix cadence. Continue the active DX12 goal; no completion
or acceptance claim until the final combined-source verification.

Baseline HEAD: `6ee4998b16a7876d2a206bed75b42ca1352d08af`. User `.vscode/`
remains untouched. Local changes only, no push or remote workflow dispatch.

## Parallel ownership

- Root: public recovery-pending contract, Runtime latest resize/retry semantics,
  integration, documentation and final validation.
- Recovery agent: DX12 lease/ticket/cohort recovery and focused tests.
- Mailbox agent: private core managed-window work registry and Win32 lifecycle
  dispatch, with weak ownership and token-only queued messages.
- CI/window agent: opt-in WARP CI; composition-first ordinary-window route and
  dynamic transparency implementation after interface agreement.

## Implementation status (not validation)

- [x] Integrate reviewed owned DRED scalar diagnostics and native controls.
- [x] Add neutral `renderer_recovery_pending` contract; preserve distinct
  begin-only admission and completed-recovery retry results.
- [x] Add private latest-desired-resize owner; root/child defer unfinished
  recovery and apply the latest size/scale before frame acquisition.
- [x] Author Runtime coalescing/reentry/lifetime/error/structure regressions.
- [x] Add manual hosted WARP/common CI and fail-closed desktop prerequisites;
  no self-hosted scheduling without actual server-side authorization.
- [x] Implement managed-owner token mailbox and asynchronous DX12 retirement;
  native/CPU tests authored, independent source review in progress.
- [x] Implement safe composition-first dynamic ordinary-window transparency;
  retain native menu drawing, failure-only Popup fallback, no public window mode.
- [x] Extend fresh client-title-bar/no-HMENU windows through the same capability
  checks; add four native cases and a focused structure guard. No client+HMENU
  workaround, unsafe lease release or default change is introduced.
- [x] Update existing tests whose documented contract intentionally changes and
  author new native/pure/structure tests, without weakening old lifetime checks.
- [ ] Final source review and consolidated build/test/native/WSL/ASan/SDK matrix.

The already-running DRED no-run jobs4 compilation finished successfully before
the cadence switch was enacted: 11 executables built, **zero tests run**. Do not
treat that as acceptance of this newer, changing batch. After the implementation
owners and independent source reviewers finished, root began consolidated
compilation and verification. No native acceptance has been established yet.

Runtime source review found a concrete root callback-close continuation gap.
Root now rechecks close/shutdown/resources after view and render callbacks and
before acquisition; a focused close-during-render case is authored. This was a
source-review correction, not an observed executed RED/GREEN result.

## Consolidated validation progress (not final acceptance)

- Initial DX12 no-run build: exit 0, 224 executables built, zero executed. The
  subsequent client-title-bar additions require the current incremental build.
- CI Python fixtures: 38 pass. Independent review identified a single-run log
  marker-order gap; root ran two new regressions and observed both fail before
  the minimal checker fix, then the complete 38-case fixture suite passed.
- First whole-workspace no-run stopped on D: disk exhaustion (native linker
  `C000007F`/`LNK1000`); no tests were accepted from that run. The approved backup
  deletion attempts were rejected before launch, so no deletion was claimed.
- After disk space recovered, a fresh no-run jobs4 passed: 989 Windows test
  executables, including 228 DX12, 255 architecture and 47 header tests. The
  current source inventory contains 3350 files (excluding planning/user .vscode).
- First serial jobs1 whole-workspace execution finished: 987 pass, two failures,
  zero skips, with DX12 WARP/debug/GPU validation. It exposed a state-header
  structure cap violation and late recovery of a previously released native
  lease incorrectly enlisting a newly reconstructed device.
- The state header is now 98 lines (cap 100), with operations in focused owning
  leaves and the unchanged resource transition implementation in its own CPP.
  Both affected structure tests pass; no cap was relaxed.
- The recovery failure was reproduced in the original native probe and a CPU
  regression before repair. First actual release now freezes a lease identity
  cutoff; late old requests retain pre-release blockers but exclude replacements.
  Existing unresolved waves remain authoritative, and a genuine new live loss
  joins without extending the deadline. Focused CPU/native/owner recovery: 5/5
  pass. Temporary diagnostics are being removed before the combined rerun.
- One initial focused invocation omitted the suite selector: the intended five
  executables built, but unrelated members reported no matching tests. That
  command's exit 2 is retained and not counted as a successful workspace build;
  the corrected suite-specific no-run invocation exited 0.
- Final independent source Standards and Spec reviews reported no confirmed
  findings within their non-author scopes; actual execution remains mandatory.
- After removing temporary diagnostics, a fresh no-run built all 989 programs.
  The whole serial run finished 988/989: all 228 DX12 cases passed; the only
  remaining failure was an old capture-publication structure assertion still
  looking for its declaration in the state header rather than the new capture
  leaf. The assertion now checks the private leaf, thin compatibility include
  and absence from the state header, with a new leaf cap; no behavior oracle
  or existing cap was weakened. Full architecture rerun: 255/255 pass.
- This run is bound to the 3355-file snapshot at external
  `root-attempt-2/source-freeze` (freeze SHA
  `ae4ff13b2b4f84d798ed05e4a3ea70b9767b15b16522d3dcde315936e0898b42`).
  Exact postflight matched before the structure-test correction. It is a
  historical failed full run, not acceptance of future changed sources.
- CI's 38 Python fixtures pass again; the prerequisite PowerShell parses,
  desktop helper C# compiles and reports this local console's DWM/Default
  desktop facts. These are local checks, not hosted or authorized desktop CI.

## Final implementation candidates before the remaining consolidated matrix

- Safe bounded DRED breadcrumb observation has been integrated after independent
  source review and exact base-hash checks (33 candidate files; five new
  production leaves, seven DX12 tests and one structure test):
  confirmed removal/debug-only, ReadProcessMemory into owned bounded numeric
  data, no raw node/name/COM dereference, explicit partial/unreadable/cycle/
  inconsistent/truncated outcomes. It is non-atomic best-effort diagnostics,
  never authority for recovery or destruction. Address reuse still limits
  diagnostic coherence. The original plan requires debug breadcrumbs, not a
  page-fault walker or a universal device-wide coherent snapshot. Targeted
  no-run compilation is for integration feedback, not runtime acceptance.
- Automatic DX12-first initialization is implemented. Late
  fallback requires an explicit successful rollback of never-submitted,
  never-committed native presentation plus owner-thread alpha token release;
  opaque destructor cleanup is not proof. Invalid requests/configuration,
  runtime recovery and unproven rollback remain errors. Explicit backend
  requests never fall back. Focused combined runtime tests passed 23/23,
  architecture 257/257 and neutral policy/planning 16/16. The first architecture
  run found a redundant registration already covered by its owner glob and an
  old declaration-owner assertion; both were corrected without weakening caps.
- After those focused gates, the separately reviewed 31-file activation was
  applied atomically with exact base hashes: Windows default/auto now tries DX12
  first as a **local validation candidate**. Public/default tests, real poisoned
  loader controls, docs and the explicitly Vulkan-only SwiftShader CI job were
  migrated together. No release/default-readiness certification or publication
  is implied before the complete activated-source matrix passes.
- No current-snapshot HW/ASan/WSL/SDK matrix will be duplicated while these
  candidates are unfinished. After integration and independent review, derive
  the final source/test inventory and run the consolidated matrix once.

## Unchanged safety/claim boundaries

- The local Windows default candidate is DX12-first after focused readiness;
  final full-matrix readiness remains pending. Explicit DX12 never secretly
  changes backend or adapter; runtime failure never selects another backend.
- No native teardown of busy resources, no synchronous foreign-owner wait while
  holding the origin gate, no pointer payload in queued window messages.
- DRED diagnostics own their data. Guarded best-effort breadcrumb reads do not
  claim external-owner quiescence, coherent device-wide lifetime or full DRED.
  A page-fault walker is not required by the original plan; no internal mutex or
  renamed epoch token is used to manufacture an ownership guarantee.
- Native CI activation needs external runner authorization; local files are not
  evidence of deployment. Native macOS and arbitrary-driver certification cannot
  be fabricated locally.

## Consolidated acceptance progress (2026-11-25, local)

Implementation integration is complete within the documented capability envelope;
this is NOT overall goal completion or a release/default-readiness certification.
The current production Windows factory is DX12-first with checked initialization-
only fallback; explicit requests and runtime operations do not switch backends.

- Windows snapshot:3398 source paths, HEAD unchanged, freeze
  `4fb4f5ace2c86e3cc687d9055bf8cdc9d0373ae78b75a61a794e880392d3d321`.
  Fresh warmed full run:1004/0, exact39-member/test inventory, no skips;
  source pre/postflight exact. The failed attempt3(1003/1) remains retained:
  getting-started exceeded the existing180-line cap; fixed without cap changes.
- Intel73219, NVIDIA74481 and WARP74333: each240 native cases, plus107 common
  cases for each of Vulkan and DX12. Debug enabled; WARP additionally GPU
  validation. Three explicitly forced-unpinned rollback cases remain qualified.
- ASan WARP/debug:240 native and107 per common backend, no skips; exact source.
  Adapter proof is the existing visible/hidden first-begin predicate, not every test.
- Full39-member Release build and local installed SDK pass. Manifest-derived
  exact DX12 archive closure is127 source objects; explicit DX12 and healthy
  automatic DX12/WARP captures pass in clean and owned-invalid-loader runs.
  /MT direct consumer tested; CMake configure only, not a published SDK.
- CI fixtures:38 DX12 Python cases; driver PS5=15pass+2 PS7-only expected skips,
  PS7=17pass; actual system TEMP short paths pass on both; graph9/headers15;
  runner success/evidence-IO-failure/hard-timeout3 controls pass, no GPU claim.
- WSL first715 pass was not complete materialization proof: an independent GCC
  fixture exposed four Git120000 links exported as regular placeholder files.
  External-only typed export restores exactly those approved links; no repository
  source/test change or oracle weakening. New typed freeze
  `9f45cf9af4c8dd12ecbaaa1147ac372a66f0011e743ac0b2ac296bfd83111914`
  preserves the identical source byte manifest and test inventory, with explicit
  equivalence to the Windows freeze. Fresh715/0, full39 build, typed source checks,
  GCC2 and macOS architecture-rejection1 pass. The latter is not native macOS.
- Evidence: `D:/Dev/shots/cgpui-dx12-implementation-batch-20261124/root-attempt-4`.
  Original failed runs and immutable phase metadata remain; never sum overlapping
  replay counts into a fictitious unique-test total.

Still pending:18-series comparable performance measurement (first setup rejected
an obscured client point before any renderer creation/sample; user desktop help
requested), current-doc-only closeout, and external hosted/desktop CI authorization
and execution. No commit, push, merge, publication or workflow dispatch occurred.
DComp remains private native composition without D3D11 copy; unsupported ordinary
alpha combinations and retained alpha lease on opaque return stay documented.
Do not claim all-window parity, arbitrary-driver/TDR certification or total completion.
Old-backup removal remained blocked by execution policy; no bypass/deletion was
performed by this task. Later observed free-space increases are not cleanup evidence.

## Current handoff (2026-11-25 00:07 local)

All root build/test sessions are finished. No source edits may be applied before
remaining performance invocation unless its original-freeze plan is deliberately
revised. Original Windows source postflight still exactly matches3398 files.
Current local receipt:
`D:/Dev/shots/cgpui-dx12-implementation-batch-20261124/root-attempt-4/local-status-receipt.json`
SHA `64410d12d4bb857064e07959be7b4c6b330ae7198f25b72101e3b9af6ee64958`.

Three-file current docs candidate (unapplied, independent review0):
`D:/Dev/shots/cgpui-dx12-implementation-batch-20261124/candidates/docs-current-status-20261125`
manifest SHA `cf786d8baa768a7933bcbf6696a59ec44e99539621d118a194390c2bcaf42a6e`.
This supersedes old docs-closeout, which incorrectly grew the SDK script to174 lines.
New candidate keeps170 and changes only2 documentation string literals, plus current
status in dx12-backend.md/renderer-recovery-pending.md. All historical evidence stays.
Apply only after original-freeze performance gates, with exact base hashes; perform
focused documentation/packaging tests and explicitly label docs-only post-test delta.

Performance exact prerequisites were checked from current SDK build: /O2 /DNDEBUG
/MT, real probe link edge, SDK audit SHA
`65d3c3ff918caf238373a98a5fe0bdef0e234de8bc8870a059f564efe3c58293`,
Release executable SHA `89f7ef8896c7da28deb1c76910c3b61324c0389bc79388825c094501c2441a3d`.
Use unchanged perf-prepared/run-final-performance.py with a NEW evidence directory,
Intel LUID73219/selector0 and original Windows freeze; no build/GPU work concurrently.
First performance failure is retained, not retried automatically. User has been asked
to clear main-screen upper-left830x670 pixels. Current Edge observation was later
than failure and is not proof of its historical cause. Do not weaken visibility,
change foreground/topmost policy, or manufacture valid samples.

Remote commit/push/PR/merge/hosted dispatch request remains unanswered; no remote
write has occurred. Native desktop workflow must stay fail-closed without actual
server-side authorization. Overall goal stays ACTIVE, not achieved.

## Source freeze reopened: original-requirement gap closure (2026-11-25)

The original-plan audit identified omitted Vulkan device-loss reconstruction parity
and DX12 high_performance/minimum_power selectors. They are being implemented live;
the previous3398-file attempt4 freeze and receipts remain historical evidence only.
The preceding freeze/no-edits/performance-only instructions are superseded. New
implementation, independent review, registration and a fresh consolidated source
freeze/matrix are required before performance or current-document acceptance.
No new native verification has yet run for these additions.

## Original-requirement closure verification started (2026-11-25)

Source3452 is now frozen for attempt5 (Windows freeze6ad9380a385770ebde964d7aa865d7dad44dac0258db83aab3a3f2118b32fd42; typed WSL0ae912e066ac4c12dc63772d433c20b71b7028c44d8fe73713ea162f6088f480).
Required inventories: Windows1028/Linux725/common124/DX12243/architecture261.
The original-plan gaps (Vulkan reconstruction/CPU retention and both diagnostic
adapter selector families) are implemented, not yet fully accepted. Integration
no-run1028 passed. Focused original recovery run17/1 failed the new popup desktop
oracle; a confirmed missing PMv2 test precondition was repaired without changing
rendering or screen thresholds, then exact same popup entry passed all controls.
Adapter3 and architecture261 passed. Full frozen Windows run is in progress.
Use new reviewed tools at candidates/original-requirements-matrix, not old107
contracts. Old failed attempts remain. Source edits are now prohibited until a
genuine failing validation requires an explicitly new freeze.
A fresh native PowerShell authorized-old-backup cleanup attempt was rejected
before process launch; deleted zero. Do not bypass using another mechanism.

## Attempt6 test-ownership-only correction (2026-11-25)

Attempt5 fullWindows finished1025pass/3fail with exact frozen pre/postflight.
The three failures were old renderer tests looking for native create/destroy
inside vulkan_state.cpp after lifecycle ownership moved to vulkan_device_resources.cpp.
Updated those three assertions to require BOTH state delegation and concrete leaf
creation/destruction, keeping all previous native/resource checks; no production
change. Focused5+1 passed. Independent source review confirms each original test
had exactly this one stale condition and the new assertions are strictly stronger.
Attempt6 freezes exactly those3 test changes over attempt5:3452files, inventory
Windows1028/Linux725/common124/native243 unchanged. Windows pin
1b62517f4f76271ebd66639f580733d45fa76355c3cc7f3dee9ce5ec5df1c84d; typed WSL pin
5307f741254d3cd2e9dd1cb46b8c03589979572868643b4819f0119d017da9de.
Full Windows serial rerun session8473 is active; do not launch other build/GPU/WSL
work until it actually exits. The warm no-run earlier had a 'Finished dev' line
before all test linking; final process completion is authoritative, not that line.

## Attempt7 final portable predicate correction

Attempt6 fullWindows1028/0 passed with exact pre/post frozen source. A subsequent
independent standard check confirmed vector erase_if inherits remove_if's const/
non-mutating Predicate requirement. The new retention prune had updated age in
that predicate. Split its const deletion decision from the survivor age update;
added a focused structure assertion. This is two-file delta only, no interface
or inventory change. Actual full renderer124/0 and focusedstructure1/0 pass.
Attempt7 source3452 frozen Windows b881bf426934d6b624022e9389d2a27c736dde85f7f3d58ebf8c270a10c810e2;
typed WSL f8bc851897376540b519443ab5b2c2922daa7bebbec6115e9996c789c215b7d0.
Current root session40510 is running full warmed Windows4/no-run then tests1
and strict postflight. Do NOT launch another build/GPU/WSL run until actual exit.
No other source edits are planned or authorized by current audit findings.

Next serial stages: fresh actual adapter0/1/WARP probes, hardware0/hardware1/WARP
phases (native243+common124 both), Release/SDK and ASan, typed WSL725+39allbuild,
CI/driver fixtures, new performance18series if desktop condition is available.
Use reviewed candidates/original-requirements-matrix/final-prepared (bundle
f0e538a2add14346655ba0f504dc1eb1fc8b8fbebcb996a107c43e8a7fe68b16) and
typed-wsl-prepared (238bf4043984f5aa6fd6ca3b544e5dc1c7b941904ec14d65e9be37e33e881108).
Inventory SHA stays028421c54cb6bbb5c2702a34f62a0dd4f456ff4e5cb54464762e9f72ac61e562.
Typed mirror baseline is STILL attempt4 a30d4dca408a6920e37e95b2b71b6583c92d7dffa856ccd46efc271b10ced2db;
no WSL sync has occurred since attempt4. The only mirror is archlinux:/root/cgpui-logic-cleanup-20261123.
Old README exact fixtures correction stays authoritative: linux_gcc_toolchain_test.py2
and macos_sdk_arch_test.py1, not sdk_package_headers. Full script interface unchanged.

Disk: old target binary backups1068/3778700288 bytes were inventoried; approved
PowerShell deletion was rejected before process launch (zero removed), not an OS
sharing failure. Do not try alternate deletion tools/shells. User has been asked
asynchronously to manually free at least8GiB; no reply yet. Latest D about2.5GB,
C about3.7GB. Recheck before ASan/Release; do not start a knowingly disk-exhausting
build. Avoid expanding validation into unrelated refactors. Remote authorization
and original performance desktop-clear request still unanswered.

## Attempt7 current acceptance (2026-11-25 01:52 local)

Root sessions40510/90805/60978/71512/44022 all exited0. Current original
freeze b881bf426934d6b624022e9389d2a27c736dde85f7f3d58ebf8c270a10c810e2
still byte-exact3452 after all execution. Independent reports confirm Windows
1028/0/39; fresh actual adapter0/1/WARP LUID73219/74481/74333; each lane native243
plus common124x2; Release39/exact129 DX12 archive objects and two local /MT SDK
consumers; typed WSL725/0/39 plus GCC2/macOS architecture fixture1. Local Windows
CI38, PS7 driver17, PS5 driver15+2 specified PS7-only skips, SDKgraph9/headers15,
runner3 pass. Do not sum overlapping suite runs as distinct tests or claim every
preference test stayed on WARP; those tests explicitly override lane adapters.

Current receipt: root-attempt-7/local-status-receipt.json. New adapter file SHA
 af123a3203b2af2c29ff714126cee91f5696c03f57ede0710623a8658b1daa67.
WSL mirror now matches attempt7 typed freeze f8bc851897376540b519443ab5b2c2922daa7bebbec6115e9996c789c215b7d0,
not attempt4. No active root build/GPU/WSL job remains.

Remaining original-freeze gates: ASan (D free space below safe8GiB budget;
user manually freeing space requested), optimized18series performance (old
setup rejected occlusion before renderer creation/zero samples; clear primary
upper-left830x670 requested, no confirmation yet). No unchanged-obstruction
retry, topmost/foreground override, weakened guard, or deletion-policy bypass.
Old backup deletion remained rejected before launch; deleted zero. External
free-space increases do not establish this task performed cleanup.

Two-MD docs-only candidate prepared but NOT APPLIED in
root-attempt-7/docs-closeout-candidate, manifest SHA
c2341cd3a869edbc83032e6b9fad02b498d081d667c45167e9545a7716501219.
SDK Python remains unchanged170 lines. Apply final current-status docs only after
remaining original-freeze runs, then focused doc checks; no need to reopen
implementation or rerun all behavior for pure prose. Remote commit/push/PR/merge,
workflow dispatch, release/publication still not authorized or performed.
Goal remains ACTIVE, not complete. No arbitrary-driver/TDR, oldWindows/native
macOS, hostedCI or performance-parity certification is claimed.

## Local closeout and remaining original Mac gate (2026-11-25)

Space recovered externally; root did not delete backups. ASan session52725
finished0: exact243 DX12+124 common per backend, zero failures/skips. Independent
ASan review991db945eb0381ac11c679c84aa75640cdb7f29f4be367539820c5cac7cefb38.
User explicitly confirmed clear desktop. Performance session68104 then finished0:
18 planned series,120reported warmup and600steady samples each, all raw audits0.
Independent10800-row review9e95ec93a5bc229e55dfb1d0e98d4b5225251f788282d55836d5acb19baba948.
Same Intel LUID73219/Release probe59a39622ce98b8c418923ba5384222837839d70ea7ddbeb40e80f6a5c8884f03.
No GPU/FPS/parity claim: local DX12 CPU frame-envelope p95 is higher than Vulkan.

Final two-MD docs candidate applied; no code/test/build-script changes. Exact
remaining3450 files and discovery match attempt7. docs-only-delta receipt828f48b67338f5bf744e014faf6d1ae299f9fbc740b501396801381430206b8a.
Focused existing documentation smoke2/0; they do not directly read the twoMD,
so actual facts/links and exact delta were separately checked. SDK Python unchanged.
Current consolidated evidence: root-attempt-7/local-closeout-receipt.json.

Completion audit found original phase1 explicitly requires shared code tested on
native macOS. docs/macos-validation.md records only2026-11-21 history, earlier
than this DX12 shared extraction/toolchain. Therefore full goal NOT yet complete.
Asked user for an authorized Mac workspace/connection; no remote access/publication
was inferred. Needed current-equivalent-source Apple Silicon validation:
  export MACOSX_DEPLOYMENT_TARGET=26.0
  bash scripts/dev/build.sh -p packages/renderer
  bash scripts/dev/test.sh -p tests/renderer-neutral
  bash scripts/dev/test.sh -p tests/header_cleanliness
  bash scripts/dev/test.sh -p tests/architecture renderer_planning_ownership_structure_test
Expect16+47+1 exact tests; preserve nativeOS/Xcode/mcpp/source binding and full logs.
This is a targeted shared-module gate, not arbitrary all-platform certification.
No active root builds/GPU/WSL runs remain. Goal ACTIVE pending Mac environment,
with no new confirmed source defect; do not rerun passedWindows/WSL/ASan/perf
merely because this environment is unavailable. No commit/push/PR/merge/dispatch
or publication occurred. Do not claim overall completed until this original gate
is proven or the human explicitly changes requirements.

## Completed under user-confirmed scope (2026-11-25)

The user explicitly instructed: 记录为macOS的待跟进项即可.
Native macOS shared-renderer validation is now an accepted deferred follow-up,
not a current DX12 completion blocker and never represented as a pass.
Recorded in docs/macos-validation.md with native environment,16+47+1 tests,
source identity and evidence requirements; linked from docs/dx12-backend.md.
Final current source differs from the accepted attempt7 implementation by exactly
three documentation files only; all3449 other frozen files/test discovery unchanged.
Final existing doc smoke2/0 and git diff --check passed. Independent scope/doc
review confirms no remaining required work for the Windows DX12 milestone.

Final receipt: D:/Dev/shots/cgpui-dx12-implementation-batch-20261124/root-attempt-7/milestone-completion-receipt.json.
This supersedes earlier active/pending/blocker entries without overwriting history.
Windows1028, Linux725, hardware/WARP243+124x2 each, ASan243+124x2, Release39,
localSDK129DX12objects/two consumers, CI fixtures and18performance series accepted.
Performance remains descriptive CPU wall-clock evidence with higher localDX12p95,
not a GPU/FPS/parity threshold claim. Supported-window limitations remain explicit.
No commit/push/PR/merge/remoteCI dispatch/release/publication or cleanup-policy
bypass occurred. Mac follow-up remains intentionally open; current DX12 milestone
has no remaining required implementation or local-acceptance work.
