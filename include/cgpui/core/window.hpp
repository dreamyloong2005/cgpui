#pragma once

#include "cgpui/core/geometry.hpp"

#include <cstdint>
#include <cstddef>
#include <optional>
#include <string>

namespace cgpui {

struct ImeTextInputPlacement {
  Rect rect;
  std::optional<Rect> candidate_rect;
  std::size_t byte_offset = 0;
  std::string surrounding_text;
  std::size_t selection_anchor = 0;
  std::uint32_t content_hint = 0;
  std::uint32_t content_purpose = 0;
};

enum class ImeTextInputSupport {
  unsupported,
  available,
};

struct WindowChromeOptions {
  bool titlebar_visible = true;
  bool decorations = true;
  bool resizable = true;
  bool transparent_background = false;
};

struct WindowDescriptor {
  std::string title = "CGPUI";
  Size size{1280.0F, 720.0F};
  WindowChromeOptions chrome;
};

struct WindowState {
  Size framebuffer_size;
  DpiScale scale;
  bool close_requested = false;
  ImeTextInputSupport ime_text_input_support =
      ImeTextInputSupport::unsupported;
  std::optional<ImeTextInputPlacement> ime_text_input_placement;
};

} // namespace cgpui
