#pragma once

#include "dx12_pacing_control_test_support.hpp"
#include "cgpui/platform/platform_application.hpp"
#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/runtime_rendering.hpp"
#include <array>
#include <chrono>
#include <functional>

namespace cgpui_test {
struct Dx12RuntimePacingWindow {
  Dx12PacingControlApi api; // Synthetic poll only; every configuration/handle remains native.
  HWND hwnd = nullptr;
  cgpui::RendererBackendInfo initial;
  unsigned width = 0, height = 0;
  unsigned begins = 0, pending = 0, errors = 0, captures = 0;
  unsigned resizes = 0, paints = 0, successes = 0, keys = 0, key_routes = 0, closes = 0;
};

class Dx12RuntimePacingScene final : public cgpui::View {
 public:
  explicit Dx12RuntimePacingScene(Dx12RuntimePacingWindow&);
  void paint(cgpui::PaintList&, cgpui::Size) override;
  cgpui::EventResult handle_event(const cgpui::PlatformEvent&,
                                const cgpui::WindowRuntimeContext&) override;
 private:
  Dx12RuntimePacingWindow& window_;
};

struct Dx12RuntimePacingFixture {
  using Clock = std::chrono::steady_clock;
  cgpui::PlatformApplication& application;
  std::array<Dx12RuntimePacingWindow, 2> windows;
  std::array<cgpui::WindowRuntimeId, 2> ids{};
  std::function<void(const cgpui::WindowRuntimeContext&)> on_tick;
  std::function<void(const cgpui::WindowRuntimeContext&)> on_frame;
  Clock::time_point started = Clock::now();
  unsigned factories = 0, errors = 0, user_ticks = 0;
  bool failed = false, completed = false;

  bool require(bool condition, const char* message);
  unsigned index(cgpui::WindowRuntimeId) const;
  void setup(cgpui::AppContext&);
  void after_frame(const cgpui::WindowRuntimeContext&);
  cgpui::Result<std::unique_ptr<cgpui::Renderer>> create(const cgpui::RenderSurfaceDescriptor&);
  int run(const char* title);
  bool pending_without_success() const;
};

cgpui::Result<std::unique_ptr<cgpui::Renderer>> dx12_runtime_pacing_capture(
    const cgpui::RenderSurfaceDescriptor&, Dx12RuntimePacingWindow&);
bool dx12_runtime_pacing_pixels(const cgpui::WindowRuntimeContext&, const Dx12RuntimePacingWindow&);
} // namespace cgpui_test
