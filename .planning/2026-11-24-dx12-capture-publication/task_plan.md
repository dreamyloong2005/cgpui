# Successful capture publication boundary

2026-11-24. Baseline `a51c09a8` (composition admission accepted). Overall DX12
completion goal remains active. Root only builds/tests/GPU; no push.

Source audit found readback assigns state.pixels before fallible debug and DComp
Commit. The public header does not specify success-only capture history; this
slice explicitly tightens the DX12 failed-Present safety boundary without changing
Vulkan history or old borrowed-pointer invalidation. Existing diagnostic success
publication remains unchanged. No public API or native injection switch.

Owner: return completed CPU pixels from focused dx12_capture.cpp, hold a local
candidate through native validation/Commit, publish only at successful completion.
Preserve frame's old-pixels reset, pending/retry semantics, native synchronization
and pixel layout. Adapt the experimental composition probe's one direct reader.
Add a structure guard without relaxing existing entry/header caps.

RED/GREEN: actual Renderer/debug device, valid frame and requested readback,
controlled native InfoQueue error added only after begin. Raw Present must return
that exact error marker with live device/generation and preserved successful
diagnostics, but no failed-frame pixels. Both cold and warm cases run, even when
the first fails. Unavailable setup is a failure, not a semantic RED or a skip.
This is late debug-check reproduction, not real Commit-failure or TDR proof.

Evidence: `D:/Dev/shots/cgpui-dx12-capture-publication-20261124`.
Candidate source/design came from the external public-operation audit. RED/GREEN
and final frozen acceptance are observed and recorded in `acceptance.md`.

## Observed progress

- Real WARP baseline RED after successful build: cold and warm both exit18,
  exact injected late error, live device, preserved diagnostics, exposed pixels.
- Applied owned readback/local candidate/successful-end publication. Allocation
  now precedes Map; final optional move is compile-time nonthrowing. Getter and
  previous-pixel invalidation remain unchanged. Direct probe caller adapted.
- Review hardened queue scan to permit only the one exact injected native error,
  before clearing messages. WARP GREEN cold/warm both0; no other error hidden.
- Added focused structure registration and precise DX12-only doc boundary;
  independent spec/native-safety and Standards source reviews0.
- Expanded structure226/226 and native123/123 WARP+debug passed. The explicit
  composition bin was rebuilt; Intel/NVIDIA/WARP x hidden/visible all exit0,
  including the changed direct readback caller. This is capability evidence,
  not universal OS/driver or production recovery certification.
- Frozen2971path matrix completed exit0 in evidence final/, with production
  graph refresh before SDK closure: Windows829/WSL668, native123 per adapter,
  common94 per selection, ASan123/common94+94 and installed SDK all passed.
  Exact inventory and raw/normalized source checks pass. Slice accepted, not
  whole-backend completion; see acceptance for fixture and measurement limits.
