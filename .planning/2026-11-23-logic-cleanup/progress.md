# Progress

2026-11-23: User authorized fixes and migration-directory removal. Rechecked baseline and worktree: HEAD 91e68993, only .vscode untracked. Read audit and reused confirmed RED evidence. Delegated disjoint runtime, platform/rendering and documentation/architecture work. Root owns test registration and all serial test execution.


Integration checkpoint:
- Registered regressions for Win32 registry, runtime retry/window record stability/self-close/owned renderer/failure cleanup/window redraw, terminal callback lifetimes, Metal readback, and new structure boundaries.
- Confirmed baseline RED: registry crash (runner exit133), runtime retry exit13, terminal captures exit3. The old terminal GREEN build was deliberately interrupted because window-state headers were changing; no green result claimed.
- Native host probe compiled and ran: DCompositionCreateDevice(nullptr) and Device2(nullptr) both created target/visual/root and committed successfully (all HRESULT 0). Official SDK marks render device optional. Rebuilt host-boundary architecture guard failed exit5 before repair.
- Actual Vulkan upload lifecycle probe compiled with real staging/recording/resources and failed exit1: "one-shot image upload was discarded before submission". Repair now preserves pending staging across acquisition retry; awaits GREEN.
- Removed verified docs/migration and scripts/migration; archived dated native Mac evidence under docs/macos-validation.md.
- Text ownership moved to core font/value leaves, platform font database, renderer text sources/public leaves; former UI public headers remain thin compatibility includes. Package source registration now matches physical ownership.
- Terminal callbacks move out on completion/cancel/group cancel; destruction occurs outside mutexes. Regression includes reentrant capture destructors, queued cancellation and I/O shutdown. Terminal metadata remains queryable by existing handles.


Independent review and extra regression checkpoint:
- First architecture run: 180 passed / 12 failed, log architecture-green.log. Failures were old path/ownership tokens and original module budgets; owners corrected modules/guards without increasing budgets.
- Renderer-neutral planning baseline: new renderer-only member failed linking with 20 unresolved legacy CPU planner symbols. After moving CPU planning to neutral owner and preserving compatibility forwarding, all 3 neutral tests passed (planning-neutral-green.log).
- Independent review found destructor-reentrant retirement could mix renderer/native or tree/view cohorts. New actual regression failed exit3 (retirement-red.log); fixed by atomically detaching each resource cohort and draining new cohorts before record reclamation.
- Explicit runtime shutdown must destroy user callback/view resources while task metadata is still alive. New test failed exit1 (shutdown-capture-red.log); cleanup order fixed. TaskGroup now uses an opaque weak lifetime token so RAII destruction after a runtime is safe. Ordinary TaskHandle outliving-runtime contract is unchanged.
- Reviewer found I/O capture destructors could register escaping hooks after registry shutdown. Actual test failed exit1 (io-shutdown-red.log); registry now closes under its lock and rejects registration.
- Cross-thread entity shutdown destroyed captures under its mutex. New focused test failed exit137 in 0.18s, not a timeout (cross-thread-shutdown-red.log); queue now swaps operations into a local retired list released after unlock.
- Include-only consumer compiled from a scratch copy containing ONLY public include files failed C1083 on ../../../src/ui/window_runtime_internal.hpp (installed-headers-red.log). True PIMPL/state isolation is the next separately validated slice, not just a header path relocation.
- Current UI/async/architecture checkpoint no-run build is in progress. No full-suite GREEN, WSL GREEN, or native macOS result is claimed yet.


First combined behavior checkpoint:
- no-run build: 311 programs built successfully (194 architecture, 105 UI, 12 async).
- Execution: 306 passed / 5 failed. All 12 async tests are GREEN, including terminal capture release, explicit shutdown, TaskGroup outlive, I/O registration reentry and cross-thread shutdown. Reentrant window growth/close/retirement, owned renderer cleanup, child redraw and async-origin tests are GREEN.
- Remaining failures: three structure checks (pending unregistered PIMPL probe files counted by matrix, stale retirement delegation token, redundant include exceeding original presentation budget); two UI checks (default/context id0 root compatibility; retry fixture did not pump requested wakeup). Owners are resolving without weakening behavior/size assertions.
- Pending PIMPL probe sources temporarily live under this plan's probes/ so the first-stage architecture matrix remains a truthful inventory; they will move to tests and be registered when that slice starts.

Second Windows checkpoint (2026-11-23):
- Full no-run Windows build succeeded: 626 test programs, 35 members (windows-stage1-build.log).
- Full serial execution: 623 passed / 3 failed; architecture194, headers32, UI105, platform67, async12 all passed. Native Win32 runtime retry and Intel/NVIDIA Vulkan Popup pixel tests passed.
- Three renderer failures are source-ownership guards still requiring former vulkan_ CPU planner names. Behavioral assertions before those guards passed. Minimal ownership updates and prose-only cleanup are under focused renderer rerun.
- Registered runtime_public_state_isolation_test and runtime_installed_include_consumer in their suite manifests and architecture/header CI inventory.
- sizeof RED actually executed: runtime-isolation-red.log, exit1: WindowRuntime must contain only its owning opaque-state pointer. External include-only C1083 RED from previous checkpoint is retained.
- Authorized true opaque-state migration now that runtime behavior is GREEN; SDK closure validation proceeds independently. No full post-migration GREEN or WSL result claimed yet.

