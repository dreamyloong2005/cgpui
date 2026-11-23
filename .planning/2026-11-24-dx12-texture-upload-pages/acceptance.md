# Acceptance: bounded DX12 texture staging reuse

Date: 2026-11-24. Baseline `241b226a`, branch `phase-v`.
Status: **PASS for this bounded staging-allocation milestone**. Whole-DX12 completion remains active separately.
Evidence: `D:/Dev/shots/cgpui-dx12-texture-upload-pages-20261124/`.

## Delivered scope

A single private `Dx12UploadPages` implementation owns persistently mapped native
UPLOAD pages. Geometry and texture staging have separate pool instances, with
focused adapters preserving geometry's 16-byte/UINT view contract and texture's
512-byte placement, native row pitch and last-row source-span contract.

Both pools use 256 KiB standard pages, retaining at most four each after the
next safe reset (combined retained capacity at most 2 MiB). Oversized allocations
are rounded to 64 KiB and retired, with surplus standard pages, at the next safe
reset. This is not a peak-frame or whole-renderer memory cap. No overwrite or
release occurs on failed/unknown GPU completion. Native retirement clears both
pools before releasing the old device; same-device resize may retain them.

GPU copy/barrier recording remains in texture_resources. Destination textures,
SRV descriptors, immutable CPU snapshots, cache eviction and upload commit rules
are unchanged. Recorded-but-unexecuted upload lists do not mark a texture uploaded;
completed uploads commit before checking later Present/readback/composition errors.
The obsolete state.uploads vector is removed, not retained as a parallel path.
Public diagnostics count tight payload only, excluding pitch/placement/page bytes.

Readback, descriptor pooling, per-draw membership scans, glyph partial updates,
command reuse and asynchronous frame overlap are outside this bounded milestone.
Window/menu/Popup semantics and default/auto Vulkan remain unchanged. A stale
public factory comment now correctly describes bounded initial ordinary alpha;
there is no public API change. This is not whole-DX12 completion or an FPS claim.

## Real red-to-green

The native budget test draws 128 independent 3x2 RGBA images with padded input
rows and no unnecessary trailing padding on the last row. It checks all 768
texels before checking resources, and verifies 3072 payload bytes and 128 draws.
Before implementation it failed with **128 staging resources** (exit 5), recorded
in texture-budget-red.log. That is an observed resource count, not an estimate.

| Frame | Staging pages | New native pages | Texture payload bytes |
| --- | --- | --- | --- |
| Cold 128 images | 1 | 1 | 3072 |
| Cached images | 1 | 0 | 0 |
| Replaced immutable images | 1 | 0 | 3072 |

All 768 texels are checked on each frame. Geometry uses its own independent pool.
The GPU multipage test draws eight odd-width 33x257 images and an optional 513x513
image. All 3072 checked covered/background pixels per frame match. Three passes
report pages 4/3/4, capacity 2031616/786432/2031616 bytes, new pages 4/0/1, and tight
payload 1324068/271392/1324068 bytes. Removing the oversized image lets its
transient page retire; later adding a replacement creates only that large page.

## Targeted checks

- texture-targeted-green.log: **9 passed**, WARP/debug. Real GPU budget,
  multipage/oversize, immutable image replacements and mixed glyphs; real-device
  layout/bytes/alignment/last-row checks; invalid inputs and allocator overflow;
  bounded retention with native resource identity and release sentinels;
  recorded-but-never-Execute regression; completion-failure retention; resize
  and actual idle-device RemoveDevice recovery with reupload/cache diagnostics.
- native-targeted-build.log / native-targeted-green.log: **95 built; 95 passed**.
  Includes prior geometry allocation/order/retention/failure/recovery tests and
  legacy synchronization tests migrated to actual texture-pool pages.
- architecture-build.log / architecture-test.log: **210 built; 210 passed**.
  New guard is registered in mcpp and the CI header matrix; geometry's existing
  guard now protects delegation to the shared owner; shadow checks are unchanged.
- Independent Standards: 0 hard or actionable smell findings. Independent Spec:
  0 findings. See review.md; reviewers did not claim to run tests.

