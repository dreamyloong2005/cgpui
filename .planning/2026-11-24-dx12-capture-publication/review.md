# Capture publication review

Baseline `a51c09a8`; actual tracked and new source, including task plan and tests.
Root alone runs engineering builds/tests/GPU. Reviewers were read-only.

## Standards

`/root/capture_standards_review`: 0 actionable findings. Focused capture and
presentation ownership, private owned-value boundary, no public implementation,
registered native and structure regressions, existing entry caps retained.
The experimental direct caller follows the same publication boundary.

## Spec and native safety

`/root/capture_boundary_review`: 0 remaining actionable findings. The public
header never promised success-only capture history; docs deliberately describe
the strengthened DX12 behavior, not a fabricated neutral historical contract.
No Vulkan semantic changes or getter-based masking. Allocation precedes Map;
all native checks and diagnostic preparation precede nonthrowing publication.

The earlier test review found that immediate ClearStoredMessages could hide
unrelated native errors. The regression now scans every ERROR/CORRUPTION and
requires exactly one matching application ERROR before clearing. Cold/warm
cases run independently and preserve successful diagnostics/device generation.

## Root execution before full freeze

WARP RED: cold/warm both result18, late injected error with exposed candidate
pixels. GREEN: both result0, exact error and no failed-frame pixels. This is
controlled late debug rejection, not evidence of real Commit failure or TDR.

Architecture226 and native123 passed. Explicit composition probe rebuilt and
all six Intel/NVIDIA/WARP hidden/visible rows exit0. Final frozen complete
matrix is separate and must finish before acceptance/commit.

## Final execution

Frozen2971path driver completed exit0: Windows829/WSL668, all builds,
native123 on Intel/NVIDIA/WARP, common94 per selected backend, ASan123 and94+94,
driver/SDK fixtures and actual installed /MT consumer. Exact native/shared
inventories and raw/normalized source matching pass. See acceptance for the
auxiliary inventory-parser correction and all remaining scope limitations.
