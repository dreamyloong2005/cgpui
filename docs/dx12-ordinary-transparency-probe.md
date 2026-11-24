# Ordinary DX12 transparency and native HMENU: probe evidence

Date: 2026-11-23. This document preserves the **negative uncorrected probe** and
its later successful alpha-repair follow-up. At that milestone the bounded slice
accepts initial transparent DX12 creation on a CGPUI-owned, decorated ordinary
window with a visible system title bar, Windows build 26100+, and successful
native alpha enablement. Raw HWNDs, other ordinary chrome and dynamic upgrades
were unsupported. The newer implementation adds eligible client-title-bar initial
alpha and composition-first dynamic alpha, without converting old HWND-flip
history. See [current backend status](dx12-backend.md) for supported combinations
and the separate, still-pending combined-batch acceptance. This historical probe
does not validate those additions. DComp stays private; there is no public
window-mode choice.

## Question and experiment

Can a redirected, decorated `WS_OVERLAPPEDWINDOW` retain native HMENU drawing
while a direct DX12 composition swapchain presents real per-pixel desktop alpha?
The probe deliberately bypasses production surface policy. Each case creates a
fresh HWND and native menu over its own solid-green backing window, renders two
different frames with alpha 0/0.5/1 bands, and checks both GPU RGBA and desktop
RGB. The menu has an owner-drawn color marker and a separate **standard native
text item**. The marker is a diagnostic, not a proposed menu replacement.

Eleven recipes were tested on Intel Graphics, NVIDIA RTX 5070 Ti Laptop GPU and
WARP, each with DX12 debug requested, on Windows build 26300.9974:

| Recipe | Desktop alpha | Native menu result |
| --- | --- | --- |
| Plain redirected, white GDI client | No; white backing | Reference menu retained |
| Full `-1` frame extension, black GDI client | No; white backing | Menu blank |
| Blur-behind whole client, black GDI client | No; green becomes approximately `(3,171,3)` | Menu colors altered |
| Full extension + whole-client blur | Yes | Native text colors altered |
| `NOREDIRECTIONBITMAP` control | Yes | Menu disappears |
| GLFW-style blur region, no extension | Yes | Native text colors altered |
| Same + explicit `DWMSBT_NONE` | Yes | Native text colors altered |
| Full extension + GLFW-style region | Yes | Native text colors altered |
| Same + explicit `DWMSBT_NONE` | Yes | Native text colors altered |
| Positive bottom margin = actual client height + GLFW-style region + NONE | Yes | Native text colors altered |
| Positive bottom margin + whole-client blur + NONE | Yes | Menu blank |

The GLFW-style `CreateRectRgn(0,0,-1,-1)` returned a simple region bounded by
`(-1,-1,0,0)` on this host, not `NULLREGION`; the probe records this rather than
assuming an empty region. Client height is queried after native menu installation.
No hidden child presentation window, D3D11 bridge or readback presentation is used.

All 33 cases produced the expected GPU pixels; all setups completed. Eight of
eleven recipes per adapter displayed correct desktop alpha, but none preserved
the reference menu drawing at the same time. **This is not proof that every
possible Windows/DComp topology is incompatible with native menus.**

## Why a menu marker failure is not the entire conclusion

The final probe captures only its own target window and compares the standard
text item, independently of the owner-drawn marker. For the no-extension
GLFW-style region, the native glyphs remain visible but are incorrectly green:

- Opaque reference: all 553 solid glyph pixels are `(160,160,160)`.
- Transparent candidate: those same pixels are `(160,255,160)`.
- The text background changes from `(240,240,240)` to `(240,255,240)`.
- The positive-bottom + GLFW-style region + NONE candidate has the same result.

These values agree across all three adapters. Thus "all menus disappeared"
would be an incorrect summary: some are blank, some disappear, and some retain
text with corrupted color/alpha. `GetMenu`, `DrawMenuBar`, successful DWM calls
and a nonempty text ROI alone would all give false confidence. Marker changes
are already observable after DWM setup, before DX12/DComp creation.

## Retained sources and reproduction

The throwaway source and temporary mcpp registration are archived on local
branch `codex/prototype-dx12-ordinary-20261123`, commit
`3cde7234d70ab341b10be8a7b4ae7f072e636f10` (based on `65eb6ae6`). They are not in
the production test suite: the probe exits successfully after recording negative
observations, so its process success must not be reported as feature acceptance.

In a checkout of that archive, create a fresh absolute output directory and run:

```powershell
$env:MCPP_JOBS = '1'
$env:CGPUI_DX12_DEBUG = '1'
$env:CGPUI_DX12_ADAPTER = 'warp' # Repeat in fresh processes with '0' and '1'.
$env:CGPUI_HOST_PROBE_OUTPUT_DIR = '<fresh absolute directory>'
scripts/dev/test.ps1 -p tests/renderer-dx12 dx12_ordinary_host_probe_test
```

Evidence: `D:/Dev/shots/cgpui-dx12-ordinary-20261123/`:
`probe-v3-{warp,0,1}.log`, corresponding `*-images/` BMPs,
`compare-menu-rgb.py`, `menu-rgb-comparison.json`, and archived v1/v2/v3 sources.
The captured target rectangle is 400x280; this is not a whole-desktop screenshot.

## Follow-up: native RGB was intact, menu alpha was not

