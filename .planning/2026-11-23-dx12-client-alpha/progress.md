# Implementation and validation progress

Baseline: `4ce08c0a`, branch `phase-v`; evidence date 2026-11-23.
Evidence directory: `D:/Dev/shots/cgpui-dx12-client-alpha-20261123/`.

## Completed implementation

- Reproduced the initial client-alpha factory refusal before enabling the
  bounded route (`client-initial-red.log`, exit 7).
- Reproduced existing opaque client/HMENU overlap and rejected the attempted
  NCCALCSIZE adjustment. The locally passing captionless probe is not adopted:
  it violates the documented `WS_SYSMENU`/`WS_CAPTION` contract.
- Preserved probe sources on local branch
  `codex/prototype-dx12-client-menu-20261123`, commit
  `bb343e0a77a9ad1d99b56d51c31b6a3c31b410bc`. Production has no prototype files,
  native-style rewrite, replacement menu or geometry adjustment.
  (That branch was deleted on 2026-11-25 without merging; its source is gone.)
- Enabled initial no-application-HMENU alpha in the private platform owner;
  froze the initial decorated system/client title-bar mode while leased.
- Added focused RAII menu admission with read-only registry preflight before
  mutation and transaction reentry protection. Refusal test was RED (exit 4)
  before the policy, then GREEN. Real install/clear callbacks each exercise a
  menu-free target, so the reentry test cannot pass merely due to `GetMenu`.
- Added six public native tests plus focused shared test support, structure
  coverage and platform lease expectations. Controls assert actual native
  state, not an unimplemented `WindowMinimized` event.
- Updated backend/title-bar/backdrop docs. Independent Standards, Spec and
  documentation cross-checks reported zero actionable findings; see review.md.

## Frozen-source validation

- Snapshot: 2,770 source files, excluding `.planning/` and user `.vscode/`.
- Full Windows no-run: 739 built, 0 failed (`windows-final-build.log`).
- Full Windows execution: 739 passed, 0 failed (`windows-final-test.log`).
- Windows all-member build passed (`windows-all-build.log`).
- The external WSL harness first failed before source sync: its CRLF line
  ending made Bash parse `pipefail\r` as an invalid option. Confirmed 12 CRLF
  lines and no standalone CR, normalized only `verify-wsl.sh` to LF, preserved
  `wsl-driver-crlf.log`, and resumed at WSL with `verify-remaining.ps1`. No
  production/source/test oracle changed, and no suite result was discarded.
- WSL full execution: 641 passed, 0 failed; all-member build passed.
  All 2,770 frozen paths match the dedicated mirror with CRLF/LF normalization
  only for non-NUL text (`wsl-source-match.json`).
- Intel adapter 0, NVIDIA adapter 1 and WARP each passed all 70 native DX12
  tests with debug enabled; WARP also enabled GPU validation. Each log contains
  all six new client tests exactly once and no skipped test (`matrix-summary.json`).
- ASan built and passed all 70 native tests. SDK graph/header fixtures passed
  4/15 tests, and the external installed-only consumer compiled, linked and ran.
- Final Windows source check passed: 2,770 files, no new/removed/changed paths.
  Full results and exact limitations are recorded in acceptance.md.

All builds/tests are owned by root and run serially. The WSL destination is the
dedicated `/root/cgpui-logic-cleanup-20261123` mirror, not a user checkout.
No existing SDK copy has been deleted; no cleanup authorization was received.
The installed-only SDK consumer used copied external headers and same-volume
hardlinked libraries after all builds, avoiding another roughly 700 MiB library
copy. This verifies installed-header/link closure, not an immutable independent
release snapshot; later in-place changes to the original libraries could affect
the links. No production build runs after that consumer within the driver.
