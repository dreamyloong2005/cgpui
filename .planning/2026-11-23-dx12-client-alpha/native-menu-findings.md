# Client title bar / native HMENU layout investigation

Status: **candidate failed; not a supported implementation** (2026-11-23).
No conclusion here establishes that all possible Windows implementations fail.

## Observed feedback loop

The public `win32_client_title_bar_menu_test` creates an opaque client-title-bar
window, installs a real native menu, and requires its OS-reported menu/item
rectangles to be outside the client rectangle, directly below the window's top
frame (without a vacant system-caption band). This precedes alpha testing.

- Baseline RED: `client-menu-red.log`, exit 6. Menu `277,311..907,340`,
  client `277,266..907,626`: the menu overlaps the client area.
- Candidate RED: `client-menu-candidate.log`, exit 6. Menu
  `315,349..945,378`, client `315,345..945,664`: the candidate's proposed
  `WM_NCCALCSIZE` origin shifts the client, but the actual menu still occupies
  its old caption-relative area and overlaps the client. The existing
  `win32_client_title_bar_test` passes in the same run.
- Logs live under `D:/Dev/shots/cgpui-dx12-client-alpha-20261123/`.

The rejected candidate copied the NCCALCSIZE input and reduced its proposed
top by `caption_height - frame` before calling `DefWindowProcW`. Calling DWP
with an adjusted proposal is therefore **not sufficient** to move this real
native HMENU. Do not weaken the geometry assertion or accept a blank opaque
reference to promote it to success.

## Primary-source boundaries

1. [WM_NCCALCSIZE](https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-nccalcsize)
   defines the rectangular client-area calculation. With `wParam=TRUE`, returning
   zero without modifying the rectangles removes the standard frame/caption.
   It does not document a separate native menu-position operation.
2. [Custom Window Frame Using DWM](https://learn.microsoft.com/en-us/windows/win32/dwm/customframe)
   documents client-frame extension, standard-frame removal, custom drawing and
   hit testing. It does not promise that changing NCCALCSIZE relocates a retained
   native HMENU. Its frame-extension recipe is not proof of alpha/native-menu
   compatibility and must not replace this project's verified material path.
3. [SetMenu](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setmenu)
   attaches a menu to a non-child window; the window is redrawn after a change.
   [DrawMenuBar](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawmenubar)
   redraws the changed menu bar. The current `win32_apply_native_menu` already
   calls both; neither signature accepts a position.
4. [Window Styles](https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles)
   describes `WS_CAPTION` as the title bar including `WS_BORDER`; `WS_CHILD`
   windows cannot have a menu bar. Consequently a child HWND menu host is not a
   documented substitute for the same native menu, and removing `WS_CAPTION`
   would be a real style/topology change, not another client-rectangle adjustment.
5. [GetMenuBarInfo](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmenubarinfo)
   retrieves [MENUBARINFO](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-menubarinfo),
   whose `rcBar` reports the menu rectangle. The documented setter
   [SetMenuInfo](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setmenuinfo)
   accepts [MENUINFO](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-menuinfo):
   style, max-height, background, help ID and application data, but no origin.

## Ranked next hypotheses, not conclusions

1. Native menu placement follows the actual `WS_CAPTION` style, independently of
   the adjusted NCCALCSIZE proposal. A standalone style experiment without
   `WS_CAPTION` would predict a menu directly below the frame. Production alpha
   currently explicitly requires `WS_CAPTION`, so even an opaque experiment
   passing would not justify enabling alpha or silently changing production
   styles. Verify frame, resize, controls, native menu pixels and alpha separately.
2. A cached menu layout survives the adjusted proposal. A real frame/menu
   invalidation might change the result, but this is lower-confidence because
   the existing path already uses `SetMenu` plus `DrawMenuBar`. Success must
   include real `GetMenuItemRect`, pixels and native interaction, not just the
   returned client rectangle.
3. The current documented contract may not support all of retained `WS_CAPTION`,
   app-owned in-client title bar, native HMENU outside the client, and no empty
   caption band. If verified mechanisms fail, keep that combination unsupported;
   do not use owner-draw, undocumented UAH/menu APIs, unrelated popup windows or
   a public renderer/window-mode switch as a hidden substitute.

After a viable mechanism passes the minimal test, expand to narrow widths and
wrapped rows (`GetMenuItemRect` proves multiple distinct row tops), maximize and
restore, menu removal/reinstallation, resize edge hits, and true selection/cancel
loops. Do not add pass-shaped assertions around the rejected candidate.

## Isolated captionless-style experiments and scope decision

The separate raw-HWND `win32_client_menu_style_probe_test` did not call the
production candidate leaf or change the alpha gate. With an HMENU it passed
NCCALCSIZE directly to DefWindowProc using the actual native style.

- Probe v1, `client-menu-style-probe.log`, exit 43: both the requested original
  and requested captionless styles became actual style `0x14cf0000` after
  CreateWindowEx. This was **not** a true captionless experiment: USER32
  normalized the new overlapped-window style to include `WS_CAPTION`.
- Probe v2, `client-menu-style-probe-v2.log`, PASS: after creation and before
  showing, explicit `SetWindowLongPtr(GWL_STYLE, actual & ~WS_CAPTION)` plus
  `SWP_FRAMECHANGED` succeeded; actual visible style was `0x140f0000`.
  Native menu `430,230..1130,259` lay below the frame; client
  `430,260..1130,690` was separate with no old caption band. The original
  style remained a strict-geometry negative control.
- Both cases passed real File/Open selection and Escape cancellation, a
  real `SC_KEYMENU/L' '` system-menu open/cancel loop, six expected system-menu
  commands, maximize/restore/minimize/restore state changes, and synthetic
  caption/maximize/menu/resize hit tests. Frame-only geometry also passed
  while maximized/restored. These results do not validate physical dragging,
  Aero Snap, accessibility, multi-DPI behavior, or alpha on that topology.

The captionless topology is **not adopted**: Microsoft's
[Window Styles](https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles)
documents `WS_SYSMENU` with the requirement "The WS_CAPTION style must also be
specified." A successful local probe does not turn the unsupported combination
into the framework's native-system-menu contract. Production retains its existing
style and no-menu client-title-bar geometry rather than trading native behavior
for an unverified topology.

Current bounded implementation scope: initial client-title-bar DX12 alpha only
when no native HMENU is attached. Transparent renderer creation must refuse an
already menu-bearing client window without hiding/replacing that menu. While a
client-alpha lease is active, application-wide nonempty native-menu installation
must be rejected before changing any HWND, menu model or accelerator table.
Clearing the menu remains allowed; after the last alpha lease is released, native
menu installation is allowed again, and a subsequent transparent renderer must
again refuse the menu-bearing client. Existing system-caption alpha with native
menus remains supported. This is explicit refusal, not silent overlay or menu
substitution.
