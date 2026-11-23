# Ordinary DX12 transparency and native HMENU: probe evidence

Date: 2026-11-23. This is an investigation result, **not an enabled feature**.
Normal transparent DX12 creation and enabling transparency still return
`unsupported_platform`; opaque normal windows and native alpha Popups are
unchanged. The adopted policy retains private DComp without public window modes.

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

## Next gate

Do not ship one of these recipes by hiding HMENU, forcing framework menus,
adding application-facing presentation modes, or silently changing backend.
A future candidate must preserve native drawing and then pass actual menu-loop
commands, menu installation/removal, client geometry, activation, resize/DPI,
backdrop changes, input, teardown and device recovery. Opaque-to-transparent
switching also needs evidence that a retired HWND flip swapchain leaves no stale
frame, with restoration on failure. A child host alone does not make the parent's
opaque backing transparent.

This milestone separately fixes the backend-neutral runtime clear policy for
already alpha-capable ordinary windows. Recording-frame tests for that fix do
not validate an ordinary DX12 topology or native macOS behavior.

Primary references:
- [GLFW 3.4 Win32 transparency recipe](https://github.com/glfw/glfw/blob/3.4/src/win32_window.c#L371-L403)
- [DWM blur overview and alpha handling](https://learn.microsoft.com/en-us/windows/win32/dwm/blur-ovw)
- [DwmEnableBlurBehindWindow and GDI alpha caveats](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/nf-dwmapi-dwmenableblurbehindwindow)
- [Frame extension and negative margins](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/nf-dwmapi-dwmextendframeintoclientarea)
- [System backdrop types](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwm_systembackdrop_type)