Bridge isolation checkpoint:
- Four focused non-template storage/observer/access modules replace direct runtime-field use in public templates. Independent semantic review found no regression in entity counts, global invalidation, observer validation/IDs or cross-thread owner sharing.
- Windows no-run 250 targets and serial run 250/250 passed (runtime-bridge-{build,test}.log): UI105, API parity100, async12, header33.
- Renderer historical-prose cleanup extended to all 54 affected renderer tests. 182 behavior-function bodies are unchanged against HEAD; current ownership/registration checks are retained. Full renderer rerun after the remaining51 prose removals is still pending.
- SDK installed-header preflight moved to a focused Python module, removing source-header copying and rejecting stale src/ output. 15 Python tests pass; wired to Windows/Linux/macOS CI. SDK schema4 and native-Mac schema2 reflect the removed workaround fields. No existing package or release has been overwritten or published.
- Single-pointer runtime migration is now in progress after the bridge GREEN.

Opaque-state checkpoint:
- Real MSVC copied-include-only consumer compile now passes (installed-headers-green.log); the isolated directory contains no src/. SDK Python preflight also accepts the copied live include closure (sdk-header-closure-live-green.log).
- Full UI dependency graph compiled after 122 fields moved into 13 focused private state groups. Existing public observer value types remain source-compatible; public templates use erased bridges and never reach owning storage.
- Independent Spec review: field declarations/defaults retained; 1,077 explicit state accesses match the ownership map; public API unchanged ignoring format/comments. No new independent issue beyond shutdown sequencing under repair.
- Extended shutdown regression actually FAILED exit1 in 0.03s (runtime-pimpl-shutdown-red.log), exposing user captures in globals/entities/observers/actions/animation/frame hooks released after task metadata detaches. Repair is required before acceptance, not papered over by the passing include-only compile.
- Fresh WSL validation mirror /root/cgpui-logic-cleanup-20261123, not the user's dirty /root/cgpui-linux. Renderer-neutral 3/3 passed with GCC16.2.1 (wsl-neutral-test.log). Initial wrapper attempts failed before compilation due shell quoting/CRLF, then normalized wrapper execution succeeded. Full WSL validation still pending.

Final validation in progress:
- Expanded shutdown test now GREEN along with all 12 async tests. PIMPL checkpoint 248/250: only remaining failures were source-token checks for the removed private header and renamed scale ownership, both repaired without changing behavioral assertions.
- All product and test sources are frozen for final full Windows build/run, WSL run, installed consumer link and scoped sanitizer validation.
- Independent Standards review found no additional actionable issues; Spec review likewise clean except the shutdown issue now repaired. Original effective same-variable line caps compared against HEAD: no increases.
- Migration-only scripts and large JSON inventories moved out of the product repo into evidence/planning-history; only concise planning/history and final acceptance remain in .planning.

Cross-platform follow-up:
- First fresh-mirror WSL run built600 tests and ran592pass/8fail. Seven source-text checks were traced to copied Windows CRLF inputs, not runtime behavior: normalizing their inputs to Linux checkout LF made the unchanged seven executables all pass (wsl-newline-diagnosis.log).
- x11_input_test independently repeated exit5 (no initial redraw). Diagnosis: checked XCB requests can cache Expose in the userspace event queue; the loop blocked in poll before draining cached events. A focused deterministic queued-event regression and repair are being added; extending timeouts would hide the bug.
- All WSL UI105/async12/renderer68/neutral3 tests passed in the first run; no native Mac behavior claim.


Cross-platform GREEN:
- WSL final full no-run601 built, full serial601/601passed (wsl-final2-{no-run,test}.log). X11 original input now passes; new cached-before-run and cached-in-wakeup cases pass, including20repeat and X11 suite7/7.
- One first post-fix test launch had an unconfirmed immediate application-initialization failure (exit1, not queue starvation). Added useful error diagnostics; isolated/repeated20 and full final run passed without a retry workaround or weakened assertion. No speculative fixture/product change for that one launch.
- WSL external public include-only consumer linked against copied static libraries and ran successfully (installed-sdk-linux.log). SDK Python15 and unsupported Mac architecture guard1 passed in Linux; these do not run Metal.
- Windows ASan scoped build is running; mcpp invalidated the prior ASan output on toolchain identity, so it is rebuilding rather than claiming reused validation.

Final closeout:
- Windows AddressSanitizer completed: 41 window-runtime tests, 12 async tests and the real Win32 registry regression all passed (54/54). Logs are asan-ui-test.log, asan-async-test.log and asan-registry-test.log in the external evidence directory.
- Rechecked the complete final results from retained logs: Windows 628/628, WSL 601/601. Product/test sources are unchanged against the final WSL source snapshot. git diff --check is clean.
- Independent follow-up confirmed both cached-XCB-event scenarios have actual RED/GREEN evidence. No speculative change was made for the one unreproduced initialization failure; private Xvfb is not a claim about every real window manager.
- Kept only planning/acceptance Markdown in this plan directory; temporary source previews, probes and Python cache were moved to the existing external planning-history evidence directory. No user .vscode content, remote branch, existing Linux checkout or published package was changed.
