# Common pixel and real runtime acceptance

2026-11-24. Baseline `36b3cd40`; bounded milestone accepted, entire DX12 goal active.

Evidence root: `D:/Dev/shots/cgpui-dx12-common-parity-20261124`.
**Final accepted matrix is `verified/`**, not the first interrupted matrix.

## Changes and actual RED/GREEN

- Thirteen existing pixel scenes now share an explicit named Vulkan/DX12 creator,
  strict actual backend/selection/fallback/adapter checks, and unchanged pixel
  oracles/tolerances. A requested-DX12/actual-Vulkan identity and clip case really
  failed before the one-expression factory fix; the same cases then passed DX12.
- Vulkan exact backdrop topology 0/5/9 remains separately tested; DX12 0/3/6 is
  independently asserted. Private Vulkan tests remain Vulkan tests even when the
  containing suite is repeated under the DX12 selector.
- Real native root-first/child-first WindowRuntime cases remove the device and
  observe automatic typed-retry redraw, same two renderer factories, generation
  increment, once-uploaded image/glyph restoration, matching pixels and warm cache.
  No present interception, fake success or fixture retry loop is used.
- Explicit `-Renderer both` performs two runs, retaining the first nonzero exit;
  ordinary native failures do not skip the second backend. Defaults unchanged.
  Actual native-command boundary, PS5 StrictMode, ASan forwarding and state
  restoration have isolated fixtures. Windows CI registers both PS hosts only;
  hosted execution/GPU CI is not claimed.

## Preserved integration failures

1. Old capture reference: renderer_frame_pixels exit50, fixed path, same test green.
2. Old closeout references: phase_e closeout exit11, migrated references with capture
   assertions retained, same test green.
3. Documentation exceeded existing 180-line cap: architecture 211/212; condensed
   guide, unchanged guard passes. No structural cap was raised.
4. First integrated fixture invocation used a nonexistent custom TEMP parent:
   setup failed; directory created and fixtures passed. Original log retained.
5. First full matrix passed all C++/GPU/ASan gates but stopped at four fixture
   assertions: short (8.3) TEMP versus PowerShell long-path spelling. Tight RED
   and `samefile=True` proved the cause. Canonicalizing the newly created fixture
   directory fixed it without relaxing exact assertions or changing production.
   Dedicated genuine Win32-short-alias ordinary/ASan cases both failed pre-fix,
   then passed on PS7 and PS5. Source was refrozen and the entire matrix rerun.

## Final frozen-source results (`verified/`)

| Gate | Actual result |
| --- | --- |
| Windows all tests | 777 pass, 0 fail; all-member build passes |
| WSL dedicated mirror | 647 pass, 0 fail; all-member build passes |
| Architecture (inside full suite) | 212 pass, original caps retained |
| DX12 Intel / NVIDIA / WARP | 97 / 97 / 97 pass, debug enabled; WARP also GPU validation |
| Common renderer DX12 Intel / NVIDIA | 92 / 92 pass, actual hardware identity logged |
| Common renderer explicit both with WARP | Vulkan 92 and DX12 92; actual WARP software identity |
| ASan DX12 | 97 pass |
| ASan common explicit both | Vulkan 92 and DX12 92 pass |
| Driver fixtures PS7.6.9 | 17 pass |
| Driver fixtures PS5.1.26100.9818 | 15 pass, 2 explicit PS7-only skips |
| SDK graph / header fixtures | 9 / 15 pass |
| Installed Windows consumer | actual /MT compile/link/run, clean and invalid app-local Vulkan loader |
| Source consistency | all 2859 paths match on Windows and normalized WSL |

All 13 common scenes were included; the 92-suite total is not 92 DX12-only tests.
No native suite SKIP marker appeared in these three local adapter logs; adapter
isolation exercised distinct LUIDs. The Windows and WSL source checks exclude
planning and user .vscode only, not implementation/tests/docs. Final pipeline
exited zero. Standards and Spec reviews found 0 actionable issues; Standards
also independently reviewed the final fixture correction.

## Limits and next work

Installed libraries are same-volume hardlinks to verified local builds, not an
immutable release. The consumer confirms opaque/Popup/client-alpha captures,
128 texture uploads/cold-warm-replacement texels, and no Vulkan PE import/load;
CMake is **configure-only**, not a CMake binary build or release CRT acceptance.
WSL warnings remain in logs; WSL is not native Linux desktop/macOS certification.
No FPS/performance, arbitrary driver TDR, old-Windows, complete ordinary-window
menu/alpha compatibility, cross-thread automatic recovery, default switch or
release/publication claim follows. No push.

Next prepared bounded fix: unsupported DX12 backdrop must follow the public
`tint only` contract. Native RED/B candidates are external and unapplied here.
Pacing and WARP-CI investigations remain proposals, not completed production work.
