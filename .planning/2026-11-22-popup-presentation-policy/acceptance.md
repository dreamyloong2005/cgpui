# Popup policy acceptance — 2026-11-22

## Final contract

The final user instruction is to keep Vulkan Popup and use the overlay only
on failure, not to ban Windows Vulkan by backend identity. Supported Vulkan
and Metal renderers opt in. The runtime intersects actual renderer and
platform capability, attempts independent popups when supported, and rejects
unsupported direct popup requests without activating partial windows.

Tooltip and PopoverMenu retain the existing overlay path for unavailable
capability, failed native creation, failed renderer initialization or failed
placement. Their existing explicit widget popup opt-out is unchanged.
Presentation remains private; no native/composition window mode is exposed.
The Windows Vulkan/D3D11/DComp path remains in use. DX12 is not implemented
in this slice. Normal-window GPU selection, transparency/backdrops and native
menus are unchanged.

## Scope and ownership

- Main checkout: `D:/Dev/Projects/cgpui`, branch `phase-v`, baseline `11f7eb76`.
- Design checkout: `D:/Dev/Projects/cgpui-dx12-design`, branch `dx12-design`,
  baseline `b513dc0c`. Its plan and GPUI research note are separate changes.
- Neutral public capability is a declaration in the renderer leaf header;
  base behavior, Vulkan policy and runtime negotiation have focused sources.
- Window activation remains exactly 70 lines. Existing architecture limits
  were not relaxed; the new test is also registered in the CI header matrix.
- Preserve pre-existing `.vscode/`; no push, merge, global build-config change
  or broad filesystem cleanup.

## Verification

Logs are under `D:/Dev/shots/dx12-alpha-no-dcomp-20261122/`.
Windows uses mcpp 2026.11.21.1 through `scripts/dev/test.ps1` with
`MSVC_ROOT=D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207`.

| Check | Result | Log |
| --- | --- | --- |
| Real Win32 Vulkan capability, before restoring support | RED, exit 9 | `vulkan-popup-restored-red.log` |
| Same capability after restoring support | 1/1 passed | `vulkan-popup-restored-green.log` |
| Windows renderer/UI/architecture `popup` filter | 9/9 passed | `popup-failure-only-targeted.log` |
| Windows full suite, `MCPP_JOBS=1` | 598/598 passed | `windows-full-failure-only-serial.log` |
| WSL renderer/UI/architecture `popup` filter | 8/8 passed | `linux-popup-failure-only-final.log` |
| WSL full architecture suite | 188/188 passed | `linux-architecture-failure-only-final.log` |
| WSL Vulkan surface filter | 4/4 passed | `linux-vulkan-surfaces-failure-only-final.log` |

The successful-popup control still creates an independent menu and does not
paint it again in the root. Four failure cases exercise Tooltip/PopoverMenu
overlay painting, keyboard/mouse activation, dismissal and native/record
cleanup. Real Win32 popup pixel tests cover Intel/NVIDIA, normal menus and
pointer-transparent tooltips, including alpha-zero and half-alpha pixels.
The Linux mirror's 31 scoped files were verified byte-for-byte with SHA256;
Wayland and X11 surface tests executed, rather than taking environment skips.

Two default-parallel Windows full runs returned 597/598, both at the unchanged
`win32_popup_window_test` exit 15 (compound owner/visibility/nonactivation
assertion). The isolated test passed unchanged, and the full run with the
official per-invocation `MCPP_JOBS=1` override passed 598/598. mtest records
confirm overlapping popup/focus/input/activation test processes without
resource mutual exclusion in the parallel failures. This supports an
isolation concern, not proof of the exact failed conjunct. No assertion was
weakened; no test-runner/global configuration change or test-flakiness fix is
included in this slice.

## Standards

Independent final review found no actionable violations. It checked actual
older structure constraints, module ownership, CI/mcpp registration, thin
public/aggregate headers and absence of public presentation-mode leakage.

## Spec

Independent final review found no actionable deviations. It verified Vulkan
opt-in, both runtime guards, successful popup behavior, genuine failure
fallback, cleanup, and exclusion of unimplemented DX12 work.

## Validation limits

No native macOS, full Linux suite or ASan run in this slice. WSL rendering
uses a dzn implementation that warns it is not Vulkan-conformant; mcpp also
warns about missing SubOS runtime metadata and uses host glibc. These runs
are not proof of native macOS behavior or hermetic SDK packaging.
