# System appearance and DPI awareness

CGPUI reports the desktop's appearance settings (light or dark, accent color,
transparency effects, client-area animation, high contrast) and notifies the
application when Windows broadcasts a change. On Windows it also declares
Per-Monitor V2 DPI awareness before the first window exists, so windows get
real monitor DPI and `WM_DPICHANGED`.

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
an empty callback stops delivery). For each notification it records a
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

## Other platforms and limits

- Wayland, X11, macOS, and test applications inherit the base implementation:
  `supported = false`, `backend = "unsupported"`, light mode, no accent color.
  The callback setter accepts a callback but never calls it. DPI awareness
  reports `unsupported`.
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
