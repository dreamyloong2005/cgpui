# Frame admission milestone review (not full DX12 completion)

Baseline: `3f2df20f`. Reviewed actual dirty source before final validation freeze.
Scope: Win32 timer fail-closed, neutral runtime readiness/lifetime, DX12 private
handle/grant/admission/retirement, frame-owned CPU timing, bounded test acquisition,
caller migration and architecture grouping. No default/backend/Popup policy change.

## Independent review

- Standards: `/root/repo_progress`, with focused timer/helper inspection:
  zero confirmed findings. Renderer85<=100, Win32 application170<=170,
  architecture manifest232<=240. Existing phase H/I/J caps were not relaxed.
- Spec: `/root/staging_spec`: zero confirmed findings, including post-cancel
  lifetime rechecks, synchronous timer delivery, late R1 deadline/WM_QUIT fixes
  and the common pixel capture -> bounded helper -> real begin structure chain.
- Stage B review initially found a no-op64x64 resize test; changed to an actual
  96x96 ResizeBuffers and capture-dimension assertion, then independently closed.
- Caller migration independently reconstructed by `/root/pacing_b_review`:
  62 files,142 success acquisitions;54 classified raw occurrences retained.
  Original Present/errors/pixels/generation/lifetime assertions unchanged.
- Architecture grouping independently reviewed by `/root/frame_wait_review`:
  19 actual DX12 guard moves, owner glob plus exact CI/discovery inventory;
  222 architecture+47 header programs, no original test omitted or duplicated.

These are static reviews, not claims that the matrix ran or that all DX12 work
is complete. GPU/driver/performance/old-OS limits remain explicit.

## Additional local evidence before freeze

- Timing A compiled then failed at exact17ns seed assertion; B same CPU ledger
  test passed. This proves arithmetic/value wiring, not exclusive native-wait
  duration, GPU execution or input latency.
- Native runtime fairness/close tests passed2/2 using synthetic poll results but
  real Win32 queues, renderer/frame/capture and runtime timers. Natural native
  root/child recovery tests separately exercised pending without observer retries.
- Added native-timer sustained-pending backoff control: after350ms, a measured
  approximately200ms quiet interval observed3 polls per window. A temporary
  64->2ms mutation failed the bounded-poll oracle; restored original bytes passed.
  Mutation was test evidence only; source is2/64ms and is frozen below.
- Timing and mutation build requests briefly overlapped; mcpp serialized them
  with its build-directory lock (recorded in logs). The final matrix is one
  serial driver on restored frozen source, not acceptance of mixed intermediate
  build state. No other engineering executor ran.
- Initial full native caller run:84pass/28fail (direct successful-begin sites did
  not yet handle pending). After migration:114pass/0fail on WARP+debug.
- Architecture initial219run:211pass/8fail due old240-line registry limit.
  Actual DX12 owner-directory grouping preserved caps. Next222run:221pass/1fail
  from common capture guard's old direct-call expectation; updated the real
  ownership chain (not its behavioral assertions); repeat222/222 passed.
- Test helper contract/native queue controls2/2 passed, including32-message cap,
  smaller/zero budgets and original WM_QUIT exit code. Only success-acquisition
  test callers use it; production runtime and negative/recovery oracles do not.

## Freeze

Evidence: `D:/Dev/shots/cgpui-dx12-frame-pacing-20261124`.
`source-snapshot.json` / `.tar`:2940 source paths, excluding user `.vscode` and
planning records. Initial Windows raw-source comparison matches. Full matrix
currently running; final acceptance is not asserted by this review record.
