# Two-axis review

Fixed point: `c40dd0c4e08beb1fe5d1caf547681d7d0c9e8d6e`.
Review scope: working changes and new files against that baseline, not the
separately archived throwaway prototype. Spec source: this task plan and the
bounded ordinary-alpha decisions captured in the three updated backend docs.
Standards source: AGENTS.md modular-development/build rules plus Fowler heuristics.
Independent reviewers did not own builds; execution evidence is in acceptance.md.

## Standards

No hard violations in the production module layout or documented build rules.
Platform alpha ownership/protocol/native-menu repair and the private renderer
lease are focused leaves; broad owners only delegate. Existing caps remain.
New structure test is registered in mcpp and the dedicated CI target matrix.
The full existing suite caught the aggregate window header at 136 rather than
its 134 cap; two short declarations were normalized to existing single-line
style, with no threshold change, and the new guard now checks 134 explicitly.

One nonblocking possible Duplicated Code heuristic: four native acceptance
scenarios repeat before/open/after RGB+focus comparisons. Shared capture/menu-loop
mechanics already have a common owner; per-scenario assertions and diagnostics
are retained to keep acceptance cases independently legible. This is not a
production-module breach and does not weaken their exact pixel oracle.

## Spec

No remaining actionable findings after correction:

1. Pointer-bearing window messages were unsafe for queued messages. The protocol
   now carries only integer instance/cookie tokens. Regressions dispatch malformed
   PostMessage variants with zero and nonzero WPARAM; no payload is dereferenced.
2. The first opaque-history implementation accidentally prohibited managed opaque
   worker-thread factories. Public regression reproduced exit3; only history
   exchange now uses bounded same-process SendMessageTimeout, with fail-closed
   semantics. Alpha operations remain owner-thread-only. The regression passed.
3. Positive native tests incorrectly assumed every Windows host supports the new
   attribute. Actual pre-26100 systems must first reject alpha and preserve the
   native window, then explicitly skip unsupported positives. Supported hosts
   cannot turn setup errors into skips. The older OS path is not locally executed.

Docs distinguish initial support, unimplemented dynamic upgrades and archived
experiments. No backend/default switch, disabled Vulkan Popup, public mode,
replacement native menu or full-parity claim was introduced.

Summary: Standards 0 hard violations / 1 nonblocking test-code heuristic;
Spec 0 remaining actionable after the compatibility and wire fixes.
