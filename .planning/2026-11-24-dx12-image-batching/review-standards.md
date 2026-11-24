# Independent actual-source Standards review — image batching

2026-11-24. Reviewer: `/root/capture_standards_review/orphan_test_review` (not the implementation author).

**Standards: 0 confirmed actionable findings.** This is an actual-source review of the image-batching slice, not runtime/matrix acceptance or completion of the separately approved missing-image skip policy.

## Reviewed identity and source equivalence

- Fixed baseline and observed HEAD: `0f6b2c6eb4e24b5437ad559d1923ae697ef3d49a` (capture-history slice).
- Read `git diff 0f6b2c6e`, all 13 newly added image-slice source/test files, the focused owners and test migrations, and the owning completion/capture boundaries. The image slice contains 30 files: 17 tracked modifications and 13 additions; planning artifacts and unrelated `.vscode` contents are excluded.
- Independently compared every actual slice file with the previously reviewed `D:/Dev/shots/cgpui-dx12-public-contract-20261124/images/candidates/rebased-capture/stage-b/`: **30/30 text matches and 30/30 SHA-256 matches after CRLF-to-LF normalization**. Raw file hashes differ because the actual working files use CRLF and the candidate uses LF; this review does not claim byte-for-byte raw-hash equality.
- All 17 supplied baseline snapshots also exactly match `git show 0f6b2c6e:<path>` after newline normalization. Therefore the actual baseline-to-source content change is the previously reviewed A+B candidate, with no unexpected integration delta.
- Reviewed combined candidate patch SHA-256: `a0027820a14c66cecf0c156a9d0977173f76c77bba15c99f3665b66c079f2305`.
- Actual normalized source fingerprint: `8915cf3aedc78556e97319ad6355f1b9dcf057139b5c237420817e9fca5cb5f9`. Method: sort the 30 Stage-B paths; hash each UTF-8/LF-normalized actual file; hash the UTF-8 sequence `repository-relative/path<TAB>lowercase-sha256<LF>` in that order. This is a source fingerprint, not a test result.

## Focused ownership and standards

Applied `AGENTS.md` modular-first/mcpp rules and the Standards review smell baseline as judgment heuristics.

- `src/renderer/dx12/dx12_image_batch_internal.hpp` owns copied pending upload/invalidation values; `dx12_image_batch.cpp` performs copy/deduplication without mutating the installed cache or validating superseded intermediate requests.
- `dx12_image_prepare.cpp` owns the Present-time invalidation-prefix/final-upload/ordered-resolution sequence. Swapping requests into local vectors releases pending raw bytes on preparation returns/exceptions. It does not take native allocation, image-aging or successful-census ownership.
- `dx12_image_request_internal.hpp` and `dx12_image_resolve_internal.hpp` remain private declaration/data leaves; `dx12_image_resolve.cpp` owns immutable-cache lookup and validated device-scaled geometry. Existing commands preserve their kind/index/slot order.
- Frame and presentation receive only narrow routing/validation calls. New headers are thin; no implementation is piled into renderer/application/public aggregates. Public changes are contract comments, not a native API or developer mode.
- Native realization, weak retention tracking, successful completion, GPU retirement and recovery remain separate unchanged owners. Missing pending-image resolution is rejected before diagnostic planning and again at dispatch, not silently counted as a draw.
- Structure coverage accompanies behavior coverage. Existing caps are not enlarged: actual frame96/100 and presentation97/110; renderer is unchanged. New private leaves stay within their focused budgets. The capture structure test adds seven checks without removing prior assertions.
- Production source glob, new native/shared test discovery, architecture glob and CI inventory all cover the added files. No xmake/build-system change or weakened registration is introduced.

No actionable duplication, speculative abstraction, ownership confusion or other Standards smell was identified.

## Existing guard preservation and deliberate migrations

| Actual test/owner | Review result |
| --- | --- |
| `tests/renderer-dx12/dx12_image_cache_test.cpp` | Earlier/later same-ID draws now expect the final upload, while caller-copy and interleaved primitive-order pixels remain. Separate presented invalidation still rejects the next missing draw under this slice's existing policy. A successful seed makes abandoned replacement preserve known cached bytes; malformed/missing-input cases remain. |
| `dx12_retained_image_history_test.cpp` | Abandoned and early-rejected raw requests no longer enter installed-image census. A new successful seed establishes genuine prior history before unchanged-history/resize assertions. The next successful census remains exact rather than merely nonzero. No arbitrary late-failure rollback guarantee is invented. |
| `dx12_retained_image_snapshots_test.cpp` | Obsolete same-frame per-draw replacement is replaced by two held finished frames with the same ID across actual red/green presentations. Exact retained counts progress4 ->8, remain8 after presented invalidation and next-frame cleanup, then4 ->0 as final owners drop. This preserves immutable lifetime coverage instead of deleting it. |
| `dx12_texture_upload_snapshots_test.cpp` | Two distinct IDs preserve two independent native staging payloads under the new same-ID policy. Caller mutation, exact image tiles, repeated glyph ink, dedupe, cold/warm bytes and staging-page assertions remain. Same-ID historical lifetime is tested separately above. |
| `dx12_image_retention_cache_test.cpp` | Adds direct old255/new17 immutable CPU-byte assertions without removing existing ownership/census cases. |
| Capture publication | Actual `dx12_frame.cpp` still has no pre-Present history reset. Preparation follows preflight and returns input errors before native submission. Actual presentation still publishes only an engaged capture candidate after successful completion; resize/recovery reset boundaries, borrowed-storage limits and existing history tests are retained. |
| New shared/native tests | Three shared semantic oracles keep explicit actual-backend selection, raw Present outcomes and fresh-background captures; no source-side relaxed expected result. Native tests cover final-value validation, CPU-only queue ownership/order/abandonment and a genuine acquired orphan frame with correctly scoped HWND/API owners. |

## Explicit scope remainder

The user's newly approved **missing-image skip** behavior is the next slice. This reviewed image batch still rejects missing/incompatible cache resolution (`dx12_image_resolve.cpp:11-12`), and its current negative tests/docs intentionally describe that baseline. It is **not an accidental regression or finding in this frozen slice**, but full requested image-policy parity is not yet complete. Implement the skip policy with focused behavior/structure tests in its own subsequent change; do not claim this review accepts that future behavior or silently weaken malformed-input tests to obtain it.

## Verification limits

No engineering edits, builds, tests, GPU/fault execution, root-session inspection, subagents, or `.vscode` content inspection occurred. The only repository write is this planning report. Root owns the actual A RED/B GREEN, compiler results and frozen hardware/WARP/debug/GPU-validation/ASan/Windows/WSL/SDK matrix. Root's runtime progress is not reproduced or certified by this source review.

**Summary: actual-source Standards0; worst issue none; runtime acceptance and missing-image skip remain separately owned work.**
