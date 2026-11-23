# Progress

- Confirmed baseline e4d551a2 with only pre-existing untracked `.vscode/`.
- Scoped next slice to outer shadows, retaining default/popup/recovery gates.
  Disjoint CPU planning, native GPU and public pixel tests delegated; root
  owns shaders, registration and every build/run.

- Native public shadow test demonstrated RED exit5 (unsupported box shadows).
  Neutral missing-leaf/owner and native/shader ownership regressions also ran
  RED before implementation. Evidence root:
  `D:/Dev/shots/cgpui-dx12-shadows-20261123/`.
- Shared CPU shadow parameters/geometry moved unchanged from Vulkan; Vulkan
  retains aliases/forwarders. Three public leaf headers compile independently.
  The new radius-normalization test initially used exact float equality and
  failed at15.0000009537 versus15. Existing near(1e-4) comparison fixed only the
  new oracle, not product behavior. Neutral suite now passes11/11.
- Shadow DXIL compiles/validates via canonical generator. All five Vulkan
  SPIR-V instruction payloads remain identical to baseline. No Metal rewrite.
- Native lazy shadow PSO, finite/index validation, buffer recording and ordered
  dispatch have focused owners; common PSO defaults replace the two identical
  existing state blocks without changing values (independently compared).
- Native first18/18 passed WARP+Debug; additional transform/rounded-soft test
  passes. Gaussian checks use independent std::erf/2D numerical integration,
  not copies of shader approximation. Architecture199/199 passed with old caps.
- Expanded native19/19 passed Intel adapter0 and NVIDIA adapter1 with Debug,
  and WARP with GPU-based validation. SDK Python fixtures4+15 and canonical
  DXIL-only regeneration check passed.
- Standards review found one nonblocking duplication suggestion. Extracted a
  private non-template indexed draw owner shared by rectangles and shadows,
  with structure RED/GREEN. Textured recording and lifetime semantics unchanged.
  Static re-review reports no outstanding Standards finding; Spec review also
  reports no actionable deviation. Final native matrix is being repeated.
- Initial workbench first-frame command used an incorrect executable path;
  no program was run. Correct hidden-process launch then timed out with empty
  logs, so it is NOT a passing smoke. Investigating the hidden-window paint
  trigger separately; no workbench interaction or screenshot claim.
- Hidden workbench diagnosis: its HWND was invisible and still running before
  explicit paint. Posting WM_PAINT only to that spawned process's CGPUIWindow
  completed first-frame shutdown with exit0 (WARP+Debug). This is a harness
  scheduling issue, not a shader fix; product sources were unchanged. The
  ordinary hidden launch remains a recorded timeout, not a passing test.
- Indexed helper GREEN followed by native19/19 on Intel, NVIDIA and WARP+GPU
  validation. Froze2631 source files in source-snapshot.tar/.json and started
  sequential full Windows/WSL, installed consumer and scoped ASan validation.
- Frozen Windows complete suite passed670/670 (including architecture199/199),
  all libraries/examples built, and the installed-only SDK consumer compiled,
  linked and verified actual text/image/shadow pixels. WSL validation follows
  in the dedicated agent-owned mirror; no parallel builds were launched.
- Linux/WSL full suite passed624/624; all libraries/examples and installed-only
  SDK compilation/link/runtime check passed. Scoped Windows DX12 ASan passed
  19/19 on WARP+Debug. The serial verification driver exited0.
- Repeated the qualified hidden workbench smoke against the final rebuilt
  binary: invisible before paint, explicit WM_PAINT, exit0. Rechecked canonical
  DXIL output and all2631 frozen source hashes; no product/test/source drift.
- Final independent Standards/Spec findings are both0 outstanding. Acceptance
  records actual tests, the harness qualifications and remaining production
  gates. Closing this milestone in one local commit; no push/publication and
  pre-existing `.vscode/` remains excluded.
