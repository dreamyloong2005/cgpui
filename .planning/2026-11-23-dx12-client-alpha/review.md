# Independent review: bounded initial client-title-bar alpha

Review baseline: `4ce08c0a`. Review date: 2026-11-23.
Scope: the current tracked production delta, new native-menu policy leaf,
registered behavior/structure tests, and the three updated public documents.
This review uses the final bounded scope below, not the earlier menu-layout
candidate as a promise of supported behavior.

## Specification used for review

- Initial DX12 alpha on CGPUI-owned ordinary windows with a visible, decorated
  client title bar is supported only without an attached application HMENU bar,
  on Windows build 26100+ and after actual DWM alpha initialization succeeds.
- Preserve `WS_CAPTION`, `WS_SYSMENU`, existing minimize/maximize styles and
  native controls. Do not adopt the rejected captionless-style workaround.
- An active alpha lease retains its initial title-bar mode. System/client
  changes, hidden/undecorated chrome and fullscreen are refused without mutation.
- Refuse initial client-alpha creation with an existing HMENU without hiding,
  removing or replacing that menu. Reject application-wide nonempty menu
  installation before any HWND, model or accelerator mutation while a registered
  client-alpha lease is active. Empty clearing remains allowed.
- During install and clear transactions, refuse nested installation and new
  client-alpha acquisition even when a target currently has no HMENU.
- Detect out-of-band raw `SetMenu` in alpha status/repair rather than claiming
  compatibility or repairing it into supported client-menu behavior.
- Keep system-title-bar alpha plus native HMENU support, ordinary opaque HWND
  flip behavior/history, backend defaults, and Popup policy unchanged.
- The pre-existing opaque client-title-bar/HMENU layout defect remains unfixed.
  No framework-menu replacement is forced or recommended as this implementation.

## Standards axis

Reviewer: `client_menu_alpha` (independent of the production owner).

Result: **0 hard standards violations; 0 actionable Fowler-style smells**.

The policy is a focused private RAII leaf; the application entry only delegates.
Existing module caps and native styles/menu drawing are preserved. Public tests
do not bypass production admission using manual DWM/alpha repair. Structure
coverage checks the leaf boundary, preflight/transaction ordering and behavior
test registration. The final structure registration check includes
`dx12_client_controls_test.cpp` as well as the other five client-alpha tests.

## Spec/correctness axis

Reviewer: `repo_progress` (independent of the production owner).

Result: **0 actionable findings** in the reviewed delta and new files.

Key paths inspected:

- `Win32NativeMenuInstallationGuard` performs whole-registry admission before
  the application detaches any menu or replaces its native state. Preflight has
  no application/native callback; the thread reservation is acquired after its
  allocating snapshot, avoiding a latched gate if construction throws.
- Guard refusal returns before updating the application installation result,
  menu model, command map, accelerator table or any window menu. The guard's
  owner-only destructor releases the reservation; nested guards cannot release
  an outer transaction's reservation.
- Client-alpha support checks both an existing HMENU and the active thread
  transaction. The status/repair path detects raw menu attachment without
  hiding it. System-title-bar menu admission and repair remain supported.
- Fixed-mode refusal precedes chrome mutation; the existing fullscreen guard
  applies to the expanded client route. The display-module delta is explanatory
  comment text only, not a behavior change.
- Tests exercise public factory refusal, unchanged geometry/styles/commands and
  the application model, actual reentrant resize callbacks during install/clear,
  release/reacquire, and the system-title-bar positive control.

## Documentation cross-check

Reviewer: `repo_progress/bounded_docs_check` (read-only).

Result: **0 actionable documentation findings**. The updates to
`docs/dx12-backend.md`, `docs/client-title-bar.md` and `docs/window-backdrop.md`
match the no-HMENU client scope, native-style preservation, fixed-mode and
transaction restrictions. Existing system-title-bar HMENU/Mica evidence is
explicitly qualified. The opaque client/HMENU defect and validation limits are
retained. The later controls evidence was checked against its source and
`client-controls-green.log` before being added.

## Validation evidence and limits

Reviewers did not build or run tests. Root owns serial validation. Targeted
native logs under `D:/Dev/shots/cgpui-dx12-client-alpha-20261123/` record passing
initial-alpha, lifetime/recovery, menu-refusal/reentrancy and controls checks
with WARP and DX12 debug validation. In particular, the reentrancy log reports
real install and clear callbacks with a null target HMENU and refused alpha
factories; the controls log records real native state changes and close-policy
cancellation. Source inspection, rather than a new execution by these reviewers,
supports this review. `git diff --check` passed during the documentation freeze.

At the time this record was written, root's full validation run (session 41532)
had started but had **no final result**. This record does not mark that run,
full Windows/WSL regression, ASan validation, or any unreported suite as passed.

Root's later frozen-source validation has now completed successfully. Its actual
counts and scope are recorded separately in `acceptance.md`; this is not a
claim that the independent reviewers executed those runs.
The final acceptance/log cross-check by `repo_progress` also found zero
actionable discrepancies or overstated claims before the local commit.

The accepted evidence does not establish client-title-bar HMENU support,
physical dragging or Snap Layouts interaction, physical monitor-DPI migration,
all-adapter/driver behavior, hardware TDR recovery or execution on an older
Windows build. Scale 1/2 checks are explicit renderer scales. Maximized controls
check GPU alpha; restored windows re-establish the desktop-pixel oracle. Mica
tests allow explicit OS/policy refusal with a reason and require `none`
restoration; they do not promise material application on every system.
