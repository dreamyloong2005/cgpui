# Acceptance: bounded DX12 geometry upload reuse

Date: 2026-11-24. Baseline `27605927`, branch `phase-v`.
Status: **PASS for this bounded geometry resource-allocation milestone**.
Evidence: `D:/Dev/shots/cgpui-dx12-upload-arena-20261124/`.

## Delivered scope

A focused private owner allocates persistently mapped 256-KiB upload pages and
hands the existing indexed, textured and backdrop-composite recorders borrowed
16-byte-aligned vertex/index slices. Original drawing order, ranges, scissor,
effects and frame diagnostic semantics are unchanged. The old per-draw committed
geometry buffer helper was removed, not left as an alternative path.

Before a page cursor is rewound, presentation must have proved completion through
the existing idle/fence path. No reset occurs at public begin-frame or on a
failed wait. A narrow state adapter rejects known pending/failed/removed/recovery
states before reset/upload. On the next safe frame reset it keeps at most four
standard pages (**1 MiB retained**); surplus standard pages and oversize temporary
pages are released. Oversize allocations are rounded to 64 KiB and remain alive
for the whole frame. This is a retained-memory limit, not a bound on peak frame
memory or on all renderer resources.

Native retirement clears mapped pages before releasing the old device, after
the existing completion/removal proof. Same-device resize may retain pages;
confirmed device removal releases old storage and next use allocates fresh pages.
The owner counters count successful native page allocations, not draw slices.

Texture footprint staging, readback, glyph/image immutable snapshots, descriptor
allocation, draw batching and asynchronous frame overlap are unchanged. Geometry
page capacity/bytes are not added to public texture-upload diagnostics. Default
Vulkan/auto, explicit DX12 failure behavior and native-first Popup policy stay
unchanged. No public window mode or API was added. The independently rechecked
ordinary HMENU and dynamic-opacity limitations are not relaxed by this change.

## Deterministic red-to-green

The native `dx12_geometry_upload_budget_test` renders 128 rectangles through the
normal frame path, captures real pixels and checks exact public diagnostics.
Before implementation it failed with **256 upload resources** (exit 5), recorded
in `geometry-budget-red.log`. The final test counts both retained arena pages
and any old staging resources, so removing the old vector alone cannot pass.

`geometry-budget-green.log` and `geometry-targeted-green.log` record:

| Frame | Geometry resources | New native pages | Capacity |
| --- | --- | --- | --- |
| Cold 128-rectangle frame | 1 | 1 | 262,144 bytes |
| Next reversed-color frame | 1 | 0 | 262,144 bytes |
| Next reversed-color frame | 1 | 0 | 262,144 bytes |

Every one of the 128 grid-cell centers is checked, not merely one pixel. Texture
upload bytes remain zero and command/draw counts remain 128. The logged CPU
present durations vary substantially; they are observations, **not an FPS or
latency improvement claim**, and are not used as a pass threshold.

The actual GPU multipage test records 8,192 geometry ranges plus ordered overlay
rectangles: two pages/1,179,648 bytes; a subsequent small frame retains one
256-KiB page with **zero** allocation; another large frame creates only its new
transient page. Pixel checks and public diagnostics pass at every step.

## Targeted evidence

- `geometry-targeted-green.log`: **7 passed**, WARP/debug, serial execution.
  Covers real resource budget/pixels, actual GPU oversize/index offsets,
  copied bytes/nonalias/alignment, page crossing, invalid/null/overflow rejection,
  mixed small/oversize ordering, actual retained-resource identity, retention cap,
  native page sentinel release, failed completion retention and device recovery.
- `geometry-structure.log`: **3 passed** (new module guard, existing shadow
  structure guard and CI architecture-header matrix). Both mcpp and the matrix
  explicitly register the new structural check.
- The first expanded no-run caught a test variable named `small` colliding with
  the Windows macro. It was renamed locally to `small_bytes`; no global macro
  undefinition or production workaround was added. See `geometry-targeted-build.log`
  and `geometry-targeted-build-green.log`.

The failed-fence test attaches a native resource private-data sentinel to an
actual arena page, rather than the old upload-vector surrogate. Its scripted
sync failure has no real GPU work and is explicitly a lifetime-boundary test.
It proves no early reset/release and eventual release after completion proof,
not arbitrary live-driver fault recovery. Separate real `RemoveDevice` and
resize tests verify actual native captures, page retirement, fresh allocation,
subsequent reuse and retained last-successful diagnostics.