## Frozen-source matrix

Snapshot: **2825 source paths**, excludes planning records and user `.vscode/`.
No production/test/doc edits after the snapshot. Root alone controls build/test;
no-run MCPP_JOBS=4, native execution MCPP_JOBS=1. Dedicated WSL validation mirror:
`/root/cgpui-logic-cleanup-20261123`; no user Linux/design checkout is modified.

Completed frozen-source checks:

| Validation | Actual result | Evidence |
| --- | --- | --- |
| Windows full no-run/execution | 770 built; **770 passed, 0 failed** | windows-final-build.log, windows-final-test.log |
| Windows all-member build | PASS | windows-all-build.log |
| WSL full execution/all-member build | **645 passed, 0 failed**; build PASS | wsl-full-test.log, wsl-all-build.log |
| Native Intel adapter 0 + debug | **95 passed, 0 failed** | native-0.log |
| Native NVIDIA adapter 1 + debug | **95 passed, 0 failed** | native-1.log |
| Native WARP + debug + GPU validation | **95 passed, 0 failed** | native-warp.log |
| ASan native DX12 (WARP/debug) | **95 passed, 0 failed** | asan-build.log, asan-test.log |
| ASan renderer | **89 passed, 0 failed** | asan-renderer-build.log, asan-renderer-test.log |
| SDK graph/README fixtures | **9 passed** | sdk-graph-fixtures.log |
| SDK header fixtures | **15 passed** | sdk-headers-fixtures.log |
| Installed-only Windows consumer | compile/link/run PASS, actual /MT | installed-sdk-windows.log |
| Windows raw frozen-source check | all 2825 paths match; no new/removed/changed paths | source-match.json |
| Dedicated WSL source check | 2825 paths match; CRLF/LF normalized | wsl-source-match.json |

Counts describe separate runs and are not additive totals. The Windows count
includes architecture 210, renderer 89 and DX12 95. Legacy loader tests intentionally
launch their own backend/adapter children. The final matrix needed no retries and no post-freeze source changes. The only
intentional failure was the pre-implementation resource-budget RED. Existing
intermittent Win32 Popup/X11 tests passed; they were not modified or claimed fixed.

## Installed consumer

The external consumer compiles copied public headers and same-volume hardlinked
production libraries using the actual /MT CRT, without Vulkan SDK paths/imports.
It renders cold, cached and replaced sets of 128 padded 3x2 images, verifying
all **768 source texels per frame** and tight payload counts **3072/0/3072**.
Its existing geometry (128 sampled cell centers x 3 frames), text/shadow/blur,
Popup and initial client-alpha captures also pass. Both the clean run and owned
invalid app-local Vulkan-DLL run report the same successful checks; DX12 does not
load Vulkan, and the actual executable has no Vulkan PE import.

This consumer checks public pixels/accounting, not the private page count; that
is proven by the native budget tests. Geometry log `all_pixels=matched` means
all 128 sampled cell centers, not every framebuffer pixel. The texture log names
its 768x3 sampled texels explicitly. SDK stage metadata records mutable hardlinks
and actual CRT, not a published immutable release. CMake passed configure-only
with Vulkan discovery disabled; no CMake C++ binary or release CRT was validated.
The unused CMAKE_DISABLE_FIND_PACKAGE_Vulkan warning is expected because the
new generated config contains no Vulkan discovery. No additional old SDK cleanup
was required. Final free space was about C: 2.59 GiB / D: 5.67 GiB (time-specific).

## Verification limits

Synthetic signal/wait failure has no native GPU work and tests lifetime only;
real RemoveDevice tests are explicit idle-device removals, not arbitrary TDR.
Native Create/Map failure cleanup is source-reviewed, not injected driver OOM.
Allocation counts do not prove FPS/latency improvement. WSL tests do not certify
native macOS or Vulkan conformance; known dzn/runtime-facts warnings remain.
Installed SDK staging uses copied headers and same-volume hardlinked libraries,
not an immutable release package. CMake validation is configure-only, not a
CMake binary build. Source hashes establish consistency, not binary provenance.
