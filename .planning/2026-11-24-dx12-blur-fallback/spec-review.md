# Independent Spec review: applied blur fallback

Date: 2026-11-24. Reviewer: `/root/common_spec_review`.
Baseline: `32096285f3d5ae33b4e512de61299350f7f2902b`.
Scope: working-tree production/document/registration diff and new fallback tests;
user `.vscode/` excluded. Specification: completion `blur-fallback-next.md` and
this milestone's task plan.

## Result

**0 confirmed actionable findings.**

All 13 applied A/B payload files match their independently reviewed external
candidates after CRLF/LF normalization. No semantic source, test or manifest
drift was found. The added DX12 guide text accurately describes the applied
behavior and does not present pending full-matrix work as already verified.

- The sole production change retains prior-error/released guards and existing
  device-space raw validation, then delegates unsupported capability with the
  original logical draw. Supported planning is unchanged; nested rounded-rect
  dispatch scales once and preserves the shared field-forwarding contract.
- Real native pixel/composition tests retain independent color/clip/corner/order
  probes at scales 1, 1.5 and 2, including absent/transparent tint, stack precedence,
  effective opacity zero and invisible geometry. Diagnostics match actual
  rounded work without manufacturing exact submitted work for clipped commands.
- Validation covers malformed fields even when tint is absent or geometry hidden,
  endpoint/device-scale overflow, unchanged previous successful diagnostics and
  next-frame recovery. Sigma normalization and previous-error behavior remain.
- Native supported/fallback/restored controls cover stale target retirement.
  Real RemoveDevice must produce typed retry, not successful presentation; later
  generation/capture/debug checks retain the normal recovery contract.
- Documentation distinguishes private capability-state injection from an actually
  incapable GPU, preserves explicit unsupported-window errors, and records the
  existing CheckFeatureSupport ambiguity instead of claiming to resolve it.

## Evidence and limits

Read root's saved Stage A pixels RED: actual DX12 WARP/software identity,
present error 4 (`unsupported_platform`), process exit 3. Read Stage B log:
all five native tests pass with matching identity and scale coverage. These
are root-run evidence, not executions performed by this reviewer.

No build, test or mutation experiment was run here; only this planning review
file was written. Root still owns the final source freeze and broader
Windows/WSL/hardware/WARP/debug/GPU-validation/ASan/SDK matrix. This bounded
contract fix does not close the complete DX12 goal or imply release readiness.