## Frozen-source regression

Snapshot: **2,808 source paths**, excluding planning records and user `.vscode/`.
Root serializes build/test commands. No-run builds use `MCPP_JOBS=4`, native
execution uses `MCPP_JOBS=1`. WSL source is synchronized only to the dedicated
`/root/cgpui-logic-cleanup-20261123` validation mirror, not a user checkout.

| Validation | Actual result | Evidence |
| --- | --- | --- |
| Windows full no-run/execution | **760 built; 760 passed, 0 failed** | `windows-final-build.log`, `windows-final-test.log` |
| Architecture, included above | 209 passed | `windows-final-test.log` |
| Renderer, included above | 89 passed | `windows-final-test.log` |
| Windows all-member build | PASS | `windows-all-build.log` |
| WSL full execution/all-member build | **644 passed, 0 failed**; build PASS | `wsl-full-test.log`, `wsl-all-build.log` |
| Native DX12 adapter 0 + debug (Intel) | **86 passed, 0 failed** | `native-0.log` |
| Native DX12 adapter 1 + debug (NVIDIA) | **86 passed, 0 failed** | `native-1.log` |
| Native DX12 WARP + debug + GPU validation | **86 passed, 0 failed** | `native-warp.log` |
| ASan DX12, WARP + debug | **86 passed, 0 failed** | `asan-test.log` |
| ASan renderer | **89 passed, 0 failed** | `asan-renderer-test.log` |
| SDK graph/README fixtures | 9 passed | `sdk-graph-fixtures.log` |
| SDK header fixtures | 15 passed | `sdk-headers-fixtures.log` |
| External installed-only Windows consumer | compile/link/run PASS | `installed-sdk-windows.log` |
| Windows frozen-source check | all 2,808 paths match raw bytes | `source-match.json` |
| Dedicated WSL source check | all 2,808 paths match; CRLF/LF normalized | `wsl-source-match.json` |

Counts are separate runs, not additive totals. The new native geometry tests run
on each selected adapter; the existing loader tests intentionally launch their
own WARP/Vulkan children. No source/test/document changes occurred after the
snapshot; only excluded planning records were updated from the logs.

The unchanged Win32 Popup and X11 event-loop tests that intermittently failed
in the previous milestone passed in this full run. They were not modified and
their earlier intermittent behavior is not claimed fixed by geometry pooling.
This run required no full-test retries.

## Installed consumer

Copied public headers and same-volume hardlinked production static libraries
were compiled and linked using the actual **/MT** CRT with Vulkan SDK paths
removed. The consumer executes three 128-rectangle reversed-color frames through
the public API, checks all 128 grid-cell center pixels each frame and unchanged public
diagnostics. Its existing text/image/shadow/blur, Popup and initial client-alpha
capture assertions also pass. Both clean and owned-invalid-app-local-Vulkan-DLL
runs emit `SDK_GEOMETRY_UPLOAD repeated_grid=128x3 all_pixels=matched`; here
`all_pixels` refers to all 128 sampled cell centers, not every framebuffer pixel.
The executable has no Vulkan PE import and does not load Vulkan on DX12.

Generated CMake configuration passed with Vulkan discovery disabled; this is
**configure-only**, not a CMake binary build or release CRT test. The stage's
`verification.json` records hardlinked/non-immutable libraries and actual CRT.
It is not a published SDK release. This avoids duplicating the large library set.

## Limits

This completes a bounded resource-allocation improvement, not complete DX12
parity, GPU-performance gates, default switching or release acceptance. It does
not fix ordinary client-titlebar/HMENU geometry, dynamic opaque-to-alpha upgrades
or cross-thread device-loss coordination. Existing native tests and WSL do not
certify arbitrary OS/driver/compositor behavior or native macOS.
WSL logs retain dzn testing-only/non-conformance and mcpp runtime-facts/host
sysroot warnings; this run does not certify Vulkan conformance or a hermetic
toolchain. Page Map/allocation failure handling is source-reviewed, not a claim
that real driver out-of-memory/mapping failures were injected in the native run.

Installed verification uses copied public headers and same-volume hardlinked
static libraries, not an immutable SDK release snapshot; CMake checks are
configure-only. Source hash checks establish source consistency, not independent
proof of executable provenance. No additional old SDK deletion was needed.
