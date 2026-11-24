# Independent Spec review — native-reuse stdout flush

2026-11-24. Reviewer: `/root/popup_structure`.

**0 actionable source-level findings in this two-line, test-only delta.** This is
not a rebuild/test/native-runtime certification or a conclusion that the complete
matrix audit now passes. Root owns the focused execution and subsequent evidence
audit; this reviewer did not run them.

## Exact scope and source identity

`tests/renderer/vulkan_missing_image_native_reuse_test.cpp:55-56` adds only:

```cpp
// The Windows ASan entry terminates the process without ordinary CRT exit.
if (std::fflush(stdout) != 0 || std::ferror(stdout)) return 27;
```

Current file SHA-256:
`7ae41e41d2ad1f8e98266967e53fa470f3b352b015862dda3076cb62c4a38f7b`.
Removing exactly these two lines reconstructs the frozen `verified/` source
bytes, SHA-256:
`2cf49cbe9fbb0c908f77415777e43f46c6dc70a34d030d316051412c50afc4af`.
The test is currently untracked, so an empty ordinary `git diff` is not treated
as proof of no change. Comparison used the actual frozen source inventory and
the separately reviewed candidate; the latter matches after CRLF/LF normalization
once only this two-line insertion is removed.

Independently hashed570 frozen paths covering `src/renderer/`,
`include/cgpui/renderer/`, all nine `vulkan_missing_image_native*` test/support
files, the renderer member manifest, test driver and Windows ASan entry. Only
this test's main file differs. This is a scoped preservation check, not a claim
that every unrelated workspace path was audited.

## Preserved Spec and native predicates

- The flush remains after all six phase calls and after the final fresh native
  capture, native submission/order/diagnostic checks and pixel assertions.
  Existing early failure returns are unchanged. It cannot turn a failed native
  observation into a successful phase or skip the final capture.
- The six states remain both-missing, first-uploaded, both-ready,
  second-invalidated, both-invalidated and second-resupplied, with unchanged
  upload/invalidation/availability masks and clear colors.
- The phase loop remains bounded by24 attempts, requires at least one fresh
  observation and at least two successful native reuse observations per phase,
  and does not retry a failed Present. The reliable buffer/epoch, recorded phase,
  successful submission count, exact forwarded draw order/arguments, ready/missing
  diagnostics, historical-capture preservation and device-generation predicates
  are unchanged. Historical capture is still not current uncaptured pixel proof.
- Native callbacks still forward the actual Vulkan functions and results.
  Reset/free bookkeeping and successful-submit ownership are unchanged. Trace
  RAII still restores the seven original function pointers before renderer
  destruction on ordinary scope unwinding. Neither private resource retirement
  nor production renderer cleanup/recovery/capture ownership changed.
- The final readback still forces fresh recording, checks the phase7 sequence,
  rejects seed-equivalent stale pixels and verifies the expected final colors.
  The success marker is unchanged; the new operation only publishes pending
  stdout and makes publication failure an explicit nonzero result.
- `fflush(stdout)` is checked for failure; `ferror(stdout)` also catches an earlier
  sticky stream error even if the final flush returns success. There is no
  `clearerr`, retry, ignored failure or native-success override. Return27 does
  not collide with the existing main/phase return1..26 failures.

The existing success marker precedes normal local teardown; this delta does not
move it or reinterpret it as independent evidence of completed renderer/native
retirement. Likewise, flushing a C stream is not durable-file persistence or
proof that a parent consumed its output. An explicit flush can block on output
backpressure, so the existing outer process timeout remains necessary; no timeout
or bounded-wait contract is changed or replaced here.

## Inspected original failure evidence

Read, but did not recreate, files under
`D:/Dev/shots/cgpui-dx12-missing-image-20261124/verified/`:

- `asan-native-reuse-direct-original.json` records exit0, stdout4148 bytes,
  stderr0 bytes and success-marker count0 for original binary SHA-256
  `69dce26e444ff06a349000958d9506b40f9c7953bb5d66b60e7da2ecbf7c5059`.
- Its raw stdout is4148 bytes and ends in the both-invalidated phase, mid-line
  after `direct=0 missing`; raw stderr is empty. The complete final success
  marker is absent. This preserved direct-process capture supports root's
  exclusion of mcpp log formatting as the sole cause; exit0 alone does not
  provide the missing phase/final-capture log evidence.
- `audit-original-failure.log` retains the assertion rejecting
  `asan-renderer-test.log` because the success-marker count was not exactly one.
  That audit failure remains valid for the original logs and is not waived by
  this source change.

`scripts/dev/asan_entry_windows.cpp:36-43` calls the test main, then `_cexit()`,
then `TerminateProcess`. The source comment about nonordinary process exit is
consistent with that entry path, but `_cexit()` is present: this review does not
claim that all CRT cleanup was absent or independently diagnose why that
particular runtime combination left stdout buffered. The narrow explicit flush
is appropriate evidence-publication handling without needing that broader claim.

Preserved original-evidence SHA-256 values:

| File | SHA-256 |
| --- | --- |
| `asan-native-reuse-direct-original.json` | `2eb3bb2ad810e75b175748d4e9d964dfc9b7c5cd8f6105a74c7e9322ac480e77` |
| `asan-native-reuse-direct-original.stdout.log` | `b0333fd86dca26e10f6906329c81f18ac78b305de525211cfa5bf14ce96dbe8b` |
| `asan-native-reuse-direct-original.stderr.log` | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| `audit-original-failure.log` | `b95130085f37ac81a3dcc77764a08fcdf858cb61fc333061da11f259154667aa` |

## Remaining acceptance boundary

Root must associate the rebuilt binary with this amended test source, execute the
focused ASan case under its existing process bound, retain complete stdout/stderr
and exit code, and rerun the matrix evidence audit without overwriting the
original failed evidence. A source-level0 finding report is not a new runtime
pass, does not retroactively alter the frozen matrix fingerprint, and does not
certify unrelated backends/platforms, native retirement or performance.

Only this requested planning review was written. No build, test, candidate-script
execution, GPU/native/UI action, engineering-source edit or agent spawn occurred.
