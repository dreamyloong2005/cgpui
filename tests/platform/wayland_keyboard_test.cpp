#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <optional>
#include <thread>
#include <variant>

namespace {

constexpr std::uint32_t expected_key = 30;

bool wait_for_run_finished(const std::atomic_bool& run_finished) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!run_finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return run_finished.load();
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("keyboard");
  if (!compositor.start()) {
    return 2;
  }

  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    return 3;
  }

  bool pressed = false;
  bool released = false;
  bool focused = false;
  bool blurred = false;
  bool key_pressed_with_shift = false;
  bool text_received = false;
  bool ime_preedit_received = false;
  bool ime_delete_surrounding_received = false;
  bool ime_commit_received = false;
  std::uint32_t ime_preedit_serial = 0;
  std::uint32_t ime_delete_surrounding_serial = 0;
  std::uint32_t ime_commit_serial = 0;
  std::int32_t ime_preedit_cursor_begin = 0;
  std::int32_t ime_preedit_cursor_end = 0;
  std::size_t ime_preedit_style_count = 0;
  std::uint32_t ime_preedit_style_offset = 0;
  std::uint32_t ime_preedit_style_length = 0;
  cgpui::ImePreeditStyleKind ime_preedit_style_kind =
      cgpui::ImePreeditStyleKind::highlight;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Keyboard Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* focus = std::get_if<cgpui::WindowFocused>(&event);
            focus != nullptr) {
          focused = focused || focus->focused;
          blurred = blurred || !focus->focused;
        }
        if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
            key != nullptr && key->key_code == expected_key) {
          pressed = pressed || key->action == cgpui::KeyAction::pressed;
          released = released || key->action == cgpui::KeyAction::released;
          if (key->action == cgpui::KeyAction::pressed) {
            key_pressed_with_shift = key->modifiers.shift &&
                !key->modifiers.control && !key->modifiers.alt &&
                !key->modifiers.super;
          }
        }
        if (const auto* text = std::get_if<cgpui::TextInput>(&event);
            text != nullptr && text->text == "A") {
          text_received = text->modifiers.shift &&
              !text->modifiers.control && !text->modifiers.alt &&
              !text->modifiers.super;
        }
        if (const auto* composition = std::get_if<cgpui::ImeComposition>(&event);
            composition != nullptr) {
          if (composition->phase == cgpui::ImeCompositionPhase::update &&
              composition->text == "draft") {
            ime_preedit_received = true;
            ime_preedit_serial = composition->serial;
            ime_preedit_cursor_begin = composition->preedit_cursor_begin;
            ime_preedit_cursor_end = composition->preedit_cursor_end;
            ime_preedit_style_count = composition->preedit_style_count;
            if (composition->preedit_style_count > 0) {
              ime_preedit_style_offset =
                  composition->preedit_styles[0].byte_offset;
              ime_preedit_style_length =
                  composition->preedit_styles[0].byte_length;
              ime_preedit_style_kind = composition->preedit_styles[0].kind;
            }
          }
          if (composition->phase == cgpui::ImeCompositionPhase::commit &&
              composition->text == "\xE4\xB8\xAD") {
            ime_commit_received = true;
            ime_commit_serial = composition->serial;
          }
        }
        if (const auto* delete_surrounding =
                std::get_if<cgpui::ImeDeleteSurroundingText>(&event);
            delete_surrounding != nullptr) {
          ime_delete_surrounding_received =
              delete_surrounding->before_length == 2 &&
              delete_surrounding->after_length == 1 &&
              delete_surrounding->modifiers.shift &&
              !delete_surrounding->modifiers.control &&
              !delete_surrounding->modifiers.alt &&
              !delete_surrounding->modifiers.super;
          if (ime_delete_surrounding_received) {
            ime_delete_surrounding_serial = delete_surrounding->serial;
          }
        }
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          (*app)->quit();
        }
        if (focused && pressed && released && key_pressed_with_shift &&
            text_received && ime_preedit_received &&
            ime_delete_surrounding_received && ime_commit_received &&
            blurred) {
          (*app)->quit();
        }
      });
  if (!window) {
    return 4;
  }
  if ((*window)->state().ime_text_input_support !=
      cgpui::ImeTextInputSupport::available) {
    return 14;
  }

  (*window)->set_ime_text_input_placement(cgpui::ImeTextInputPlacement{
      .rect =
          cgpui::Rect{
              .origin = {.x = 18.0F, .y = 28.0F},
              .size = {.width = 1.0F, .height = 16.0F}},
      .byte_offset = 1,
      .surrounding_text = "draft",
      .selection_anchor = 0,
      .content_hint = 2,
      .content_purpose = 3});
  const cgpui::WindowState placed_state = (*window)->state();
  if (placed_state.ime_text_input_support != cgpui::ImeTextInputSupport::available ||
      !placed_state.ime_text_input_placement.has_value() ||
      placed_state.ime_text_input_placement->byte_offset != 1 ||
      placed_state.ime_text_input_placement->surrounding_text != "draft" ||
      placed_state.ime_text_input_placement->selection_anchor != 0 ||
      placed_state.ime_text_input_placement->content_hint != 2 ||
      placed_state.ime_text_input_placement->content_purpose != 3 ||
      placed_state.ime_text_input_placement->rect.origin.x != 18.0F ||
      placed_state.ime_text_input_placement->rect.origin.y != 28.0F ||
      placed_state.ime_text_input_placement->rect.size.width != 1.0F ||
      placed_state.ime_text_input_placement->rect.size.height != 16.0F) {
    return 15;
  }

  (*window)->set_ime_text_input_placement(std::nullopt);
  const cgpui::WindowState cleared_state = (*window)->state();
  if (cleared_state.ime_text_input_support != cgpui::ImeTextInputSupport::available ||
      cleared_state.ime_text_input_placement.has_value()) {
    return 16;
  }

  (*window)->set_ime_text_input_placement(cgpui::ImeTextInputPlacement{
      .rect =
          cgpui::Rect{
              .origin = {.x = 22.0F, .y = 34.0F},
              .size = {.width = 2.0F, .height = 18.0F}},
      .byte_offset = 2,
      .surrounding_text = "az",
      .selection_anchor = 1,
      .content_hint = 512,
      .content_purpose = 13});

  std::atomic_bool run_finished{false};
  int run_result = -1;
  std::thread client_thread([&] {
    run_result = (*app)->run();
    run_finished.store(true);
  });

  compositor.request_keyboard_modifiers(true, false, false, false);
  compositor.request_keyboard_key(expected_key, true);
  compositor.request_keyboard_key(expected_key, false);
  compositor.request_text_input_enter();
  compositor.request_text_input_preedit("draft", 1, 3);
  compositor.request_text_input_delete_surrounding(2, 1);
  compositor.request_text_input_commit("\xE4\xB8\xAD");
  compositor.request_text_input_leave();
  compositor.request_keyboard_leave();

  if (!wait_for_run_finished(run_finished)) {
    compositor.request_close();
    wait_for_run_finished(run_finished);
    compositor.stop();
    if (client_thread.joinable()) {
      client_thread.join();
    }
    return 9;
  }

  if (client_thread.joinable()) {
    client_thread.join();
  }
  compositor.stop();

  if (run_result != 0) {
    return 5;
  }
  if (!compositor.wait_for_keyboard_key_sent()) {
    return 6;
  }
  if (!compositor.wait_for_keyboard_modifiers_sent()) {
    return 11;
  }
  if (!compositor.wait_for_text_input_enter_sent()) {
    return 17;
  }
  if (!compositor.wait_for_text_input_preedit_sent()) {
    return 18;
  }
  if (!compositor.wait_for_text_input_delete_surrounding_sent()) {
    return 25;
  }
  if (!compositor.wait_for_text_input_commit_sent()) {
    return 19;
  }
  if (!compositor.wait_for_text_input_leave_sent()) {
    return 20;
  }
  if (!compositor.wait_for_text_input_client_state_committed()) {
    return 21;
  }
  const cgpui::test::WaylandTextInputClientState text_input_state =
      compositor.text_input_client_state();
  if (!text_input_state.enabled ||
      text_input_state.surrounding_text != "az" ||
      text_input_state.cursor != 2 ||
      text_input_state.anchor != 1 ||
      text_input_state.content_hint != 512 ||
      text_input_state.content_purpose != 13 ||
      !text_input_state.cursor_rect.has_value() ||
      text_input_state.cursor_rect->x != 22 ||
      text_input_state.cursor_rect->y != 34 ||
      text_input_state.cursor_rect->width != 2 ||
      text_input_state.cursor_rect->height != 18) {
    return 22;
  }
  if (!pressed || !released) {
    return 7;
  }
  if (!focused) {
    return 8;
  }
  if (!blurred) {
    return 10;
  }
  if (!key_pressed_with_shift) {
    return 12;
  }
  if (!text_received) {
    return 13;
  }
  if (!ime_preedit_received) {
    return 23;
  }
  if (ime_preedit_serial != 1) {
    return 27;
  }
  if (ime_preedit_cursor_begin != 1) {
    return 30;
  }
  if (ime_preedit_cursor_end != 3) {
    return 31;
  }
  if (ime_preedit_style_count != 1) {
    return 32;
  }
  if (ime_preedit_style_offset != 0) {
    return 33;
  }
  if (ime_preedit_style_length != 5) {
    return 34;
  }
  if (ime_preedit_style_kind != cgpui::ImePreeditStyleKind::underline) {
    return 35;
  }
  if (!ime_delete_surrounding_received) {
    return 26;
  }
  if (ime_delete_surrounding_serial != 2) {
    return 28;
  }
  if (!ime_commit_received) {
    return 24;
  }
  if (ime_commit_serial != 3) {
    return 29;
  }

  return 0;
}
