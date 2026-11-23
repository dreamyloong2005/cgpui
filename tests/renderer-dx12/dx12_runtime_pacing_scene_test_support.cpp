#include "dx12_runtime_pacing_test_support.hpp"
#include <cmath>

namespace cgpui_test {
Dx12RuntimePacingScene::Dx12RuntimePacingScene(Dx12RuntimePacingWindow& window) : window_(window) {}
void Dx12RuntimePacingScene::paint(cgpui::PaintList& list, cgpui::Size viewport) {
  ++window_.paints;
  list.fill_rect({{0, 0}, viewport}, {0, 0, 1, 1});
  list.fill_rect({{8, 8}, {24, 24}}, {1, 0, 0, 1});
}
cgpui::EventResult Dx12RuntimePacingScene::handle_event(const cgpui::PlatformEvent& event,
    const cgpui::WindowRuntimeContext&) {
  if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
      key && key->key_code == VK_F8 && key->action == cgpui::KeyAction::pressed) {
    ++window_.keys;
    return cgpui::EventResult::consumed_event();
  }
  return cgpui::EventResult::unhandled();
}
bool dx12_runtime_pacing_pixels(const cgpui::WindowRuntimeContext& context,
    const Dx12RuntimePacingWindow& window) {
  const auto* pixels = context.renderer.last_frame_pixels();
  const auto* diagnostic = context.renderer.last_frame_diagnostic_snapshot();
  RECT client{};
  const float scale = context.scale.value;
  if (!pixels || !pixels->valid() || !diagnostic || !std::isfinite(scale) || scale <= 0 ||
      !GetClientRect(window.hwnd, &client) || pixels->width != window.width ||
      pixels->height != window.height || client.right != static_cast<LONG>(pixels->width) ||
      client.bottom != static_cast<LONG>(pixels->height) || !diagnostic->work.exact_match() ||
      diagnostic->submitted_draws.solid_rect_count != 2 || !diagnostic->dropped_resources.empty()) return false;
  const auto sample = [scale](float logical) { return static_cast<unsigned>(std::floor(logical * scale)); };
  return dx12_pixel(pixels, sample(16), sample(16), {255, 0, 0, 255}) &&
      dx12_pixel(pixels, sample(48), sample(48), {0, 0, 255, 255});
}
} // namespace cgpui_test
