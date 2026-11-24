# Actual-source Spec / native-safety review

2026-11-24. Reviewer `/root/popup_structure`, not the missing-image author.
**0 actionable source findings.** Runtime acceptance remains separate.

## Scope

Reviewed `git diff ce3b2e8f` and seventeen untracked implementation/doc/test leaves
against this task plan, the direct human decision in
`../2026-11-24-dx12-completion/public-contract-decision.md`, and external design.
HEAD is `ce3b2e8f402741c972c7af00f9c09ca8b5784870`. All forty actual candidate
paths exactly match the independently reviewed external final Stage B after
CRLF/LF normalization. Native-reuse supplement is not yet included in this review.

## Contract and preservation evidence

- Shared raw validation precedes fallback/clamping/absence, checks stored finite
  geometry/clip/composition/tint and sampling, and verifies derived vertices/clip.
  Adapter extent limits precede absence. Upload layout is validated on actual
  final buffers, never confused with image identity or pixel readiness.
- DX12 typed lookup returns empty only for valid absent identity; installed
  mismatch remains an error. A CPU texture lacking native realization follows
  normal lazy upload and error propagation. Same-ID conflicting draw requests
  reject, including missing-only frames. Explicit compatible uploads/invalidation
  retain the final-ID prefix contract without promising arbitrary rollback.
- Missing payloads preserve original command metadata/order, plan visible work,
  and perform no native call or submitted hook. Unresolved pending markers still
  reject. Raw copies are released on preparation exit. Existing preflight and
  CPU-input/native-failure routing remain distinct.
- Vulkan owns current selected image indices in both successful fresh-record and
  reuse results; it does not consult stale stored signatures or raw request count.
  Reuse equality includes image commands/clip stacks/order/extent. Selection uses
  the recorder's cursor and visibility rule. Image/cache validation precedes
  mutation; native recording/submission failure routing is unchanged.
- Baseline diffs confirm unchanged DX12 capture/publication, immutable retention,
  success-only completion/aging, texture/native retirement, orphan preflight and
  recovery owners. Cache/eviction migrations retain 120/121 and resupply evidence.
  Existing capture and removed/late-removed orphan tests are unchanged.

## Acceptance boundary

Root reports Stage A Vulkan correct pixels then semantic exit41, and DX12 valid
missing rejection exit40. This reviewer did not inspect those runtime logs or
execute build/test/GPU work. B GREEN/full matrix are root-owned pending gates.
Constructed CPU reuse results do not establish native reuse; the separate revised
native observer requires actual-source review and execution after integration.
Metal remains an external, uncompiled/native-unverified follow-up. No full
cross-platform parity or runtime success is claimed. Unrelated `.vscode` untouched.

## Addendum: Win32 fixes and revised native-reuse supplement

2026-11-24. **0 actionable source findings in this addendum.** The initial
forty-path equality and exclusion of native reuse above describe the earlier
snapshot, not the current tree. Current normalized comparison against external
`stage-b/` is 37/40 identical; the only differences are the two merged registries
and `renderer_missing_image_validation_test.cpp`. Its complete delta is exactly
the parenthesized `(std::numeric_limits<T>::max)()` calls at lines46 and60,
avoiding the Win32 macro without changing values, assertions or production code.

Independently read all thirteen supplement paths and compared their actual bytes
after CRLF/LF normalization with
`D:\Dev\shots\cgpui-dx12-missing-image-20261124\native-reuse\revised\candidate`:
**13/13 identical**. The supplement contains only test/support/registration
changes; its three registry merges preserve the original missing-image entries.

- The original P2 oracle gap is closed: trace callbacks append actual forwarded
  direct/indexed arguments into the recording epoch's four-event sequence
  (`vulkan_missing_image_native_trace_test_support.cpp:46-62,91`). Matching checks
  every field and exact order, including direct firstVertex0 versus6, not only
  count or phase expectations (`vulkan_missing_image_native_draw_sequence_test_support.cpp:8-23`).
  The production recorder uses original image index times six and forwards that
  firstVertex (`src/renderer/vulkan/vulkan_image_draw_recording.cpp:66-78,148-149`).
  Every phase checks this oracle before counting reuse; final capture checks it
  too. The CPU control explicitly includes the original firstVertex0-to6 witness,
  all four masks, parameter mutations, reorder, duplicate, extra/missing and overflow.
- Typed PFN/VKAPI callbacks forward real functions and preserve real VkResult.
  Only successful QueueSubmit contributes evidence; no fake success or fallback
  pass exists. Present/begin errors fail immediately. The independent invalid-
  input controls remain intact, apart from the two macro fixes.
- Successful Begin starts a new epoch/sequence; reset invalidates completion,
  submissions and sequence; free clears its record. Reuse requires the same
  recorded buffer/epoch with prior submission and no current Begin. Fixed64
  records/four events avoid callback allocation; overflow cannot pass. RAII
  restores all seven pointers on ordinary/early-return unwinding before renderer
  destruction. This is explicitly a serial test-process observer.
- Six uncaptured phases each require fresh plus two native reuse observations.
  Historical seed equality is not presented as current pixel proof; only final
  requested capture verifies final pixels. The24-attempt bound is not a deadline.

No build, test, GPU execution, or root-session inspection was performed by this
reviewer. Root reports successful compilation and CPU/native runs with external
30s/120s process timeouts; those are attributed reports, not independently
verified runtime evidence. Native teardown/waits require that outer timeout.
