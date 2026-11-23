# Independent Standards review

Date: 2026-11-24. Baseline: `32096285f3d5ae33b4e512de61299350f7f2902b`.
Scope: actual working-tree diff plus the new fallback tests/support/guard,
excluding user `.vscode/`. Compared all 12 A payload files and the one B owner
against the independently reviewed external candidates. All 13 match after
CRLF-only normalization; raw hashes differ because of line endings, so no
byte-identical raw-source claim is made.

## Result

**0 confirmed actionable findings.** No documented-standard violation or
actionable Fowler-baseline smell was identified in this bounded change.

- `AGENTS.md` module-first rules are preserved: the production edit remains in
  `dx12_backdrop_blur_frame.cpp`; native fixture behavior and diagnostics have
  separate focused implementation leaves with a declaration-only helper header.
  The shared base fallback remains the sole field-forwarding owner.
- The original logical draw is delegated after the existing device-scaled raw
  validation, avoiding double DPI conversion. Prior-error/released guards,
  supported-plan validation and native submit/recovery errors retain their
  existing owners; no public API, factory mode or broad entry-file logic is added.
- Explicit Windows support/test registrations preserve the existing test-only
  library and its dependencies. The new guard is registered in mcpp and the CI
  inventory without removing the common/runtime guards or loosening old caps.
- The native fixture retains real state/frame/GPU capture. Frame/state lifetime
  ends before HWND destruction; only the private support decision is injected.
  Recovery checks retain typed failure and original successful diagnostics.
- Updated `docs/dx12-backend.md` describes tint/no-tint, malformed-input and
  rounded-work accounting precisely. It separates injected capability evidence
  from incapable hardware and acknowledges the existing capability-query error
  ambiguity. No default switch, arbitrary-driver or full-completion claim is added.

## Evidence limits

This reviewer performed static inspection and source comparison only, with no
build/test/GPU execution and no production source edits. Root reported native
A RED (present unsupported, exit 3), B five-test GREEN and both structure checks
passing. Those reports do not replace the pending final frozen-source matrix;
root owns final execution, evidence consistency and acceptance.
