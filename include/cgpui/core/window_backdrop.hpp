#pragma once

namespace cgpui {

// System material drawn behind a window. Each backend maps what it can:
// Windows uses DWM system backdrops (blur maps to acrylic); Wayland uses a
// compositor blur protocol (mica, mica_alt, and acrylic map to blur).
enum class WindowBackdrop {
  none,
  mica,
  mica_alt,
  acrylic,
  blur,
};

} // namespace cgpui
