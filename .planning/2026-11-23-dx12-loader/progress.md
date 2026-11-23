# Progress

- Authorized cleanup completed: 140 old `.lib` copies across 12 non-release
  SDK verification stages, 8.02 GiB freed. All logs/manifests/headers/consumer
  artifacts retained; latest hardlinked SDK and source/target libraries untouched.
- Public startup RED observed: `loader-independence-red.log`, exit 1 before
  backend selection, confirmed ordinary PE Vulkan import.
- Private Windows runtime bindings implemented; Linux linkage unchanged.
- Public startup now GREEN: real DX12/WARP capture, no Vulkan module before,
  during or after rendering.
- Owned invalid-loader child cases GREEN: DX12 still renders; explicit Vulkan,
  default and auto fail with Win32 error 193 and no fallback; invalid DX12
  adapter does not load Vulkan. Same-process retry loads healthy system Vulkan
  after removing only the validated fixture DLL; real pixels and lifetime pin
  are checked.
- Required-symbol unit binding GREEN: all 88 missing-entry cases refused,
  typed complete success, no partial publication or dummy invocation.
- Concurrent native initialization GREEN: 16 threads, 88 stable real exports,
  repeated admission and resident module.
- Architecture GREEN: 207 passed. SDK CMake dependency fixture was RED with
  unconditional Vulkan discovery, then GREEN; 9 graph/README cases now pass.
- Independent Standards/Spec reviews: 0 actionable findings, separate in review.md.
- Frozen source: 2,782 paths, excluding `.planning/` and user `.vscode/`.
- Full serial validation started; do not infer its result from targeted passes.

## Frozen-source results received

- Windows no-run: 744 built, 0 failed; full execution: 744 passed, 0 failed.
- Windows all-member build passed. Native Vulkan paths are part of the full
  renderer suite, not replaced by DX12-only validation.
- WSL 642 passed and all-member build passed; 2,782 manifest paths match.
- Native selector 0/1/WARP runs each passed 72 tests; WARP enabled GPU validation.
- ASan passed DX12 72 and renderer 89; SDK graph/header fixtures passed 9/15.
- Installed-only real consumer and PE checks passed, both with clean app directory
  and owned invalid Vulkan DLL; Vulkan SDK environment removed. CMake configure
  (not binary build) passed with Vulkan discovery disabled.
- Final Windows source identity: 2,782 files, no added/removed/changed paths.
- Exact evidence and remaining limitations are recorded in acceptance.md.

Evidence: `D:/Dev/shots/cgpui-dx12-loader-20261123/`.
The complete source snapshot is used only with the existing dedicated WSL
validation mirror `/root/cgpui-logic-cleanup-20261123`, not a user checkout.
