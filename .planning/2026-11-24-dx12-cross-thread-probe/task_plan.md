# Idle foreign-thread native-retirement probe

Baseline: `279a2e8c31b459865dac0fb00f4ac05111cf98c2`. Overall DX12 completion
remains ACTIVE; this is a tests-only prerequisite, not production recovery.

- [x] Verify all 3231 command-reuse source paths and content, 17 dependency pins,
  sealed package/reused artifacts; preserve `.vscode/` and old evidence.
- [x] Integrate 12 focused test sources and two additive registrations.
- [x] Compile native probe without running; all 248 architecture tests pass.
- [x] Verify independent owned-process/job outer deadline, then run six serial
  non-GPU supervision cases (original, two targeted mutations, restored).
- [x] WARP debug/GPU-validation: real omit-purge RED44/parent94 for both modes,
  exact restoration GREEN; explicit Intel hardware pumping/parked GREEN.
- [x] Preserve production foreign-thread refusal with the existing regression.
- [x] Independent actual-source review and source-consistent regression closure:
  full Windows946 (native201/architecture248), plus the new WSL structure guard.
  This tests-only slice does not claim a new ASan/SDK/full-Linux matrix; the
  next production slice still requires its complete expanded regression.

Evidence: `D:/Dev/shots/cgpui-dx12-cross-thread-recovery-20261124/root-attempt-1/`.
The external non-GPU fixture uses an independently inspected two-assertion overlay:
only WAIT_TIMEOUT maps to targeted RED205/214; WAIT_FAILED/other wait errors map
to206/217. It does not change the candidate supervisor, timeouts or production.
All six runs must bind this overlay rather than claim the old fixture bytes.

Scope: idle opaque HWNDs, distinct owner threads (pumping/parked), actual idle
RemoveDevice, exact captured rect pixels, native-only teardown from another
thread, reconstruction/destruction on the owner. Capture forces DIRECT; active
BUNDLE retirement, busy/active frames, origin-gate-held coordination, Popup,
composition/alpha, asynchronous pending recovery and universal driver safety are
not proved. No production thread filter, public API or backend default changes.
