# X11 private display reset race — accepted focused fix

2026-11-24, parent baseline ce3b2e8f. Trigger: first missing-image full matrix
Windows864 pass, WSL676 pass/1 fail. x11_queued_event_loop_test second application
creation returned xcb_connect failed for DISPLAY before rendering/event-loop work.
That entire failed matrix remains in D:/Dev/shots/cgpui-dx12-missing-image-20261124/final.

## Diagnosis and actual evidence

External x11-display-diagnosis directory holds original/native binary hash and
all per-attempt logs. Reused the exact compiled x11_queued_event_loop_test with
its existing two connection/event-loop scenarios and 3s watchdog, plus a10s outer
process bound. Each trial owned a private Xvfb; its PID remained alive each time.
Original script:100 attempts,7 connection failures and194 keymap initializations.
Single-variable candidate adding -noreset:100 attempts,0 failures and1 initialization.
Xvfb -help confirms -noreset prevents last-client reset. This identifies server
reset/next-connect competition, not a renderer pixel failure or watchdog issue.

New existing structure guard compiled first and failed exit4 before implementation;
identical guard passed after adding the flag to the actual script. Actual deployed
script normalized CRLF->LF exactly as existing WSL mirror synchronization does:
100 further attempts,0 failures. Then actual dedicated WSL mirror platform suite:
55 passed/0 failed. Normalization hashes and raw logs are retained.

The first actual-script trial accidentally copied Windows CRLF shell bytes directly
to Bash and failed at parsing before starting Xvfb/tests. actual.log is retained;
actual-lf.log is the corrected harness invocation, not an application fix or a
weakened test. No attempt is relabeled passing and no test assertion/timeout changed.

## Scope and review

Only scripts/dev/test-display.sh and its existing architecture owner change.
-noreset keeps one private display generation until existing owned-PID EXIT cleanup;
readiness, native test behavior, session fallback policy and host clipboard isolation
boundaries remain unchanged. No new option, skip or application-side retry.
Independent actual-source Standards0/Spec0 reports are adjacent. Focused accepted;
full Windows/WSL/adapter/ASan/SDK matrix on the new source is still required as part
of missing-image acceptance. Overall DX12 goal remains active.
