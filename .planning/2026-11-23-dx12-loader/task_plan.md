# DX12 startup independent of the Vulkan loader

Baseline `2a3e9bc6`, branch `phase-v`. Preserve user `.vscode/`; no push.
Root owns serial builds/tests; helpers own distinct tests/reviews only.

## Authorized cleanup

User authorized old SDK cleanup. Verified 12 non-release installed verification
stages, exact manifest matches, no reparse paths and independent library copies.
Removed only 140 `.lib` copies (8.02 GiB), retaining headers, consumer artifacts,
manifests and logs. Latest client-alpha hardlinks, earliest incomplete-manifest
stage, source and target libraries remain. Evidence:
`D:/Dev/shots/cgpui-sdk-cleanup-20261123/report.json`; D: recovered to 8.16 GiB.

## Scope and red signal

Normal Windows dispatcher/SDK consumers selecting DX12 must not import or load
Vulkan. Vulkan remains default/auto; explicit errors never fall back. Compile-time
Vulkan SDK headers remain required. Linux direct linking and alpha/menu/topology
behavior stay unchanged. No public loader option or fault-injection environment.

`scripts/dev/test.ps1 -p tests/renderer-dx12 dx12_loader_independence_test`
was RED: exit 1, "Vulkan loader was imported before DX12 selection", 0.08s.
`dumpbin /imports` also shows a normal `vulkan-1.dll` import in the dispatcher
consumer. Evidence: `D:/Dev/shots/cgpui-dx12-loader-20261123/`.

Design options considered: typed private runtime bindings; MSVC delay-load with
complete import admission/SEH and SDK flag propagation; a separately deployed
backend DLL. Chosen typed bindings remove ordinary imports without growing the
public/SDK interface. Delay-load flags are not currently preserved by SDK link
metadata; separate backend deployment is unnecessary for this milestone.

## Ownership and checks

- Private Windows loader owner, typed required-symbol list and pure binding
  leaf; no broad dispatcher/renderer implementation dumping ground.
- Gate before first Vulkan call. Bind a complete candidate before publication;
  serialize admissions, retry failures, pin successful code for process lifetime.
- Safe default DLL search (app/System32/explicit user dirs), not CWD/PATH.
- Keep existing optional version/device-extension queries optional.
- Native public fresh-process and owned app-local invalid-loader checks, typed
  binding failure checks, normal Vulkan and Windows/Linux regression.
- Architecture/SDK graph guards, installed-only consumer with no Vulkan link
  dependency or extra linker flags, independent Standards and Spec review.
- Full frozen-source Windows/WSL/native/ASan validation and local commit.

## Completion

- [x] Authorized cleanup and retained evidence.
- [x] Public RED reproduced, private implementation and structure guards.
- [x] Native independence/isolation/retry, binding and concurrency checks.
- [x] SDK graph/generator/consumer changes and updated documentation.
- [x] Independent Standards/Spec review, zero actionable findings.
- [x] Full frozen-source validation and installed SDK acceptance.
- [x] Local commit only; preserve user .vscode, no push.
