# Frame-batched image acceptance

Baseline0f6b2c6e, phase-v. User-approved final-ID batching; overall DX12 active.
Root owns runtime acceptance, local commit/no push. Missing-image skip is a
separately approved next slice, deliberately not yet implemented here.

## Implemented boundary

Frame owns copied raw upload bytes, last same-ID replacement and deduplicated
invalidations; draws record owned device-scaled requests in original slots.
After one-shot release and recorded-error/native preflight, focused CPU owners
apply invalidations, final uploads and immutable draw resolution in that order.
Preparation returns CPU errors without poisoning/rebuilding the native device;
local raw request buffers are freed on preparation exits before successful
diagnostic sampling. No arbitrary failed-Present rollback guarantee is added.

Retained immutable CPU snapshots, weak image census, exactly-once successful
aging, native realization/retirement, healthy orphan completion and success-only
capture history keep their existing owners. Existing caps are not raised.
Timing docs exclude the new pre-submit CPU prefix from selected stage sums.

## Actual focused evidence

`D:/Dev/shots/cgpui-dx12-image-batching-20261124` retains the A and B logs.
A actual shared3/3 Vulkan/Intel passes; DX12/WARP three semantic exit40 results
after valid setup/native work: final-ID wrong pixels, prefix cache loss, and
abandoned replacement/invalidation mutating cache. No backend-specific oracle.
B exact same3/3 passes for both. Full native139 WARP/debug and architecture229
pass, including migrated cross-frame8->4->0 retention and32-byte mixed staging.

Actual-source Standards0 and Spec/native-safety0 are recorded separately; all30
slice files match the previously reviewed candidate after newline normalization.
External installed verifier addition is independently source-reviewed0 and adds
fresh captured final-ID/mixed-order/caller-copy, prefix and abandonment checks,
actual image upload4->0 while preserving existing3076-byte retention assertions.
Both installed runs and the frozen matrix subsequently passed as recorded below.

## Frozen full matrix

`D:/Dev/shots/cgpui-dx12-image-batching-20261124/final`,3011 source paths excluding
planning/.vscode/builds. Root serial jobs4 no-run/jobs1 execution. Full graph was
refreshed before installed SDK. Driver exit0 on this slice's first full run;
exact inventories and raw Windows/normalized WSL source audits passed.

| Gate | Observed result |
| --- | --- |
| Windows workspace / all-member build |854 passed,0 failed / passed |
| WSL mirror workspace / all-member build |672 passed,0 failed / passed |
| Architecture |229 passed, existing caps unchanged |
| Intel / NVIDIA native+debug |139 each |
| WARP native+debug+GPU validation |139 |
| Shared DX12 Intel / NVIDIA |100 each |
| WARP shared Vulkan then DX12 |100+100 |
| ASan native / shared both |139 /100+100 |
| PS7 / PS5 driver fixtures |17 /15 passed,2 explicit PS7-only skips |
| SDK graph / header fixtures |9 /15 |
| Installed SDK image contract |final-ID/mixed order/prefix/abandonment, upload4->0 in both runs |
| Installed retained/orphan/capture history |all earlier checks preserved and passed |
| Loader isolation |owned invalid Vulkan DLL run passed; no Vulkan import/load |
| CMake |configure-only passed, not binary/release verification |
| Source/inventory audit |3011 exact Windows and normalized WSL paths, native139/common100, no skips |

Shared100 is a mixed suite, not100 independent DX12 pixel scenes. Native139
includes CPU and controlled native cases. SDK uses actual /MT direct compile,
link and run, not certification of an immutable release artifact.

No native Mac/all-driver/benchmark/release/default-switch claim. SDK staging
uses copied headers and mutable hardlinked libraries. The preceding capture
slice's single unlocated opaque-menu fixture timeout remains a known gap, not
repaired by image batching. Other full-DX12 work remains in completion/task_plan.
