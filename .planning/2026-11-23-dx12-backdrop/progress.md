# Progress

- Confirmed baseline f81fa683; only pre-existing untracked `.vscode/`.
- Scoped continuation to backdrop blur, keeping transparency, recovery and
  default-switch gates separate. Delegated disjoint neutral/native/test owners.
- Public edge -> blur -> foreground tracer ran RED, present exit5 unsupported.
  Evidence root: `D:/Dev/shots/cgpui-dx12-backdrop-20261123/`.
- Existing Vulkan planner has potential finite-sigma square/int conversion
  overflow at extreme magnitudes. Preserve ordinary semantics during extraction,
  then lock down and correct that boundary with an explicit regression.
- Neutral extraction2/2, four independent headers and shared-shader ownership
  passed after their REDs. All five Vulkan SPIR-V and four previous DXIL
  instruction payloads remain identical to baseline; only source/flag metadata
  changed. New backdrop DXIL validates with dxc.
- Slang combined Sampler2D.Load generated an invalid DXIL/HLSL overload. The
  canonical compile command reproduced it; explicit DX12-only t0 Texture2D/s0
  SamplerState fixed compilation while preserving Vulkan binding and algorithms.
- First native WARP+Debug pixel tracer passed, followed by20/20 existing+new
  native tests and focused native ownership GREEN. No-blur frames allocate no
  effect GPU targets; each visible effect retains its own snapshot/ping/pong.
- Extreme finite-sigma regression demonstrated radius=-2147483648 and sigma=inf
  in the inherited planner. Preserve the ordinary float path, widen overflowing
  squares only, and clamp radius before integer conversion; neutral14/14 passed.
  A separate newly-public extent representability boundary is under test.
- Extent RED caught unrepresentable sizes, float-rounded frame boundaries and
  ceil-division issues. Reject sizes above INT32_MAX, use the original float
  calculation through2^24 and a private double path above, and avoid ceil-div
  addition overflow. Neutral suite now15/15, architecture200/200 with old caps.
- Public effects slice passed independent std::erf Gaussian-step checks for
  sigma4/12/32/64 and literal rounded/tint/clip/opacity/translation at DPI1/1.5/2.
  Mixed slice passed two sequential effect snapshots including intervening draws
  and image/native-glyph/shadow binding restoration. WARP+Debug for each slice.
- Standards review reports0 actionable findings. Spec static review reports no
  confirmed deviation; remaining lifecycle/validation tests are still pending,
  so no full milestone acceptance has been claimed yet.
- Lifecycle and validation slices passed on WARP+Debug: all five blur tests
  passed together. Final affine slice independently passed rotation/nonuniform
  scale/shear against a nonuniform screen-space Gaussian edge at DPI1/1.5/2.
  No tolerances were tuned to measured GPU output.
- Independent native correctness review found no new issue in barriers,
  descriptor/resource lifetimes, state restoration or failure handling.
  Real backdrop demo built and completed a hidden+explicit-paint first frame.
- Froze2664 source files in source-snapshot.tar/.json; started the final
  Intel/NVIDIA/WARP+GPU-validation native matrix before full serial verification.
- Final native25/25 passed on Intel+Debug, NVIDIA+Debug and WARP+Debug+GPU-based
  validation. Spec closure reports0 actionable findings across the six new
  public tests and implementation scope. Full serial Windows/WSL/SDK/ASan now
  running against the frozen sources.
- Frozen Windows full suite passed685/685, all library/example builds passed,
  installed-only SDK consumer compiled/linked and verified text/image/shadow
  plus actual backdrop edge smoothing. WSL mirror validation follows serially.
- Frozen WSL full suite passed633/633, all libraries/examples and installed-only
  SDK compile/link/run passed. Scoped Windows DX12 ASan+WARP+Debug passed25/25;
  serial validation driver exited0. No native macOS claim.
- Final backdrop demo and workbench hidden+explicit-paint first-frame smokes
  exited0. Canonical DXIL check and source hash comparison passed; all2664 files
  remain identical to the tested snapshot. git diff --check clean.
- Closing this milestone in one local commit with Standards0/Spec0 outstanding;
  no push/publication. Preserve `.vscode/`. Transparency/Popup, device recovery,
  exceptional-sync retirement, diagnostics/performance and default-switch gates
  remain explicit pending work rather than full-DX12 completion claims.
