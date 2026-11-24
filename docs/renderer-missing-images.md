# Missing image requests (DX12 / Vulkan)

A valid image request whose pixels are unavailable does not reject the frame:
other ordered drawing continues. Visible omissions remain planned but unsubmitted
resources, with `missing_submission_resource` and `work.exact_match() == false`.
There is no setting or placeholder rendering. Fully clipped images keep each
backend's existing planning convention; this change does not normalize all
cross-backend diagnostic resource indices or batch grouping.

Nonzero identity, positive source pixel extent, supported format/sampling, finite
raw and derived draw values, and compatible allocation identity are still
required. Same-ID conflicting descriptors reject even if the ID has no pixels.
Stride and byte-size are upload metadata, not allocation identity or readiness.
Final actual upload bytes retain the backend's established stride/length rules.
Invalid uploads and real native/GPU failures are never translated into missing.

DX12 resolves typed absence only after final frame invalidations/uploads. A CPU
snapshot lacking native realization still follows normal lazy upload and failure
propagation. Missing markers retain original command identity and own no texture.
Final-ID batching, abandonment, immutable retention and capture publication
remain owned by their existing modules. No arbitrary failed-Present rollback is
promised; callers can explicitly resupply on a later healthy frame.

Vulkan diagnostics consume the current successful recording result's selected,
visible image indices, including command reuse. They must not use request count
or a stale stored signature (capture/upload recordings do not commit signatures).
Vulkan cache compatibility is validated before mutation; explicit compatible
final uploads or prefix invalidation can replace an old allocation identity.
Existing Vulkan unavailable-resource allocation/lazy upload is not redesigned.

Metal is an explicit follow-up: its missing-image rejection, raw validation and
submission accounting have not been adapted here. Windows/WSL results cannot
certify Metal or native macOS. This source change by itself is not runtime proof.

## Acceptance

Run the same `renderer_missing_image_contract_test` with explicit DX12 and Vulkan
backend selection. It uses fresh black/cyan/magenta/yellow captures and exact
interior probes: surrounding draw order, uncovered missing background, later
upload/caller ownership and cached drawing. Exit40 denotes valid missing input
rejected; exit41 denotes correct pixels with false missing accounting. Setup,
other native failures, wrong pixels and available-image diagnostics are separate
failures, not the intended differential RED. No raw Present retry is permitted.

Also run the shared native invalid-input controls, shared CPU validation, DX12
marker/cache/prefix/eviction/retention/orphan/failure suites, Vulkan selected-image
accounting/cache preflight/reuse suites, and focused architecture tests. Confirm
native error propagation separately; success of these inputs is not fault-injection
coverage. Tests must be built/run by the engineering owner, not inferred from code.
