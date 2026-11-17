# System appearance and DPI awareness

CGPUI reports the desktop's appearance settings (light or dark, accent color,
transparency effects, client-area animation, high contrast) and notifies the
application when the desktop announces a change: Windows through broadcast
window messages, Wayland through the xdg-desktop-portal Settings interface.
On Windows it also declares Per-Monitor V2 DPI awareness before the first
window exists, so windows get real monitor DPI and `WM_DPICHANGED`. On
Wayland it reports the awareness implied by the scale protocols it binds.

## API

`include/cgpui/platform/platform_appearance.hpp`:

```cpp
enum class SystemAppearanceMode { light, dark };

struct SystemAppearance {
  bool supported = false;
  std::string backend;
  SystemAppearanceMode mode = SystemAppearanceMode::light;
  bool accent_color_available = false;
  Color accent_color{};
  bool transparency_effects_enabled = false;
  bool client_area_animation_enabled = true;
  bool high_contrast = false;
  std::string unsupported_reason;  // empty when supported
};

enum class SystemAppearanceChangeSource {
  color_set, colorization, client_area_animation, high_contrast };

struct SystemAppearanceChange {
  SystemAppearanceChangeSource source;
  SystemAppearance appearance;  // snapshot read after the message
  bool changed = false;         // differs from the previous snapshot
};

using SystemAppearanceCallback =
    std::function<void(const SystemAppearanceChange&)>;
```

`include/cgpui/platform/platform_dpi_awareness.hpp` defines
`PlatformDpiAwareness` (`unsupported`, `unaware`, `system_aware`,
`per_monitor_aware`, `per_monitor_aware_v2`) and `PlatformDpiAwarenessState`
(`awareness`, `declared_by_framework`, `already_set_by_process`, `backend`).

Entry points:

| Layer | Query | Notification |
| --- | --- | --- |
| `PlatformApplication` | `system_appearance() const`, `dpi_awareness() const` | `set_system_appearance_callback(SystemAppearanceCallback)` |
| `WindowRuntime` | `system_appearance() const`, `dpi_awareness() const` | `set_system_appearance_callback(SystemAppearanceCallback)` |
| `App` | `system_appearance() const`, `dpi_awareness() const` | `set_system_appearance_callback(SystemAppearanceCallback) const` |
| `Window` | `appearance() const` (the process-wide snapshot) | none |

Notifications are application-level callbacks. `PlatformEvent` does not gain
an appearance alternative because these settings are process-wide on every
backend. The runtime keeps one callback (setting a new one replaces the old,
an empty callback stops delivery). When the bridge is installed
it records a `system-appearance-monitor` diagnostic whose `supported` field
says whether the backend can deliver changes. For each notification it
records a
`PlatformDiagnosticKind::appearance` diagnostic with the operation
`system-appearance-changed`, and calls `request_render()` when `changed` is
true. Then it calls the application callback on the UI thread. The callback
fires for every relevant broadcast, so check `changed` to skip repeats.

```cpp
app.set_system_appearance_callback(
    [&](const cgpui::SystemAppearanceChange& change) {
      if (change.changed) apply_theme(change.appearance);
    });
```

## Windows sources

| Value | Source |
| --- | --- |
| `mode` | `HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize` `AppsUseLightTheme` (0 = dark; missing = light) |
| `transparency_effects_enabled` | same key, `EnableTransparency` (missing = enabled) |
| `accent_color` | `HKCU\Software\Microsoft\Windows\DWM\AccentColor` (0xAABBGGRR, the user accent); falls back to `DwmGetColorizationColor` (the frame tint). Alpha is reported as 1 |
| `client_area_animation_enabled` | `SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION)` |
| `high_contrast` | `SystemParametersInfoW(SPI_GETHIGHCONTRAST)`, `HCF_HIGHCONTRASTON` |

Change messages are received by one hidden top-level window owned by the
Win32 application (`CGPUIAppearanceMonitor`). A message-only window would not
receive broadcasts.

| Message | `source` |
| --- | --- |
| `WM_SETTINGCHANGE`, `lParam` = `"ImmersiveColorSet"` | `color_set` |
| `WM_SETTINGCHANGE`, `wParam` = `SPI_SETCLIENTAREAANIMATION` | `client_area_animation` |
| `WM_SETTINGCHANGE`, `wParam` = `SPI_SETHIGHCONTRAST` | `high_contrast` |
| `WM_DWMCOLORIZATIONCOLORCHANGED` | `colorization` |

Title bars: each CGPUI window gets `DWMWA_USE_IMMERSIVE_DARK_MODE` (attribute
20, falling back to 19 on Windows 10 1809 to 1909) when it is created. Every
change notification re-applies it to all CGPUI windows on the UI thread. The
call has no effect on Windows builds older than 1809.

DPI awareness: the Win32 application constructor and the window factory call
`SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)`
once per process. If a host manifest or an earlier call already fixed the
awareness, the call fails with `ERROR_ACCESS_DENIED`; CGPUI treats that as
normal, sets `already_set_by_process`, and reports the awareness that is in
effect. With Per-Monitor V2, client coordinates are physical pixels and
`WindowState::scale` follows the monitor DPI; input is divided by that scale.

## Wayland sources

