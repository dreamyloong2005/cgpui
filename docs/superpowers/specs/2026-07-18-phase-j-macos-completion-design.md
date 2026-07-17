# Phase J macOS Completion Design

Date: 2026-07-18
Status: approved

## Context

Phase H completed production Cocoa and Metal behavior and recorded a native
macOS Debug baseline. Phase J subsequently completed its upstream audit,
official examples, performance, stress, documentation, release, and final
verification bands with macOS execution explicitly excluded. The current
authority therefore reports `complete_non_macos` even though the macOS product
backend itself is complete.

This work reopens the existing Phase J Steps 799-840 contract and makes macOS a
fully verified release platform. It does not create a new phase or use the
Steps 841+ upstream-drift reservation.

## Decisions

- Phase J is reopened in place rather than receiving a separate addendum.
- macOS release artifacts are produced separately for `arm64` and `x86_64`.
- Both architectures receive native Debug, example, performance, stress, and
  Release execution.
- The minimum deployment target is macOS 13.0.
- GitHub Actions uses pinned native runners:
  - `macos-15` for arm64.
  - `macos-15-intel` for x86_64.
- Implementation is pushed to a feature branch. It is merged into `master`
  only after every macOS matrix job passes.
- Existing Phase H, Windows, Wayland, X11, and prior Phase J non-macOS evidence
  remains present as historical evidence even when the current Phase J status
  becomes `complete`.

## Goals

1. Compile and exercise all 20 pinned official-example equivalents on macOS.
2. Run the Phase J performance baseline and all eight stress scenarios on both
   macOS architectures.
3. Produce deterministic, manifest-verified Release archives for arm64 and
   x86_64.
4. Add macOS to Phase J dependency setup, CI, architecture/header checks, and
   final verification.
5. Change the current Phase J authority from `complete_non_macos` to
   `complete` only after both native architectures pass.

## Non-Goals

- Universal 2 artifacts.
- Code signing, notarization, DMG creation, or App Store distribution.
- iOS, visionOS, or Catalyst targets.
- A new upstream GPUI snapshot or Steps 841+ parity delta.
- Rewriting historical Windows/Linux or Phase H result values.

## Module Ownership

### CI Scripts

Focused scripts own each macOS workflow:

- `scripts/ci/macos-package.sh` builds and stages one architecture-specific
  package tree.
- `scripts/ci/macos-phase-j-release.sh` creates and verifies one deterministic
  Release archive.
- `scripts/ci/macos-performance-baseline.sh` runs and validates the shared
  performance runner.
- `scripts/ci/macos-stress-matrix.sh` runs and validates the shared stress
  runner.
- `scripts/ci/macos-architecture-header.sh` runs the macOS structure and public
  header matrix.
- `scripts/ci/macos-example-smoke.sh` remains the macOS public-example runner
  and is extended to satisfy the 20-example Phase J matrix.

The root Xmake file remains a thin composition point. New audit targets live in
`build/xmake/phase_j_macos_targets.lua`.

### Release Packaging

`scripts/ci/create-release-artifact.py` remains the structured archive writer.
It gains a `macos` platform and architecture metadata while retaining stable
path ordering, normalized ownership and modes, fixed timestamps, and SHA-256
content manifests.

Each package contains the public headers, architecture-specific static
libraries, the public demo executable, dependency/build metadata, and
`manifest.json`. The release wrapper creates the same archive twice from one
package tree and rejects any byte difference.

Artifact names are fixed:

- `cgpui-macos-arm64-release.tar.gz`
- `cgpui-macos-x86_64-release.tar.gz`

The corresponding manifest records `platform: macos`, the exact architecture,
`mode: release`, and `deployment_target: 13.0`.

### GitHub Actions

`.github/workflows/phase-j-windows-linux.yml` is renamed to
`.github/workflows/phase-j-desktop.yml`. Existing jobs retain their behavior.
macOS is added as native architecture matrices for:

1. Debug plus official examples.
2. Performance plus stress.
3. Release artifact creation.
4. Architecture and public-header verification.

