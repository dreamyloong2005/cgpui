# DX12 milestone: exceptional synchronization retirement

Baseline `1964c77c`, branch `phase-v`; user requested continuation. Preserve
untracked `.vscode/`, the separate design checkout and the user's Linux checkout.
No push/publication. This slice is a prerequisite, not automatic device recovery.

## Contract

- Retain submitted GPU ownership until the queue's successful fence ticket is
  observed complete, or actual device removal is observed. Never infer completion
  from an event wake alone, a failed wait, or a failed Signal.
- Persist successful pending tickets across errors; never replace their evidence
  with a CPU fence Signal. Event failures may use completed-value polling.
- Destruction must drain safely, not ignore a failed wait. A permanently failing
  queue Signal on a device that never reports removal can block destruction;
  do not pretend a timeout makes release safe. Back off instead of busy looping.
- Do not force RemoveDevice: D3D12 devices can be shared by multiple renderers
  on the same process/adapter. Real removal is terminal, never frame_retry_required
  or a successfully committed upload. Automatic recreation remains future work.
- No public test hook, environment fault switch, renderer mode, default switch,
  transparent window implementation or Popup-policy change.

## Ownership and acceptance seams

Private synchronization protocol/native-call adapter and teardown retirement
remain focused DX12 leaves. The state destructor only delegates retirement.
Private per-state native-call injection tests exercise the actual synchronization
and state destructor; public renderer tests exercise real Device5::RemoveDevice.
This internal seam is necessary to distinguish event wakes from queue completion
deterministically. Add architecture guards without raising existing caps.

- popup_structure: private synchronization/native adapter, deterministic RED
  tests, then fix after root executes RED; no builds.
- repo_progress: public real-device-removal test; no builds.
- gpui_windows_design: structural tests and independent review; no builds.
- root: registrations, integration, docs, all serial builds/tests and evidence.

## Sequence

1. [done] Minimal fault seam and executable RED on the existing algorithm.
2. [done] Ranked hypotheses, minimal lifetime/ticket fix and GREEN.
3. [done] Independent Standards/Spec/native reviews and regressions.
4. [done] Frozen Windows/WSL/native/ASan verification and acceptance.
5. [done] Local commit only (this changeset); no push/publication.

Use mcpp wrappers. MCPP_JOBS=4 for no-run builds, 1 for native test execution.
Evidence: `D:/Dev/shots/cgpui-dx12-retirement-20261123/`.
