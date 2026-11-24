# Progress

## Investigation and decision

- Baseline `65eb6ae6`, branch `phase-v`, initially only untracked `.vscode/`.
  Reused existing research agents. Read the implementation/prototype/debugging
  skills and the adopted popup policy; no new goal, automation, push or worktree.
- Initial five-recipe WARP probe found no simultaneous alpha/menu-marker pass.
  Expanded to eleven recipes to distinguish whole-client blur, GLFW-style
  region, full vs positive-bottom margins and explicit `DWMSBT_NONE`.
- Final probe recorded pre-/post-DComp native menu ROIs and owned-window BMPs
  across WARP, Intel and NVIDIA. All 33 setups and GPU captures succeeded;
  eight recipes per adapter displayed real alpha but none preserved native menu
  drawing. The standard native text matters: some candidates retain glyphs but
  turn all 553 reference gray pixels `(160,160,160)` into `(160,255,160)`.
  Independent review confirmed the same result. Do not describe every menu as
  blank, and do not conclude that all possible topologies are impossible.
- The throwaway prototype is archived on local branch
  `codex/prototype-dx12-ordinary-20261123`, commit
  `3cde7234d70ab341b10be8a7b4ae7f072e636f10`, and in the evidence folder. It and
  its temporary mcpp registration were removed from the production checkout.
  (That branch was deleted on 2026-11-25 without merging; its source is gone.)
  No ordinary DX12 support is advertised; no native menu is hidden or replaced.

## Runtime bug: RED -> GREEN

- Added public `WindowRuntime.run`/`try_open_window`/redraw tests, recording the
  `RenderFrame::clear` and `present` calls rather than testing only a private
  helper. An initial ambiguous test `View` identifier was qualified before RED.
- Both tests then failed with exit 1 on the intended symptom: four explicit
  pre-/post-alpha root/secondary cases in the transparency matrix; four explicit
  alpha cases in the dynamic-backdrop matrix. Other controls remained correct.
- Diagnosis: surface creation carries explicit chrome intent, but record clear
  fell through to a material-only helper and root rendering bypassed the record.
  Minimal fix keeps intent/live-capability selection in the backdrop leaf and
  passes the root record at the draw call. Popup/material contracts stay intact.
- GREEN: two public runtime programs and the backdrop structure program passed.
  Expanded tests include live composition changes without changing the original
  request, material removal/reapplication/refusal and creation-descriptor
  invariance. Runtime leaf remains 53 lines, header 33, caps unchanged.
- Independent Standards review: zero hard violations or smell findings. Spec
  review: zero actionable findings. Existing cached applied-material behavior is
  not a new promise of full material renegotiation after capability changes.

## Frozen verification

- Source freeze: 2731 files. Excludes build outputs, dated planning notes and the
  user's pre-existing `.vscode/`. Temporary probe is not in the freeze.
- `verify-final.ps1` exited 0: Windows 719 and WSL 639 tests passed, both all-member
  builds passed, and the two new runtime programs passed ASan. Architecture 204,
  UI 108 and native DX12 53 are included in the Windows full-suite counts.
- Windows raw-byte and WSL normalized-source comparisons both match all 2731
  frozen paths. Only excluded closeout planning notes changed afterward.
- Final independent docs/evidence review also found no actionable mismatch.
  Native macOS, dynamic ordinary DX12 route switching and release gates remain
  outside this delivered slice. Local implementation commit closes only this
  bounded slice; no push.

Evidence: `D:/Dev/shots/cgpui-dx12-ordinary-20261123/`.
