# Independent review

Baseline: `2a3e9bc6`; specification: `task_plan.md` and the bounded loader-only
scope. Source/docs were frozen before the full validation snapshot.

## Standards

Reviewer: `client_menu_alpha`. **0 hard violations, 0 actionable Fowler smells.**

The private lifecycle owner, typed binding leaf and single required-symbol
inventory keep the instance entry thin and native details out of public headers.
The existing SDK generator remains responsible for consumer dependencies; no
delay-load flag/hook contract or new xmake file is introduced. The existing
legacy SDK xmake consumer only conditionally retains old Vulkan linkage.

Checked candidate validation before publication, RAII failure cleanup,
non-throwing pointer publication under a mutex, successful process-lifetime
module retention, and fixture path/marker checks before owned-file deletion.
The repeated symbol operations derive from one inventory. Independent first-main
and child-process pixel oracles do not need a speculative general abstraction.

## Spec

Reviewer: `repo_progress`. **0 actionable findings.**

The 88 required Windows symbols close over the existing native call sites.
Instance creation admits the loader before the first Vulkan call. DX12 and
invalid backend selection do not admit it. Failed loads/bindings publish no
pointer, release candidate modules and permit another attempt; successful
admission publishes once and subsequent admissions only read the stable table.
Existing optional version/device-extension queries and Linux direct linkage
remain intact. SDK CMake/README/legacy consumer dependencies follow the actual
system-library graph, without a new consumer link option.

## Evidence boundaries

Reviewers did not run builds/tests. Root's targeted tests passed before source
freeze: initial DX12 independence, owned invalid-PE loader isolation, same-process
failure/retry, 88 missing-export binding cases, 16-thread first admission, and
207 architecture tests. Final full-run results belong in `acceptance.md` only
after the driver completes.

The invalid-loader public tests do not remove the installed system DLL. Individual
missing exports are tested through the private resolver seam, not 88 separately
built DLLs or public factories. Windows still needs Vulkan SDK headers to build
the repository; eliminating that build dependency is not this milestone.

Root subsequently completed frozen-source Windows/WSL/native/ASan/SDK validation;
the exact counts and limits are recorded in acceptance.md. The reviewers did not
substitute their static inspection for those executed tests.
