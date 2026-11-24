# DX12 WARP opt-in CI

## Status and scope

This is an **experimental, manually dispatched** lane, not a claim of passing
CI or complete native-desktop coverage. No remote runner authorization has been
inspected or configured by this change. No workflow has been dispatched.

- `dx12-warp-common.yml`: GitHub-hosted `windows-2025`, trusted default-branch
  dispatch only, checked out at the event's full commit SHA. Runs selected-WARP
  identity first, then the thirteen shared native pixel scenes, with DX12 debug
  and GPU-validation requested. No PR/push event, arbitrary-ref input, repository
  secrets, deployment or self-hosted runner is involved.
- `dx12-warp-desktop.yml`: **blocked by design**. Its only job is GitHub-hosted;
  it records `authorized=false`, `native_execution=false`, and fails. There is
  no self-hosted job to schedule. Supplying group/label variables or a boolean
  cannot enable it. Full desktop CI remains pending external authorization and
  a separately reviewed workflow change.

Workflow-local branch checks are routing safeguards, not an independent security
boundary: a branch can edit its YAML. The hosted lane uses an ephemeral hosted
machine, read-only repository permission and no repository secrets. Do not add
self-hosted access on the strength of these checks.

## Running the hosted experiment

After this source is reviewed and present on the trusted default branch, select
**DX12 WARP common experiment** in Actions, choose that branch, and dispatch it
manually. This document does not dispatch or provision anything. Other refs
fail before checkout/dependency setup. Checkout is pinned to `github.sha` and
does not persist credentials.

The job reuses the existing pinned mcpp/Vulkan-SDK dependency action and the
existing GitHub-annotation wrapper. Graphics Tools must already be installed and
queryable. Missing capabilities fail the job: no ad hoc Graphics Tools installer,
debug/GBV downgrade, software-Vulkan substitution or blanket skip is used. A
hosted image label alone is not proof of usable WARP, shader-model support, fonts,
debug layer or GPU validation. The real identity and pixel checks are required.

Compilation uses `MCPP_JOBS=4`; execution uses `MCPP_JOBS=1`. Full suite inventories
are derived from the current Windows-applicable `tests/renderer/mcpp.toml` and
`tests/renderer-dx12/mcpp.toml`, not historical totals or a stale JSON snapshot.
The hosted scene selection is a deliberate explicit subset in
`windows_dx12_inventory.py`, checked against the owning manifest. New unknown
conditions/globs, duplicate registrations, missing sources or missing selected
scenes fail closed. The evidence contains exact names and manifest SHA-256s.

## Result and evidence contract

Every run uses a fresh external directory under `RUNNER_TEMP`; artifacts include
initial status, available prerequisites, exact inventory, selected settings,
source hashes, complete per-command logs and exit/time receipts. Result files and
original command logs are not overwritten. Artifacts upload with `always()` when
GitHub can still execute the upload step. A timeout can prevent final receipts;
an initial `pending` file is never a pass. The terminal decision is `result.json`
plus successful workflow exit, not the existence of an artifact.

The checker requires every expected name exactly once, all named results `ok`,
one consistent successful summary per explicit backend run, and no SKIP anywhere
in the output. It rejects zero-match filters and a successful second backend
masking the first backend's failure. Each selected common scene needs actual
requested/expected/actual renderer identity, explicit selection, nonzero LUID,
nonempty adapter, empty fallback, and software-WARP identity for DX12. Environment
variables alone do not prove which native device executed.

The optional `desktop` script lane also runs the full current native suite and
the complete shared suite in Vulkan-then-DX12 order. Those suites include negative
and synthetic-seam tests; aggregate names/exit accounting does not claim that
every case renders, emits identity or enables GBV internally. The common scene
checks provide the selected-WARP identity gate. A skip remains a lane failure,
including unavailable independent hardware for cross-adapter cases.

Source hashes and Git state must remain unchanged across successful execution.
Job limits (45 minutes for the hosted lane) and existing native watchdogs are the
current timeout mechanisms. This change does not add a process-tree/Job Object
timeout guarantee. Fixture/source checks are not GPU acceptance, and hosted
common success is not native desktop/menu, hardware DX12 or macOS/Linux evidence.

## Desktop authorization prerequisite

Keep the desktop lane unprovisioned until an administrator has independently
configured and audited all of the following **before bringing a runner online**:

1. A dedicated organization/enterprise runner group, restricted to this repository.
2. **Workflow access = Selected workflows**, allowing only the exact full path
   `OWNER/REPO/.github/workflows/dx12-warp-desktop.yml@refs/heads/ACTUAL_DEFAULT_BRANCH`.
   Replace the placeholders with verified values. GitHub permits a non-reusable
   workflow pinned to a branch; only jobs directly defined in that selected
   workflow obtain access.
3. Auditable group membership/policy/change controls, no second default or
   all-workflows registration, and actual denial evidence for an unauthorized
   branch/workflow. Group names, labels, a confirmation variable, mutable job
   `if` checks and an environment declaration do not prove this policy.
4. An isolated disposable Windows client image (build 26100+, recorded UBR and
   image ID), preinstalled Graphics Tools, unlocked local Default input desktop
   in the active console session, DWM, required fonts and adequate work area.
   No personal workstation credentials, Session 0 runner or RDP attach/disconnect
   during the run. Full desktop coverage also needs a healthy genuine Vulkan ICD
   and suitable independent hardware for the unchanged cross-adapter tests.

If selected-workflow/ref enforcement is unavailable for the account, **do not
enable self-hosted routing or downgrade to label-only access**. A separately
trusted external controller/control repository would be required; none is
implemented here. The documentation is not evidence of server configuration.

After the external audit, a new reviewed change may replace the blocking job
with a hosted authorization/preflight gate and a directly defined native job
selecting **both** the audited group and isolation labels, pinned source SHA,
serial execution, bounded timeout and always-upload evidence. It must not accept
an arbitrary ref or treat a boolean as authorization. The existing
`windows-dx12-prerequisites.ps1 -Lane desktop` and
`windows-dx12-run.py --lane desktop` provide the native command owners; they do
not provision or authorize a runner themselves.

Official authority documentation:
- [Runner-group repository/workflow access](https://docs.github.com/en/enterprise-cloud@latest/actions/how-tos/manage-runners/self-hosted-runners/manage-access)
- [Combining runner group and labels](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax#choosing-runners-in-a-group)

## Local checker fixtures (not native acceptance)

The focused Python fixtures live under `scripts/ci/dx12_fixtures`, without changes
to existing test manifests or broad CI registration. They cover manifest
derivation, strict result/identity parsing, first-failure receipts and module/
workflow boundaries. The hosted workflow runs them before dependency setup.
For later validation, from the repository root:

```powershell
python -B -m unittest discover -s scripts/ci/dx12_fixtures -p 'windows_dx12_*test.py' -v
```

Implementation was separated from validation: writing these fixtures does not
mean they passed. Python fixtures, PowerShell/C# syntax/runtime checks, workflow
lint, actual hosted execution and eventual authorized native desktop execution
are separate validation gates.
