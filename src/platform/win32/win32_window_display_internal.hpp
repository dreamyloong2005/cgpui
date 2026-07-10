#pragma once

struct Win32WindowDisplayCommandState {
  bool fullscreen = false;
  bool windowed_rect_valid = false;
  LONG_PTR windowed_style = 0;
  LONG_PTR windowed_extended_style = 0;
  RECT windowed_rect{};
};
