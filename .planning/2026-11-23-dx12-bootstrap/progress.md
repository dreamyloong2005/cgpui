# Progress

- Confirmed baseline `4d49b9ec` and only pre-existing `.vscode/` untracked.
- Re-read the design's phases and current modules. The earlier cleanup moved
  reports/text planning, but solid/rounded geometry remains Vulkan-owned.
- Delegated disjoint selection, shared-geometry and native-DX12 modules. Root
  owns registration and all builds/tests to avoid toolchain/output contention.
- The first milestone is explicitly partial and opt-in. Auto remains Vulkan;
  unsupported drawing is an error rather than silent missing content.

## Implementation and focused validation

- Actual RED: missing native factory header, missing neutral geometry header,
  failing ownership guard, and backend-info override absence. Logs are in
  `D:/Dev/shots/cgpui-dx12-bootstrap-20261123/`.
- Shared geometry and selection checkpoint passed 6/6; the expanded metadata
  test is included in final whole-suite validation. All 197 architecture tests
  passed, without increasing existing module caps.
- DX12 now creates native hardware/WARP device, queue, fence, PSO, HWND
  swapchain; draws shared solid/rounded geometry; captures pixels and resizes.
  Native functions are focused modules rather than a renderer entry-file dump.
- WARP + debug first pass was 5/6: resize readback failed with actual
  `DX12 capture mapping failed (0x80070057)`. Debugging isolated ReadRange.End:
  the final row has no trailing row-pitch padding. Correcting just the range
  passed the original opaque/resize regression (native-capture-{red,green}.log).
- Final focused native suite passed 6/6 separately on Intel Graphics (index0),
  NVIDIA RTX5070Ti Laptop (index1), and WARP with debug layer. WARP additionally
  passed 6/6 with GPU-based validation. Tests cover pixel content, DPI/clip/
  transform/opacity, rounded/interleaved order, invalid input, multiple renderers,
  outstanding/orphan frames, and real dispatcher behavior. No automatic DX12.
- Debug setup was made one-time per process according to the API constraint;
  the old multi-renderer test already passed, so this is preventive hardening,
  not a claimed reproduced device-removal fix. Hardware selection now checks
  shader model6 before accepting each candidate.
- Independent Spec review caught missing dispatcher/DX12 archives in the base
  Windows packaging script. A new graph fixture failed, then passed after both
  archives were added. SDK shader metadata now includes experimental DXIL;
  the graph regression is wired to desktop CI. No package was published.
- DXIL-only canonical shader regeneration/check and dxc inspection passed.
  All five existing Vulkan SPIR-V instruction payloads are unchanged. The full
  all-target generator check reports existing Mac-output differences with this
  host's `2026.1-52-gc8ddf20bb` Slang versus the Mac's recorded `2026.1` compiler;
  Metal sources/outputs were not rewritten and native Mac validation is not claimed.
- Standards review found no new boundary/cap violations. Product sources are
  frozen for full Windows, WSL and external installed-consumer validation.

## Full Windows checkpoint

- First full no-run built642 tests; first serial run was641/642. The sole
  failure was the old owner/type assertion in the clip-composition closeout
  audit (exit70), not its drawing behavior. Four evidence entries now reference
  the actual neutral type declarations; all behavior checks remain unchanged.
  The focused regression passed afterward.
- Expanded the opaque native test to submit both hidden and visible,
  nonactivating HWND frames. WARP/Intel/NVIDIA all passed. These are submission
  and GPU readback checks, not desktop-compositor screenshot assertions.
- Final complete Windows execution passed642/642 (windows-final-test.log).
  Every library/example built; external installed-only headers and copied static
  libraries linked with /OPT:NOREF and ran a WARP DX12 capture through the public
  dispatcher (installed-sdk-windows.log). No private source headers were copied.
- SDK Python regression fixtures pass15+4. Product/test hashes match the source
  snapshot used by the WSL validation mirror. WSL and scoped ASan are running
  serially via external verify-final.ps1; no simultaneous build processes.

## Remaining exceptional-path boundary

The bootstrap does not certify arbitrary GPU failure handling. In particular,
repeated queue Signal/event-registration/wait failures while the device remains
alive have not been fault-injected; destructor retirement after repeated wait
failure needs a dedicated safety review/seam before production/default parity.
Normal submission, resize and frame-outliving-renderer behavior are tested.
Do not add speculative retry loops or describe this as completed device-loss
recovery. Native Metal validation, full primitive support and performance parity
also remain outside this milestone.

## Final verification and closeout

- WSL no-run built609 tests; complete serial execution passed609/609. All Linux
  libraries/examples built, SDK fixtures passed15+4, and the installed-only
  consumer linked and ran successfully. The agent-owned validation mirror was
  reused without touching the user's dirty Linux checkout.
- Scoped Windows AddressSanitizer DX12 run passed6/6 using WARP + Debug Layer.
  The sequential verification driver completed with exit0; no build overlap.
- Final documentation review added the exceptional synchronization boundary,
  normal Vulkan loader dependency in dispatcher consumers, and serial native
  Windows test guidance. Product/test/build sources still match the final
  validation snapshot; only these docs and planning closeout changed afterward.
- Acceptance and remaining gates are recorded in acceptance.md. Local commit
  excludes the pre-existing .vscode/; no remote push or publication.
