# Progress

- Confirmed clean tracked baseline `4e0455f0`; only `.vscode/` was untracked.
- Read the separate design's primitive/shader/fence sections without editing it.
- Scoped continuation to text/images and reusable texture infrastructure; opaque
  experimental rendering remains opt-in. Delegated disjoint native, shared
  planning and text modules; root owns all builds and tests.

- Actual RED: shader output ownership missing text DXIL; native image upload
  rejected (exit5); native text rejected (exit13); neutral geometry lacked its
  new leaf header/owner. Logs are under
  `D:/Dev/shots/cgpui-dx12-textures-20261123/`.
- Shared Slang text/image shaders now generate validated embedded DXIL; all five
  Vulkan SPIR-V instruction payloads remain identical to baseline. Shader owner
  regression passed. Metal outputs were not regenerated.
- Neutral planning extraction passed9/9; six independently compiled public leaf
  header tests were added to the suite and CI matrix. Existing module caps stay.

- Native text/image substrate first passed8/8, then the expanded13-test suite
  passed on WARP/Intel/NVIDIA with Debug Layer and WARP GPU-based validation.
  A final invalid-image regression also passed after fixing its test-only
  Windows max-macro compilation collision (not a product defect).
- The 120-idle-frame cache regression failed with exit11 before eviction was
  implemented; afterward eviction/sampling/cache and mixed primitives all pass.
  Text tests cover DirectWrite glyph masks/whitespace, DPI, clipping/opacity,
  same-frame page changes, multiple pages and recoverable invalid glyph input.
- Architecture198/198 and headers40/40 pass. Hello-window's real demo smoke
  (input/text/clipboard/redraw/close) also passed with explicit DX12 + WARP.
- Independent Standards and Spec reviews found no outstanding actionable issue.
  Spec review emphasizes per-texture heaps/synchronous uploads are incremental,
  not the full phase4 or planned descriptor-pool/retirement architecture.
- Product sources frozen for final full Windows/WSL, installed-consumer and
  scoped ASan runs. Public docs explicitly describe record-time CPU cache
  mutations (no abandon rollback) and remaining unsupported capabilities.

- Final Windows full run passed659/659; all libraries/examples built. External
  installed-only headers and copied libraries linked with /MT /OPT:NOREF and
  ran actual WARP text/image capture through create_renderer. The helper's
  ambiguous font brace argument was corrected to explicit FontDescriptor;
  no product change was needed and fresh output is installed-sdk-windows-v2.
- WSL full run passed618/618; all libraries/examples built, installed-only Linux
  consumer linked/ran, SDK header/package-graph fixtures passed15+4. Product
  hashes match the snapshot; scoped DX12 ASan is the remaining running gate.

- Scoped Windows ASan completed14/14 using WARP + Debug Layer. The remaining
  sequential verification driver exited0. Final native hardware/GPU validation
  recheck includes the last invalid-image regression; see acceptance.md.
- Final complete14-test native suite passed on Intel, NVIDIA and WARP GPU-based
  validation. All product/test/build hashes still match the validation snapshot.
  Acceptance records remaining gates and both review axes. Local commit excludes
  `.vscode/`; no remote push or publication.
