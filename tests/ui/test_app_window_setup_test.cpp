#include "cgpui/ui/test_app.hpp"

#include <memory>

namespace {

class SetupView final : public cgpui::View {
 public:
  explicit SetupView(int value) : value(value) {}

  void paint(cgpui::PaintList&, cgpui::Size) override {}

  int value = 0;
};

bool equal(cgpui::Size lhs, cgpui::Size rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

} // namespace

int main() {
  cgpui::TestApp app;

  cgpui::TestAppWindow first = app.open_window(
      cgpui::WindowOptions{}.title("First Test Window").size(320.0F, 180.0F),
      std::make_unique<SetupView>(17));
  cgpui::TestAppWindow second = app.open_window(
      cgpui::WindowOptions{}.title("Second Test Window").size(640.0F, 360.0F),
      std::make_unique<SetupView>(29));

  if (!first.active() || !second.active()) return 1;
  if (first.runtime_id().value == 0 || second.runtime_id().value == 0 ||
      first.runtime_id() == second.runtime_id()) {
    return 2;
  }
  if (first.root_view_id().value == 0 || second.root_view_id().value == 0 ||
      first.root_view_id() == second.root_view_id()) {
    return 3;
  }
  if (first.descriptor().title != "First Test Window" ||
      !equal(first.viewport_size(), {320.0F, 180.0F}) ||
      first.scale().value != 1.0F) {
    return 4;
  }
  SetupView* first_view = first.root_view_as<SetupView>();
  const SetupView* second_view = second.root_view_as<SetupView>();
  if (first_view == nullptr || second_view == nullptr ||
      first_view->value != 17 || second_view->value != 29) {
    return 5;
  }
  const auto found = app.window(first.runtime_id());
  if (app.window_count() != 2 || !found ||
      found->runtime_id() != first.runtime_id()) {
    return 6;
  }
  auto missing = app.try_open_window(cgpui::WindowOptions{}, {});
  if (missing || missing.error().code != cgpui::ErrorCode::invalid_argument ||
      app.window_count() != 2) {
    return 7;
  }
  if (app.window(cgpui::WindowRuntimeId{1}).has_value()) return 8;
  return 0;
}
