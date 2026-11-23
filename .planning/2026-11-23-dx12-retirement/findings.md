# Diagnosis and evidence

The `diagnosing-bugs` workflow was loaded before investigation. The deterministic
feedback seam substitutes private native synchronization operations only; it
uses actual Dx12State destruction, but never submits native GPU work. This
distinguishes a deterministic ownership/protocol regression from a driver repro.

## RED, before product fix

Command (WARP + Debug, MCPP_JOBS=1):

```powershell
scripts/dev/test.ps1 -p tests/renderer-dx12 dx12_sync_
```

`sync-red.log`: 0 passed, 3 failed; execution 0.10/0.12/0.11 seconds.

- retirement exit4: destructor released state after repeated unproven Signal
  failure. The actual destructor returns instead of retaining pending ownership.
- ticket exit3: event failure and wait failure both discard the successfully
  submitted completion ticket and attempt a new Signal.
- wake exit2: WAIT_OBJECT_0 reports idle while completed value remains below
  the required fence ticket.

`structure-red.log`: missing focused retirement module, exit1.
`sync-red-repeat.log` reproduced all three original failures a second time.
`sync-expanded-red.log` reproduced six failing test programs before the fix,
including event/wait teardown, terminal/MAX evidence, reserved ticket exhaustion
and captured wait-error handling. Synthetic pending ownership never submits GPU
work; native device objects are real but idle.
`device-removed-baseline.log`: real native idle-device removal already passes;
this is regression coverage, not the cause of the live-queue synchronization bug.
An earlier link attempt overlapped creation of the adapter source; rerunning
after the complete seam built successfully. That link failure is not counted RED.

## Ranked falsifiable hypotheses (communicated before fixing)

1. Ignored destructor wait error releases GPU owners: a retaining retirement loop
   must keep destruction blocked at the scripted retry until completion is allowed.
2. A lost successful ticket forces a second Signal: persisting it must retire
   the original ticket without another Signal after either event/wait error.
3. Event wake is incorrectly treated as fence evidence: a post-wake completed
   value check must retain ownership or wait for genuine completion.

## Design constraints

Use completed fence values or actual removed-device evidence, never CPU Signal
or elapsed time, as the release boundary. D3D12CreateDevice returns a per-process,
per-adapter singleton; forcing RemoveDevice would affect other renderers too.
Permanent unprovable completion therefore blocks teardown with backoff. This is
an explicit safety tradeoff, not complete availability or automatic recovery.

## Fix and sensitivity checks

The three hypotheses were confirmed by the same regressions turning GREEN.
The successful ticket persists independently of attempted signals, foreground
wakes recheck completion, and a dedicated no-allocation/noexcept retirement
loop polls completion/removal before any state member can be destroyed.
The native adapter captures wait errors immediately; a failed wait with no error
code uses ERROR_GEN_FAILURE, never a success HRESULT. UINT64_MAX is never queued.

`native-green.log`: WARP + Debug, all32 native programs pass, including all6
deterministic fault programs and the real idle-device removal regression.

Independent review requested stronger resource evidence than a blocked
destructor alone. Retirement tests now attach an IUnknown release sentinel to a
real upload resource through SetPrivateDataInterface; only state.uploads owns
that resource. It must remain alive before completion and be released afterward.
`resource-sentinel-mutation.log`: temporarily clearing uploads before waiting
keeps destruction blocked but causes both event/wait lifetime checks to fail,
exit3. The original state source was restored byte-for-byte in finally, and the
final matrix rebuilds it. This rules out the prior test's false-negative pattern.

Native references (read by the independent investigator):
- https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-d3d12createdevice
- https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device5-removedevice
- https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getdeviceremovedreason

## Full-suite integration correction

The first complete Windows run found one registration omission: the new
architecture test was in mcpp discover but not the separate CI header/architecture
matrix. `first-windows-final-test.log` reports692 passed/1 failed; the matrix
guard exited4 on the mismatched source count. Added the single matching row to
`scripts/ci/architecture-header-targets.txt`, independently reviewed it, then
`ci-matrix-green.log` passed. No structure cap or assertion was relaxed.
Sources were re-frozen and the entire final driver restarted afterward.
