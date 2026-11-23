# Composition admission review

Baseline `4d29c4e5`. Review covers actual dirty source including new leaves,
not merely the tracked diff. No default switch or whole-DX12 completion claim.

## Independent Standards

`/root/composition_standards_final`:0 actionable findings initially and after
coverage corrections. Private constexpr flags owner, extracted resize owner,
declaration-only support headers, focused native/Renderer tests and existing
caps were retained. Obsolete probe comment about production flags0 was corrected.

## Independent Spec

`/root/composition_spec_final` initially identified two acceptance gaps:
1. Strict zero-poll cold resize before the first admitted frame.
2. Exact handle accounting for failed post-handle construction and composition
   live-peer purge/lazy rebuild, not only isolated renderer recovery.

Added cold64->96 actual HWND/chain resize with native readiness and96px capture,
then post-Present96->64 with a new ready observation. Existing Popup lifetime and
recovery tests now use strictly forwarded native pacing observation. Duplicate
target construction closes only its own handle; original renderer retains real
pixel output. Orphan/destroyed-HWND purge and live peer lazy reconstruction check
each epoch separately, preserving borrowed-capture and old-frame assertions.

Independent source re-review: both gaps closed,0 remaining findings. This is
static review; execution evidence is separately recorded below and in acceptance.

## Pre-freeze execution

Root only, serial jobs4 compilation/jobs1 native execution:
- Original A hidden/visible on real renderer: both RED exit40 before enablement;
  both GREEN after enabled admission. No pre-Present/pre-Commit warming.
- Six explicit raw-probe rows: Intel/NVIDIA/WARP x hidden/visible, all exit0.
  These are capability experiments, not production lifecycle/performance proof.
- Initial native118/118, then new lifecycle121/121, then final review-gap
  coverage122/122 under WARP+debug. No failures in these three runs.
- Architecture225/225, including all3 new guards and unchanged existing caps.

## Frozen source

Evidence `D:/Dev/shots/cgpui-dx12-composition-pacing-20261124`.
2969 source paths in `source-snapshot.json/.tar`, excluding user `.vscode` and
planning records. Initial raw Windows source comparison matches. Serial complete
matrix driver launched after final coverage source and independent re-reviews.
Acceptance is pending until that driver and final source checks finish.

## Late doc review and SDK harness correction

The first full matrix passed Windows827,WSL667,each native adapter122,common94
per selected backend and ASan122/common94+94. Its installed SDK stage failed
before consumer creation: selective test commands had replaced the production
`build.ninja`, removing the workbench link edge needed to derive SDK closure.
Saved before/after graphs, reran the all-member production build, and the same
SDK verifier passed actual compile/link/run and source comparison. This is an
acceptance-harness ordering correction, not a DX12 failure or a skipped SDK test.
Initial failed pipeline and the separate successful diagnostic remain preserved.

A later independent doc reread found the old composition-disabled timing sentence
and imprecise unconditional-poll wording. Both current docs now match enabled
admission and cached-grant behavior. No production/test code changed. Final
scripts and a fresh whole-source snapshot are in the evidence `final/` subfolder;
the full matrix is rerun there rather than claiming old bytes verified new docs.
The final driver refreshes the production graph immediately before SDK closure.

Final driver completed exit0. Exact inventories, no native/common skips and both
source comparisons passed; see `acceptance.md` and final `matrix-audit.json`.
