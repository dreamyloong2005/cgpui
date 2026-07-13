#include "cgpui/ui/test_app.hpp"

#include <type_traits>

static_assert(std::is_move_constructible_v<cgpui::TestApp>);
static_assert(!std::is_copy_constructible_v<cgpui::TestApp>);
static_assert(std::is_copy_constructible_v<cgpui::TestAppWindow>);

int main() { return 0; }
