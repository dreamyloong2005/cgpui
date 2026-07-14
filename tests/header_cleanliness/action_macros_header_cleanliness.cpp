#include "cgpui/ui/action_macros.hpp"

CGPUI_ACTIONS(HeaderAction)
CGPUI_ACTIONS_IN(header_scope, ScopedHeaderAction)

static_assert(cgpui::Action<HeaderAction>);
static_assert(cgpui::Action<header_scope::ScopedHeaderAction>);
static_assert(std::copyable<HeaderAction>);
static_assert(std::equality_comparable<header_scope::ScopedHeaderAction>);
static_assert(cgpui::action_name<HeaderAction>() == "HeaderAction");
static_assert(cgpui::action_name<header_scope::ScopedHeaderAction>() ==
              "header_scope::ScopedHeaderAction");

int main() { return 0; }
