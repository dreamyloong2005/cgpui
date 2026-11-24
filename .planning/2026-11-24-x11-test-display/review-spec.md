# Independent Spec review: private Xvfb generation stability

2026-11-24 — `/root/popup_structure`. **0 actionable source findings.**

## Scope and fixed point

Reviewed only `git diff HEAD -- scripts/dev/test-display.sh
tests/architecture/phase_i_x11_data_transfer_structure_test.cpp` and the complete
two owning files, against HEAD
`ce3b2e8f402741c972c7af00f9c09ca8b5784870`.

Spec supplied by root: the frozen missing-image WSL matrix exposed a failure of
the second `xcb_connect` in `x11_queued_event_loop_test`; stabilize the private
test-server lifecycle without altering application behavior, timeouts, skips or
the host display. This review is not a new diagnosis or runtime experiment.

## Source conclusions

- `scripts/dev/test-display.sh:42-44` adds only `-noreset` and its explanatory
  comment to the existing private Xvfb launch. This prevents a last-client reset
  between readiness probes/test clients, while retaining one owned server process
  for the enclosing test invocation. It does not reconnect/retry a failing
  application client or relax any test assertion.
- Display-number search, lock checks, readiness probing,100 attempts and0.05s
  delay, process-liveness checks, EXIT kill/wait/log cleanup and `-nolisten tcp`
  are unchanged. Existing session opt-in and missing/unstartable-Xvfb warning
  fallbacks are unchanged, not new escape paths or claimed private isolation.
  No command changes an existing host X server or its settings.
- `tests/architecture/phase_i_x11_data_transfer_structure_test.cpp:71` requires
  the actual launch argument sequence `-nolisten tcp -noreset > "$log"` alongside
  existing private-display and non-exec wrapper guards. It neither removes a test
  nor turns connection failure into skip/pass. Its existing exit4 reports the
  missing launch invariant. This is a source-structure guard, not independently
  sufficient runtime evidence of reset behavior.

## Evidence boundary and remaining acceptance

Root reports an unchanged binary run100 times with the original script yielded
7 failures /194 XKEYBOARD initializations, while the sole `-noreset` variation
yielded100 passes /1 initialization and the server remained alive. Root also
reports the new structure guard first returned4 then GREEN. These are attributed
reports; this reviewer did not inspect their runtime artifacts or run anything.

Root's actual-script100 repetitions and complete WSL platform55 matrix remain
execution gates, together with the surrounding frozen acceptance work. No
engineering source was edited; only this review report was written. No build,
test, GPU/native execution or root-session inspection was performed.
