# Findings
- Baseline: public WindowPresentation existed only in the unimplemented DX12 plan, not product code. The revised plan keeps presentation private.
- Baseline: RuntimePopups trusted platform support only. The implementation now intersects platform support with the actual parent/child renderer; Win32's platform probe alone cannot promise custom-renderer support.
- Win32 Vulkan popup/DComp works today. The final user decision explicitly retains that path and rejects the earlier blanket-overlay policy. Rendering-backend identity is not a failure condition.
- DX12 without DComp probe: 32 HWND flip/DWM combinations on local Intel/NVIDIA did not show per-pixel alpha. Evidence D:/Dev/shots/dx12-alpha-no-dcomp-20261122/README.md; limited to tested paths and OS.
- Existing widgets fall back when capability is unavailable or opening/renderer initialization/placement fails. Regression tests now exercise those failure stages with the default popup preference, including interaction and cleanup; successful popups retain the independent-window path.
- Disk preflight C: 5.21 GiB / D: 13.56 GiB; no full ASan cold builds or automatic cleanup.

Build cache: programs.json referenced absent platform/renderer host executables in target/.build-mcpp/host/5cebfa85de88. Retry generated them successfully. mcpp automatically replaced the old 3.8 GiB target after toolchain fingerprint change; subsequent runs must retain consistent MSVC_ROOT.

The existing phase-F architecture tests set stricter activation budgets (70/75 lines) than the new guard initially allowed. Full-suite validation caught that omission in static review. Every architecture/header test also belongs in the CI header matrix, in addition to mcpp discovery; phase_g_architecture_header_matrix_structure_test enforces the second registration.
