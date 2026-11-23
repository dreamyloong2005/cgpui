# Acceptance: DX12 startup without a Vulkan loader dependency

Date: 2026-11-23. Baseline `2a3e9bc6`, branch `phase-v`.
Status: **PASS for this bounded loader-independence milestone**.
Evidence: `D:/Dev/shots/cgpui-dx12-loader-20261123/`.

## Authorized SDK cleanup

User requested old SDK cleanup before continuing. The cleanup validated exact
manifest/library correspondence, non-release verification metadata, absolute
paths under `D:/Dev/shots`, absence of reparse points and independent old copies.
It removed **140 `.lib` files across 12 old verification stages**, freeing
**8.02 GiB**. There was no recursive directory removal. Headers, consumers,
executables, manifests and logs remain, with a removal notice in each stage.
The newest client-alpha hardlinked SDK, the earliest stage without a complete
manifest, repository source and target libraries were not removed.

Inventory and before/after capacity:
`D:/Dev/shots/cgpui-sdk-cleanup-20261123/report.json`.
D: had 8.16 GiB immediately after cleanup and about 6.16 GiB after this turn's
full builds, including the additional renderer ASan suite.

## Delivered contract

- The normal Windows dispatcher and installed SDK no longer have an ordinary
  or delay import of `vulkan-1.dll`. Choosing DX12 does not initialize Vulkan.
- Windows Vulkan initialization loads on demand using safe default DLL search:
  application directory, System32 and explicitly registered user directories,
  not ambient PATH/CWD. No process-wide search-policy mutation or public loader
  option is introduced.
- A focused private module resolves **88 required typed exports** into a
  candidate before publishing any pointer. A mutex serializes admissions;
  successful pointers remain stable and the code is retained for process
  lifetime. Failed candidates are released and attempts may be retried.
- Missing/damaged loader or missing required export becomes an ordinary typed
  initialization error with the relevant diagnostic, not a startup failure or
  silent fallback. Existing optional version/extension queries remain optional.
- Vulkan stays the default/auto backend. Linux keeps its native Vulkan link.
  Repository builds still need Vulkan SDK headers on Windows; installed-only
  consumers need no Vulkan SDK headers/import library/delay-load flags.
- The SDK CMake generator discovers Vulkan only for legacy graphs that actually
  contain `vulkan-1`. Generated README and the existing legacy xmake consumer
  follow the same condition. No new xmake file was added.

## Red-to-green evidence

`loader-independence-red.log` records the public fresh-process test failing at
entry with exit 1: "Vulkan loader was imported before DX12 selection". The same
test now captures real DX12/WARP pixels while Vulkan is absent before, during
and after the renderer lifetime. The original PE import was separately observed.

`sdk-loader-red.log` records unconditional CMake `find_dependency(Vulkan)` failing
the installed-consumer requirement. Nine graph/README fixtures now pass.

The public isolation test copies itself only into an owned temporary directory:

- An exact invalid-PE app-local `vulkan-1.dll` does not prevent DX12/WARP pixels.
- Explicit Vulkan, default and auto return `renderer_initialization_failed`
  with `vulkan-1.dll` and Win32 error 193, leaving no loaded module or fallback.
- An invalid explicit DX12 adapter fails without loading Vulkan.
- In the same process, a failed Vulkan factory is followed by removal of only
  the strictly validated temporary marker DLL; a second factory then renders
  through the healthy system Vulkan loader. The module remains after teardown.
- A separate healthy Vulkan child checks lazy loading, metadata and real pixels.

The binding test refuses each of 88 missing exports and invalid resolver inputs,
without publishing globals or invoking dummy addresses. The native concurrency
test starts 16 threads together and checks complete, stable real pointers plus
module retention. These are distinct private-seam and native public tests.

## Frozen-source validation

| Validation | Actual result | Evidence |
| --- | --- | --- |
| Windows full no-run/execution | 744 built; **744 passed, 0 failed** | `windows-final-build.log`, `windows-final-test.log` |
| Architecture, included above | 207 passed | same full log |
| Renderer, included above | 89 passed | same full log |
| Windows all-member build | PASS | `windows-all-build.log` |
| WSL full execution/all-member build | **642 passed, 0 failed**; build PASS | `wsl-full-test.log`, `wsl-all-build.log` |
| Native DX12 suite, adapter selector 0 + debug | **72 passed, 0 failed** | `native-0.log` |
| Native DX12 suite, adapter selector 1 + debug | **72 passed, 0 failed** | `native-1.log` |
| Native DX12 suite, WARP + debug + GPU validation | **72 passed, 0 failed** | `native-warp.log` |
| ASan native DX12 | **72 passed, 0 failed** | `asan-test.log` |
| ASan renderer | **89 passed, 0 failed** | `asan-renderer-test.log` |
| SDK graph/README fixtures | 9 passed | `sdk-graph-fixtures.log` |
| SDK header fixtures | 15 passed | `sdk-headers-fixtures.log` |
| External installed-only consumer | compile/link/run PASS | `installed-sdk-windows.log` |
| Windows frozen source | 2,782 paths match raw bytes | `source-match.json` |
| Dedicated WSL source | 2,782 manifest paths match, text CRLF/LF normalized | `wsl-source-match.json` |

Counts describe separate runs and are not additive. The adapter runs exercise
the existing native suite under Intel/NVIDIA/WARP selection; the new loader
probes intentionally select WARP for their DX12 children regardless of the outer
selector, and healthy Vulkan children use the available Vulkan adapter. No test
of physically removing a system DLL is implied.

The snapshot excludes planning records and user `.vscode/`. WSL verification uses
only the dedicated `/root/cgpui-logic-cleanup-20261123` mirror, not a user checkout;
its check covers all manifest paths, not unrelated build/cache files.

## Installed consumer detail and limits

The actual link graph contains no Vulkan import library, SDK link directory or
delay-load option. External headers were copied and libraries hardlinked after
all production builds. The consumer used the actual **/MT** CRT, checked against
the core archive, with `VULKAN_SDK` and its PATH/include/lib entries removed.

The actual executable's PE imports have no `vulkan-1.dll`. Both clean and owned
invalid-app-local-DLL runs passed real opaque/text/image/shadow/blur, Popup alpha
and initial client-title-bar alpha GPU checks, with no Vulkan module at entry or
exit. This is not a desktop-pixel test of the installed consumer.

Generated CMake configuration also succeeds with Vulkan discovery disabled.
This is **configure-only**, not a CMake binary build or release CRT acceptance;
CMake's unused `CMAKE_DISABLE_FIND_PACKAGE_Vulkan` warning reflects that the
generated package never invokes Vulkan discovery. The stage records its actual
runtime and is explicitly not a release package or immutable library snapshot.

Independent Standards and Spec reviews report 0 actionable findings; see
`review.md`. No source change occurred after the full validation snapshot.

This milestone does not solve dynamic opaque-to-transparent upgrades, the
client-title-bar/native-HMENU layout limitation, full diagnostics/performance
parity or a default-backend switch. The physical-absence case is supported by
absence of PE imports and no loader admission on DX12, not tested by deleting a
system DLL. Missing exports are tested through the resolver seam, not 88 real
DLL variants. No native macOS, arbitrary TDR or release claim is made.
