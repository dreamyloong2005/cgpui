#include "cgpui/app/app_facade.hpp"
#include "cgpui/app/application.hpp"
#include "cgpui/app/window.hpp"
#include "cgpui/app/app.hpp"

#include <type_traits>

int main() {
  static_assert(std::is_move_constructible_v<cgpui::Application>);
  static_assert(!std::is_copy_constructible_v<cgpui::Application>);
  static_assert(std::is_copy_constructible_v<cgpui::App>);
  static_assert(std::is_copy_constructible_v<cgpui::Window>);
  return 0;
}
