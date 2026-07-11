#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <thread>
#include <variant>

#include <unistd.h>

namespace {

void write_u32(std::ofstream& output, std::uint32_t value) {
  const char bytes[]{
      static_cast<char>(value),
      static_cast<char>(value >> 8U),
      static_cast<char>(value >> 16U),
      static_cast<char>(value >> 24U),
  };
  output.write(bytes, sizeof(bytes));
}

bool write_cursor(const std::filesystem::path& path) {
  std::ofstream output(path, std::ios::binary);
  if (!output) return false;
  constexpr std::uint32_t image_type = 0xfffd0002U;
  constexpr std::uint32_t nominal_size = 24;
  write_u32(output, 0x72756358U);
  write_u32(output, 16);
  write_u32(output, 0x00010000U);
  write_u32(output, 1);
  write_u32(output, image_type);
  write_u32(output, nominal_size);
  write_u32(output, 28);
  write_u32(output, 36);
  write_u32(output, image_type);
  write_u32(output, nominal_size);
  write_u32(output, 1);
  write_u32(output, 4);
  write_u32(output, 4);
  write_u32(output, 1);
  write_u32(output, 2);
  write_u32(output, 0);
  for (int pixel = 0; pixel < 16; ++pixel) write_u32(output, 0xffffffffU);
  return output.good();
}

bool create_theme(const std::filesystem::path& root) {
  const auto cursors = root / "cgpui-test" / "cursors";
  std::filesystem::create_directories(cursors);
  for (std::string_view name : {"left_ptr", "xterm", "hand2"}) {
    if (!write_cursor(cursors / name)) return false;
  }
  return true;
}

bool wait_for_finished(const std::atomic_bool& finished) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return finished.load();
}

bool wait_for_scale(cgpui::PlatformWindow& window, float expected) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (window.state().scale.value != expected &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return window.state().scale.value == expected;
}

} // namespace

int main() {
  const auto theme_root = std::filesystem::temp_directory_path() /
      ("cgpui-cursor-theme-" + std::to_string(::getpid()));
  if (!create_theme(theme_root)) return 1;
  setenv("XCURSOR_PATH", theme_root.string().c_str(), 1);
  setenv("XCURSOR_THEME", "cgpui-test", 1);
  setenv("XCURSOR_SIZE", "24", 1);

  cgpui::test::WaylandTestCompositor compositor("cursor-theme");
  if (!compositor.start()) return 2;
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);
  auto app = cgpui::create_platform_application();
  if (!app) return 3;

  cgpui::PlatformWindow* platform_window = nullptr;
  std::atomic_int cursor_stage{0};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Cursor Theme Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        int expected_stage = 0;
        if (std::holds_alternative<cgpui::PointerMoved>(event) &&
            platform_window != nullptr &&
            cursor_stage.compare_exchange_strong(expected_stage, 1)) {
          platform_window->set_cursor(cgpui::CursorShape::text);
          platform_window->set_cursor(cgpui::CursorShape::pointing_hand);
          cursor_stage.store(2);
          (*app)->quit();
        }
      });
  if (!window) return 4;
  platform_window = window->get();

  std::atomic_bool finished{false};
  std::thread client([&] {
    (void)(*app)->run();
    finished.store(true);
  });
  compositor.request_fractional_scale(150);
  if (!compositor.wait_for_fractional_scale_sent() ||
      !wait_for_scale(**window, 1.25F)) {
    (*app)->quit();
  }
  compositor.request_pointer_move(12, 18);
  if (!wait_for_finished(finished)) (*app)->quit();
  if (client.joinable()) client.join();

  const bool applied = compositor.wait_for_pointer_cursor_set_count(3);
  const bool ready = compositor.wait_for_cursor_surface_ready();
  const auto cursor = compositor.cursor_surface_state();
  compositor.stop();
  window->reset();
  app->reset();
  std::filesystem::remove_all(theme_root);
  if (!applied || cursor_stage.load() != 2) return 5;
  if (!ready || !cursor.surface_set || !cursor.buffer_attached ||
      !cursor.surface_committed) return 6;
  if (cursor.hotspot_x != 0 || cursor.hotspot_y != 1 ||
      cursor.buffer_scale != 2) return 7;
  return 0;
}
