#include "cgpui/prelude.hpp"

#include <concepts>
#include <string_view>
#include <type_traits>

namespace {

struct SaveAction {
  static constexpr std::string_view name = "file.save";
};

struct CloseWindowAction {
  static constexpr std::string_view name = "window.close";
};

struct MissingActionName {};

using SaveActionName = decltype(cgpui::action_name<SaveAction>());

static_assert(cgpui::Action<SaveAction>);
static_assert(cgpui::Action<const SaveAction>);
static_assert(cgpui::Action<CloseWindowAction>);
static_assert(!cgpui::Action<MissingActionName>);
static_assert(std::same_as<SaveActionName, std::string_view>);
static_assert(cgpui::action_name_v<SaveAction> == "file.save");
static_assert(cgpui::action_name<SaveAction>() == "file.save");
static_assert(cgpui::action_name<CloseWindowAction>() == "window.close");

} // namespace

int main() {
  return cgpui::action_name<SaveAction>() == "file.save" &&
                 cgpui::action_name<CloseWindowAction>() == "window.close"
             ? 0
             : 1;
}
