# Logic cleanup following the 2026-11-23 audit

Baseline: `91e68993`, branch `phase-v`. The only pre-existing untracked work is `.vscode/`; preserve it. User authorized fixing the audit findings and deleting the migration directory. No push/merge or DX12 backend implementation.

## Contract and test seams
- Real Win32 window destruction unregisters every dispatch/menu observer safely.
- User callbacks can create/close windows without invalidating live dispatch/render state. Use stable ownership or ID relookup, not reserve-only workarounds.
- Window and Context invalidation target their window, not always the root.
- Window/renderer/view ownership retires together at a safe point; initialization failures roll back.
- Recoverable renderer results schedule a retry, while fatal errors preserve failure behavior.
- Terminal tasks and I/O hooks release captured resources outside locks, while terminal handle queries retain their contract.
- Correct Metal readback resource usage; native macOS validation must be reported as unavailable if not run.
- Remove verified migration directory and its references; eliminate historical-prose structure locks without weakening functional or module constraints.
- Address public/private and text ownership debts as independently testable module changes, avoiding a speculative framework rewrite.

## Phases
1. [complete] Translate known RED probes into registered regression tests; establish focused ownership and implementation plan.
2. [complete] Fix runtime/window, Win32 registry, retry policy and callback lifetimes; preserve default Vulkan Popup and genuine-failure overlay fallback.
3. [complete] Remove migration material/references, improve structure checks and module ownership.
4. [complete] Targeted Windows tests, serial full Windows 628/628, WSL 601/601, scoped Windows AddressSanitizer 54/54; no simultaneous build processes.
5. [complete] Standards/Spec review and exact acceptance evidence; local commit only, no push.

## Coordination
- repo_progress: runtime window records/reentrancy, scoped redraw, renderer ownership and startup rollback.
- gpui_windows_design: Win32 registry, rendering retry and Metal readback; focused platform/composition seam.
- popup_structure: migration and documentation/structure-test cleanup; architecture ownership refactor proposal.
- root: terminal task/I/O captures; test manifests/CI matrix; integration, builds and review.

## Evidence
Audit/repro: `D:/Dev/shots/cgpui-logic-audit-20261123/`. Baseline repro results include Win32 access violation, record growth corruption, incorrect root redraw, 11 renderers after 10 child closes, four terminal callbacks retained and retry result causing app failure.

## Constraints
mcpp >=2026.11.21.1 via scripts/dev; Windows MCPP_JOBS=1 for execution to avoid known focus-test interference; no-run builds may use four compiler workers, never concurrent build processes. Consistent MSVC_ROOT. Preserve module line budgets rather than padding or compressing unrelated code. No global tool configuration changes. All product changes require regression coverage.