The shared Phase J dependency action gains a Darwin branch with repository-local
Xmake configuration, package cache, install, global, and temporary roots keyed
by runner architecture and the existing lock inputs.

All macOS jobs export `MACOSX_DEPLOYMENT_TARGET=13.0` before configuration and
verify the process architecture matches the matrix architecture.

## Verification Data Flow

For each architecture, CI performs these steps in order:

1. Restore the locked Xmake 3.0.9 dependency environment.
2. Clean-configure Debug and run the complete registered test suite.
3. Compile all official-example equivalents and execute macOS smoke, pixel, and
   input coverage.
4. Run `phase_j_performance_runner`, retain its JSON report, and validate it
   against the checked-in regression budgets.
5. Run `phase_j_stress_runner`, retain its JSON report, and validate all eight
   scenario floors.
6. Clean-configure Release, build the package payload, and validate the package
   manifest.
7. Create the Release archive twice and require byte identity.
8. Re-read every archived file hash and verify the expected archive entry set.
9. Upload architecture-specific Release, performance, and stress artifacts.

The official-example matrix changes each macOS platform entry from
`excluded_by_user` to `compiled_and_smoked` and records the macOS-native
verification targets. Performance and stress policy files list macOS arm64 and
x86_64 as active platforms.

## Error Handling

Every macOS shell entry point uses `set -euo pipefail` and rejects:

- A non-Darwin host.
- A host architecture different from the requested architecture.
- An output root outside the repository or equal to the repository root.
- Parent traversal in an output argument.
- An Xmake version other than the locked 3.0.9 line.
- A package manifest with a different platform, architecture, mode, or
  deployment target.
- A missing staged library, header set, demo, or metadata file.
- A differing verification archive or mismatched manifest hash.
- An incomplete performance metric or stress scenario report.

No retry hides a deterministic product, packaging, or test failure. If native
Cocoa/Metal behavior fails on either architecture, the fix belongs in the
focused platform or renderer module with a regression test.

## Test Strategy

### RED

Add `tests/api_parity/phase_j_macos_completion_test.cpp` and
`tests/architecture/phase_j_macos_completion_structure_test.cpp`. They fail
while macOS is excluded from Phase J and require the new scripts, Xmake module,
workflow jobs, policy entries, dual-architecture artifacts, and final authority
fields.

### GREEN

Implement one Phase J band at a time:

1. Official examples.
2. Performance.
3. Stress.
4. Release packaging.
5. CI and architecture/header coverage.
6. Final dual-architecture evidence and closeout.

Each band receives focused tests before the final suite.

### Native Acceptance

The current arm64 Mac runs the complete local matrix, including deterministic
Release reproduction. The feature branch then runs both native GitHub runner
architectures. Final authority is updated only from successful native job
summaries and downloaded artifact audits.

The final closeout requires:

- arm64 full Debug, examples, performance, stress, Release, and structure
  evidence.
- x86_64 full Debug, examples, performance, stress, Release, and structure
  evidence.
- Deterministic archive hashes and verified content manifests for both
  architectures.
- All pre-existing Windows, Wayland, X11, and historical Phase H guards green.
- Valid JSON, clean Markdown links, structure line budgets, and
  `git diff --check`.

## Authority Updates

The roadmap, Markdown ledger, JSON ledger, final verification JSON, policies,
README, platform dependency guide, task plan, findings, and progress log are
updated together.

Current fields move from non-macOS closure to complete desktop closure:

- `phase_j_status: complete`
- `phase_j_steps_829_834_status: complete`
- `phase_j_steps_835_840_status: complete`
- `phase_j_required_gaps: 0`

The final verification document changes its scope to all active desktop
platforms and adds separate arm64 and x86_64 Debug, Release, performance, and
stress records. Historical non-macOS evidence remains explicitly labeled and
unchanged rather than being presented as if it originally included macOS.

## Delivery

Implementation occurs on `codex/phase-j-macos-completion`. The branch is pushed
to trigger native GitHub Actions. Failed jobs are diagnosed and fixed on the
same branch. After both architectures and the complete regression matrix pass,
the branch is merged into `master` without force-pushing or deleting historical
evidence.
