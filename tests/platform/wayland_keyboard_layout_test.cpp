#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <string>
#include <thread>
#include <variant>
#include <vector>

namespace {

bool wait_for_count(const std::atomic_int& count, int expected) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (count.load() < expected && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return count.load() >= expected;
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("keyboard-layout");
  if (!compositor.start()) {
    return 1;
  }
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 2;
  }

  std::atomic_int key_count{0};
  std::atomic_int text_count{0};
  std::atomic_int phase{0};
  std::atomic_bool modifiers_match{true};
  std::mutex text_mutex;
  std::vector<std::string> texts;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Keyboard Layout",
          .size = {320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event)) {
          const bool expected = phase.load() == 0
              ? key->modifiers.shift && !key->modifiers.control &&
                    !key->modifiers.alt && !key->modifiers.super
              : key->modifiers.shift && key->modifiers.control &&
                    key->modifiers.alt && !key->modifiers.super;
          modifiers_match.store(modifiers_match.load() && expected);
          key_count.fetch_add(1);
        } else if (const auto* text = std::get_if<cgpui::TextInput>(&event)) {
          {
            std::lock_guard lock(text_mutex);
            texts.push_back(text->text);
          }
          text_count.fetch_add(1);
        } else if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          (*app)->quit();
        }
      });
  if (!window) {
    return 3;
  }

  std::atomic_bool run_finished{false};
  int run_result = -1;
  std::thread client_thread([&] {
    run_result = (*app)->run();
    run_finished.store(true);
  });
  const auto finish = [&](int result) {
    if (!run_finished.load()) {
      (*app)->quit();
    }
    compositor.stop();
    if (client_thread.joinable()) {
      client_thread.join();
    }
    return result;
  };

  compositor.request_keyboard_modifier_masks(1U, 0U, 0U, 1U);
  compositor.request_keyboard_key(30, true);
  compositor.request_keyboard_key(30, false);
  if (!wait_for_count(key_count, 2) || !wait_for_count(text_count, 1)) {
    return finish(4);
  }
  {
    std::lock_guard lock(text_mutex);
    if (!modifiers_match.load()) {
      return finish(5);
    }
    if (texts != std::vector<std::string>{"Q"}) {
      std::fprintf(
          stderr,
          "unexpected first layout text: %s\n",
          texts.empty() ? "<none>" : texts.front().c_str());
      return finish(6);
    }
  }

  phase.store(1);
  compositor.request_keyboard_modifier_masks(1U, 4U, 8U, 1U);
  compositor.request_keyboard_key(30, true);
  compositor.request_keyboard_key(30, false);
  if (!wait_for_count(key_count, 4) || !wait_for_count(text_count, 2) ||
      !modifiers_match.load()) {
    return finish(7);
  }

  compositor.request_keyboard_keymap_reload();
  if (!compositor.wait_for_keyboard_keymap_sent_count(2)) {
    return finish(8);
  }
  phase.store(2);
  compositor.request_keyboard_key(30, true);
  compositor.request_keyboard_key(30, false);
  if (!wait_for_count(key_count, 6) || !wait_for_count(text_count, 3)) {
    return finish(9);
  }

  compositor.request_close();
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!run_finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  if (!run_finished.load() || run_result != 0 || !modifiers_match.load()) {
    return finish(10);
  }
  {
    std::lock_guard lock(text_mutex);
    if (texts.size() != 3 || texts[0] != "Q" || texts[1] != texts[2]) {
      std::fprintf(
          stderr,
          "unexpected reloaded layout text: %s\n",
          texts.size() < 3 ? "<none>" : texts[2].c_str());
      return finish(11);
    }
  }
  return finish(0);
}