The next probe copied the actual OS-rendered menu rectangle from `GetWindowDC`
into `BeginBufferedPaint` with `BPPF_NONCLIENT`, flushed the GDI copy, set only
its alpha to 255 with `BufferedPaintSetAlpha`, and copied it back. It did not
replace the HMENU, draw text, alter RGB, or use undocumented UAH messages.

A valid reference must first stabilize: require three consecutive equal RGB
and focus samples 100 ms apart, bounded to two seconds. Early comparisons against
a still-animating native menu were invalid references, not permission to loosen
the pixel oracle. With a stable reference, both the earlier blur-region recipe
and `DWMWA_REDIRECTIONBITMAP_ALPHA` preserved **all 10962 menu pixels exactly**
while the client composited alpha 0/0.5 over green, for changing red/blue frames.
The uncorrected case still failed all menu pixels; the opaque control preserved
the menu but failed client transparency.

The selected production mechanism is the Windows 26100+ redirection-bitmap-alpha
attribute, not the blur recipe, so material/frame-extension ownership stays
independent. Repair runs after documented native nonclient/menu events, not as
a test-side or per-frame workaround. The window instance is rechecked after
dispatch because a genuine menu command can destroy a window. The private
platform/renderer protocol carries instance/owner tokens, not borrowed pointers.

Formal tests use public `PlatformApplication` and `Renderer` APIs. A same-window
opaque reference is captured **before any renderer** exists, followed by initial
transparent renderer creation. Timer-driven `SC_KEYMENU` enters the real native
File/Open menu loop; tests observe the matching native popup and highlighted
items, capture while open, and deliver Return or Escape without synthesizing
`WM_COMMAND`. Successful command paths must emit the public `NativeMenuCommand`;
Escape must emit none. Whole-menu RGB and focus match exactly before/during/after.
No global `SendInput` is used; this does not prove physical mouse-hover behavior.

Initial 2026-11-23 native runs passed the initial-alpha/menu/cancel, menu changes,
resize/renderer-scale, lifetime, two idle-device-removal/opaque-peer recovery,
and opaque-history regressions. The Mica follow-up reported actual material
application, zero changed native-menu RGB pixels and correct premultiplied GPU
pixels; returning to `none` restored real desktop alpha with a new blue frame.
These are one-host acceptance observations, not certification across all three
adapters from the earlier negative matrix or every Windows configuration.

Follow-up evidence lives in `D:/Dev/shots/cgpui-dx12-nc-alpha-20261123/`, including
`stable-attribute-1.log`, `stable-blur-0.log`, `stable-none-1.log`,
`auto-attribute-active.log`, the `dx12_ordinary_*-first-run.log` files and
`ordinary-backdrop-first-run.log`. See [current backend scope](dx12-backend.md).

## Historical initial-only boundary and remaining gates

Do not ship one of these recipes by hiding HMENU, forcing framework menus,
adding application-facing presentation modes, or silently changing backend.
The new bounded candidate preserves native drawing and has focused regressions
for command/cancel loops, menu installation/removal, client geometry, activation,
resize/renderer DPI, Mica/none, teardown and idle-device recovery. It does not
claim support for other ordinary chrome, fullscreen transitions, real monitor
migration, physical pointer interactions or arbitrary driver TDR.

In a separate transition probe, a fresh ordinary DComp host worked, but an
ordinary HWND previously presented through an opaque flip swapchain retained
stale client content after its old GPU owners were retired. At that milestone,
production opaque windows therefore kept their flip route and rejected transparent
intent. A composition-from-first-frame experiment passed its own strict toggle
control but had not yet been adopted. A child host alone did not make the parent's
opaque backing transparent.

The current implementation now gives eligible fresh managed ordinary windows a
private composition-from-start **IGNORE** route with dynamic premultiplied-chain
replacement. This newer implementation is not acceptance established by the old
probe. Existing HWND-flip instances still retain conservative history and refuse
the rejected upgrade, even after renderer destruction. See
[current backend scope](dx12-backend.md) for present eligibility, constraints and
validation status; do not read the historical initial-only result as the current
policy for every opaque window.

The backend-neutral runtime clear policy for alpha-capable ordinary windows is
separate. Its recording-frame tests do not replace these native DX12 desktop
oracles and do not validate native macOS behavior. Positive ordinary-alpha tests
on builds below 26100 first verify `unsupported_platform`, then explicitly skip;
that older-OS path has not yet been exercised on an older Windows host.

Primary references:
- [GLFW 3.4 Win32 transparency recipe](https://github.com/glfw/glfw/blob/3.4/src/win32_window.c#L371-L403)
- [DWM blur overview and alpha handling](https://learn.microsoft.com/en-us/windows/win32/dwm/blur-ovw)
- [DwmEnableBlurBehindWindow and GDI alpha caveats](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/nf-dwmapi-dwmenableblurbehindwindow)
- [Frame extension and negative margins](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/nf-dwmapi-dwmextendframeintoclientarea)
- [System backdrop types](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwm_systembackdrop_type)
- [DWM window attributes](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute)
- [BufferedPaintSetAlpha for GDI-drawn buffered pixels](https://learn.microsoft.com/en-us/windows/win32/api/uxtheme/nf-uxtheme-bufferedpaintsetalpha)
- [Nonclient buffered paint parameters](https://learn.microsoft.com/en-us/windows/win32/api/uxtheme/ns-uxtheme-bp_paintparams)