The Wayland application reads `org.freedesktop.portal.Settings` from
xdg-desktop-portal (`org.freedesktop.portal.Desktop`,
`/org/freedesktop/portal/desktop`) on the session bus. It opens its own
private libdbus connection the first time `system_appearance()` or
`set_system_appearance_callback()` is called, so applications that never ask
do not touch the bus. Each key is read with `ReadOne`; if the portal answers
`UnknownMethod` (Settings version 1), CGPUI switches to the deprecated `Read`,
whose reply wraps the value in one more variant.

| Value | Source |
| --- | --- |
| `mode` | `org.freedesktop.appearance` `color-scheme` (u): 1 = dark; 0 (no preference), 2 (prefer light), and unknown values = light |
| `accent_color` | `org.freedesktop.appearance` `accent-color` ((ddd) sRGB). Each component must be in [0, 1]; otherwise, or when the key is absent, `accent_color_available = false`. Alpha is reported as 1 |
| `high_contrast` | `org.freedesktop.appearance` `contrast` (u): 1 = high contrast; absent = false |
| `client_area_animation_enabled` | `org.gnome.desktop.interface` `enable-animations` (b) when the portal backend exposes it (GNOME and portals that forward GSettings); otherwise the default `true` |
| `transparency_effects_enabled` | always `false`: the portal has no such key. C3 (window backdrop) will set it from the compositor's blur protocol support |

Changes: CGPUI adds a match rule for `SettingChanged` from the portal before
the first read, so a change between subscribing and reading is not lost. The
Wayland event loop polls the D-Bus socket next to the display, wakeup, and
timer descriptors and dispatches it on the UI thread without blocking, so the
callback runs on the UI thread like every other platform callback.

| `SettingChanged` key | `source` |
| --- | --- |
| `org.freedesktop.appearance` `color-scheme` | `color_set` |
| `org.freedesktop.appearance` `accent-color` | `colorization` |
| `org.freedesktop.appearance` `contrast` | `high_contrast` |
| `org.gnome.desktop.interface` `enable-animations` | `client_area_animation` |

Other keys are ignored and do not call the callback. As on Windows, the
callback fires for each relevant signal and `changed` says whether the
snapshot differs from the previous one.

Portal unavailable: if the session bus cannot be reached or the first
`color-scheme` read fails with anything other than a portal `NotFound` error
(for example `ServiceUnknown` or a 2 s timeout), the snapshot is
`supported = false`, `backend = "wayland"`, light, no accent, and
`unsupported_reason` carries the D-Bus error. The runtime's
`system-appearance-monitor` diagnostic then has `supported = false`. The
decision is made once per application; a portal that starts later is not
picked up, and a dropped bus connection turns the snapshot unsupported.

WSLg (checked 2026-11-16 in the `archlinux` WSL2 distro, kernel
6.18.33.2-microsoft-standard-WSL2): the session bus exists
(`unix:path=/run/user/0/bus`), but no `xdg-desktop-portal` package is
installed and nothing provides `org.freedesktop.portal.Desktop`.
`dbus-send --session ... org.freedesktop.portal.Settings.ReadOne` returns
`org.freedesktop.DBus.Error.ServiceUnknown: The name is not activatable`, so
CGPUI reports the unsupported snapshot there. Installing a portal backend in
the distro was not tried.

DPI awareness: Wayland has no process-wide switch; CGPUI scales each surface
itself. `dpi_awareness()` reports `backend = "wayland"`,
`declared_by_framework = true`, and `per_monitor_aware_v2` when the compositor
advertises both `wp_fractional_scale_manager_v1` and `wp_viewporter`
(per-surface fractional scale), otherwise `per_monitor_aware` (the integer
`wl_output` scale applied as the `wl_surface` buffer scale).

Tests: `wayland_portal_settings_test` parses constructed ReadOne, Read, and
`SettingChanged` messages. `wayland_portal_appearance_monitor_test` starts a
private `dbus-daemon` with a fake portal and checks the missing-portal
result, the `ReadOne` and deprecated `Read` paths, and signal delivery through
the same poll and dispatch calls the event loop makes. It skips if
`dbus-daemon` is not installed. `wayland_dpi_awareness_test` uses the test
compositor. None of them talk to a real desktop portal.

## Other platforms and limits

- X11, macOS, and test applications inherit the base implementation:
  `supported = false`, `backend = "unsupported"`, light mode, no accent color,
  and an `unsupported_reason`. The callback setter accepts a callback but never
  calls it. DPI awareness reports `unsupported`.
- The Windows snapshot is read on demand. Values only change when Windows
  broadcasts one of the messages above; registry edits made without a
  broadcast are seen on the next query but do not notify.
- Toggling transparency effects is expected to arrive as `ImmersiveColorSet`.
  This and the delivery of `WM_DWMCOLORIZATIONCOLORCHANGED` to the hidden
  monitor window have not been observed with real settings changes; tests use
  synthetic messages.
- The dark title bar was verified by reading the DWM attribute back, not by
  looking at it.
- No WinRT `UISettings` is used, so accent shades (light/dark variants) are not
  available.
- Wayland: no real portal has been observed. Behavior against GNOME, KDE, or
  other portal backends, and with a real Wayland window open, is unverified;
  tests use a fake portal on a private bus. Wayland has no title bar color
  hook: decorations are compositor-drawn or, later, client-drawn (C4).
- Wayland reads the portal synchronously on the UI thread on first use, up to
  2 s per key if the portal is slow to activate.
