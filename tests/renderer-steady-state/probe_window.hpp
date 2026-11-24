#pragma once
#include "probe_internal.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace renderer_probe {
class Window {
 public:
  Window();
  ~Window();
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
  [[nodiscard]] cgpui::Result<void> check() const;
  [[nodiscard]] cgpui::Result<void> pump(bool verify_display = true) const;
  [[nodiscard]] cgpui::Result<void> wait(Clock::time_point deadline) const;
  [[nodiscard]] cgpui::RenderSurfaceDescriptor descriptor() const;
  [[nodiscard]] cgpui::Result<void> lock_display(Report& report);
  [[nodiscard]] cgpui::Result<void> check_display() const;
 private:
  static LRESULT CALLBACK procedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
  [[nodiscard]] DisplayFacts display_facts() const;
  HWND hwnd_ = nullptr;
  HINSTANCE instance_ = nullptr;
  HANDLE serial_ = nullptr;
  bool serial_owned_ = false;
  std::wstring class_name_;
  std::string setup_error_;
  std::uint64_t display_event_epoch_ = 0;
  Report* report_ = nullptr;
};
} // namespace renderer_probe
