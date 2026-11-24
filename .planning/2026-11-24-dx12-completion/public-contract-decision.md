# Public image/capture contract decision

2026-11-24. Direct human answer to the asynchronous contract question:
"沿用现有 Vulkan 语义（推荐）".

Question explicitly selected compatibility: within one frame the final upload
for an image ID supplies that frame's draws; captured pixels retain the latest
successful requested capture. No developer-facing backend policy/window mode.

Implement as a separate reviewed contract slice after the currently frozen
retained-image acceptance. Do not silently change the source under that matrix.
DX12 will converge with existing Vulkan behavior; use common exact-oracle tests
for conflicting sequences, and deliberately migrate backend-specific assertions.
The successful capture candidate must still remain unpublished through all
fallible stages. Retaining a previous successful capture never permits publishing
pixels from a failed candidate. Define/reset/recovery boundaries explicitly.

Image batching is not a promise of transactional rollback after arbitrary failed
Present. Preserve existing Vulkan invalidation-prefix ordering and clarify
abandonment; missing-image behavior and failure codes need separate source audit
rather than inferring extra choices from this answer. Existing immutable internal
snapshots/lifetime safety remain useful; retention instrumentation counts actual
owners and must not be broken by the recording-policy change.

Source evidence and proposed common tests:
`D:/Dev/shots/cgpui-dx12-public-audit-20261124/contract-proposal.md`.
Historical frame-outlives-facade compatibility is separately supported by original
plan/ledger and has a reviewed external orphan-frame candidate. No candidate is
accepted merely by this decision or static review.
