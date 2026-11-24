# Actual-source Standards review — missing images and native reuse

2026-11-24. Reviewer: `/root/capture_standards_review/orphan_test_review`.

**Standards findings: 0 actionable findings.** This is an actual-source review
against `ce3b2e8f402741c972c7af00f9c09ca8b5784870`, not a new runtime acceptance.
Scope includes the tracked diff and the inventoried untracked implementation,
documentation and tests: original missing-image 40 paths plus revised native-reuse
13 paths, with two shared registry paths, for **51 unique paths**. `.vscode` and
unrelated work were excluded and preserved.

## Candidate-to-actual identity

Independently compared actual LF-normalized contents against both external
inventories. The actual source is **not wholly identical to the original 40-path
candidate**:

- 37 of its 40 paths still match exactly, including every production change and
  the unchanged Stage-A/Stage-B shared contract oracle.
- `scripts/ci/architecture-header-targets.txt` and `tests/renderer/mcpp.toml`
  contain the reviewed additive native-reuse registrations. They match the
  revised supplement's final registry contents; no prior registration is removed.
- `tests/renderer/renderer_missing_image_validation_test.cpp` has only the two
  inspected Windows macro-safe substitutions at lines 46 and 60:
  `std::numeric_limits<T>::max()` becomes `(std::numeric_limits<T>::max)()`.
  Numeric inputs and assertions are unchanged; neither is a weakened control.
- All 13 actual revised-supplement paths match its inventory. The supplement's
  full patch identity is
  `e69dc9dad2698232813bee7cf7433479a0c8c640d144f1a0e3dbee4d3a3d3243`.

## Ownership, layout and registration

- Shared raw/derived image validation has a native-independent private data and
  declaration leaf (13 lines) and focused implementation (57 lines), owned by
  `packages/renderer` through its existing `src/renderer/*.cpp` glob. DX12 and
  Vulkan retain focused request-validation, resolution and accounting owners.
  Public `renderer_frame.hpp` changes are contract comments only: no new policy
  switch, native detail or non-template implementation is exposed.
- Typed DX12 absence, pending requests and resolved textured draws remain
  distinct. A missing marker owns no texture; the original command kind/index/
  resource offset and sequence remain intact. Only that marker is omitted by
  dispatch; unresolved requests and lookup/validation errors still fail.
- No existing structure cap was increased or removed. Static source counts are
  command recording 201/220, reuse header 114/130, diagnostic resources 138/160,
  Vulkan presentation 180/180 and image-texture frame 63/110. All explicit caps
  in the missing-image and image-batch guards remain satisfied by source counts.
  Native trace/loop/main are 130/150, 55/90 and 56/100. New sequence declarations
  and implementation are focused 21/25-line test-support leaves.
- Existing structure assertions change only the required typed-lookup/variant
  assignment spelling. Added guards cover private ownership, validation before
  omission, original slots, preflight before mutation, actual callback arguments,
  sequence lifetime and both native predicate consumers.
- Owning mcpp source globs and explicit test discovery cover every added source
  and test. Common validation is unconditional; Vulkan CPU cases retain
  Windows/Linux scope; native reuse support/executables remain Windows-only.
  Both new architecture guards are discovered and in the CI inventory. No build
  system replacement or static-initialization GPU selection was introduced.

## Oracle integrity and preservation

- Vulkan production diagnostics use current successful recording-result-owned
  selected source indices in both fresh and reuse branches. The focused helper
  shares the native draw-order cursor and clip resolver, rather than deriving
  submission from request count or stale signature storage.
- The revised native observer forwards each real ABI call first and stores its
  actual parameters in a fixed-capacity recording-epoch sequence. Successful
  Begin replaces it, successful reset invalidates it, free clears it and native
  reuse retains it. Fresh/reused acceptance checks exactly `I0 [D0] [D1] I1`;
  image source indices remain 0/1 with firstVertex 0/6, never compressed after
  omission. Exact count/instance/offset checks, overflow and full order checks
  reject the original P2 witness, duplicates and reordered rectangles/images
  before a reuse success is counted. The same predicate's CPU mutation controls
  remain present. Typed forwarding, real return propagation and RAII restoration
  have not been weakened. The observer does not claim descriptor/pixel proof for
  every uncaptured historical frame.
- Shared native tests retain fresh pixel/order/resupply/caller-ownership controls
  and distinct semantic RED exits 40/41. Malformed-input tests exercise raw
  Present failures, both absent and installed IDs, real upload/descriptor errors,
  capture-history preservation and healthy final resupply. No Present retry or
  arbitrary failed-Present rollback assumption was added.
- Existing cache/batch/eviction migrations alter only valid-missing expectations.
  Invalid uploads/mismatches, final-ID replacement, abandonment, original order,
  immutable ownership, exact 120/121-frame aging, retention bytes and subsequent
  reupload remain covered. Their missing controls now require fresh pixels and
  truthful dropped-resource accounting instead of merely accepting Present.
- DX12 frame preflight/recovery, native submission, capture publication, retention
  completion and orphan owners are unchanged from the baseline. Preparation
  errors still return before native-failure routing; local raw upload vectors die
  before submission. Fence/native checks and composition commit still precede
  successful completion and requested capture publication. Aging and weak
  immutable-payload sampling remain in the successful-completion owner. Orphan
  surface/epoch/removal checks and native retirement/recovery paths are preserved.
  Vulkan capture, queue failure and presentation recovery ownership also remains
  intact; the presentation edit only retains the recording result for accounting.

## Review limits

Only this planning report was written. No engineering edits, subagents, builds,
test/architecture/native/GPU execution or root-session access occurred during this
actual-source review. Root's build, CPU/native execution and timeout/binary-hash
evidence are separate acceptance records, not results reproduced here. A native
timeout must cover initialization through destruction and require successful
process exit, not merely a printed success marker. Metal/native macOS and any
broader cross-platform completion remain outside this review's claims.
