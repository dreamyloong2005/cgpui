#include "window_runtime_test_support.hpp"

#include <optional>
#include <span>
#include <string_view>
#include <vector>
#include <variant>

namespace {

struct RuntimePaletteAction {
  static constexpr std::string_view name = "typed.command.runtime";
};

struct ContextPaletteAction {
  static constexpr std::string_view name = "typed.command.context";
};

struct AppContextPaletteAction {
  static constexpr std::string_view name = "typed.command.app-context";
};

struct ViewPaletteAction {
  static constexpr std::string_view name = "typed.command.view";
};

struct FocusedPaletteAction {
  static constexpr std::string_view name = "typed.command.focused";
};

struct IgnoredPaletteAction {
  static constexpr std::string_view name = "typed.command.ignored";
};

RuntimeFixture* typed_command_fixture = nullptr;

void dispatch_typed_command_metadata_sequence() {
  auto& callback = typed_command_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 67,
      .action = cgpui::KeyAction::pressed});
}

std::optional<cgpui::CommandPaletteEntry> find_entry(
    std::span<const cgpui::CommandPaletteEntry> entries,
    std::string_view action_name) {
  for (const cgpui::CommandPaletteEntry& entry : entries) {
    if (entry.action_name == action_name) {
      return entry;
    }
  }
  return std::nullopt;
}

class TypedActionCommandMetadataView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    context.focus(focused_element_id);
    context.runtime.register_app_action<RuntimePaletteAction>(
        [this](const cgpui::WindowRuntimeContext&) {
          runtime_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_app_action<ContextPaletteAction>(
        [this](const cgpui::WindowRuntimeContext&) {
          context_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_app_action<AppContextPaletteAction>(
        [this](const cgpui::WindowRuntimeContext&) {
          app_context_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_view_action<ViewPaletteAction>(
        [this](const cgpui::WindowRuntimeContext&) {
          view_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_focused_element_action<FocusedPaletteAction>(
        focused_element_id,
        [this](const cgpui::WindowRuntimeContext&) {
          focused_count += 1;
          return cgpui::EventResult::consumed_event();
        });

    helper_entry =
        cgpui::command_palette_entry<RuntimePaletteAction>(
            cgpui::CommandPaletteEntry{
                .action_name = "stale.runtime.name",
                .title = "Runtime Command",
                .group = "Typed",
                .scope = cgpui::ActionScope::app,
            });
    context.runtime.register_command_palette_entry(helper_entry);
    context.register_command_palette_entry<ContextPaletteAction>(
        cgpui::CommandPaletteEntry{
            .action_name = "stale.context.name",
            .title = "Context Command",
            .group = "Typed",
            .scope = cgpui::ActionScope::app,
        });
    context.app_context()
        .register_command_palette_entry<AppContextPaletteAction>(
            cgpui::CommandPaletteEntry{
                .action_name = "stale.app-context.name",
                .title = "App Context Command",
                .group = "Typed",
                .scope = cgpui::ActionScope::app,
            });
    context.register_command_palette_entry<ViewPaletteAction>(
        cgpui::CommandPaletteEntry{
            .title = "View Command",
            .group = "Typed",
            .scope = cgpui::ActionScope::view,
            .view_id = context.view_id,
        });
    context.register_command_palette_entry<FocusedPaletteAction>(
        cgpui::CommandPaletteEntry{
            .title = "Focused Command",
            .group = "Typed",
            .scope = cgpui::ActionScope::focused_element,
            .element_id = focused_element_id,
        });
    context.register_command_palette_entry<IgnoredPaletteAction>(
        cgpui::CommandPaletteEntry{
            .title = "",
            .group = "Typed",
            .scope = cgpui::ActionScope::app,
        });

    entries.assign(
        context.command_palette_entries().begin(),
        context.command_palette_entries().end());
    typed_group = context.command_palette_entries_for_group("Typed");
    runtime_result = context.dispatch_command_palette_action(
        std::string(cgpui::action_name<RuntimePaletteAction>()));
    context_result = context.dispatch_command_palette_action(
        std::string(cgpui::action_name<ContextPaletteAction>()));
    app_context_result = context.dispatch_command_palette_action(
        std::string(cgpui::action_name<AppContextPaletteAction>()));
    view_result = context.dispatch_command_palette_entry(entries[3]);
    focused_result = context.dispatch_command_palette_entry(entries[4]);

    return cgpui::EventResult::consumed_event();
  }

  cgpui::ElementId focused_element_id{81};
  int runtime_count = 0;
  int context_count = 0;
  int app_context_count = 0;
  int view_count = 0;
  int focused_count = 0;
  cgpui::CommandPaletteEntry helper_entry;
  std::vector<cgpui::CommandPaletteEntry> entries;
  std::vector<cgpui::CommandPaletteEntry> typed_group;
  cgpui::ActionDispatchResult runtime_result{};
  cgpui::ActionDispatchResult context_result{};
  cgpui::ActionDispatchResult app_context_result{};
  cgpui::ActionDispatchResult view_result{};
  cgpui::ActionDispatchResult focused_result{};
};

int test_typed_action_command_metadata_binding() {
  RuntimeFixture fixture;
  TypedActionCommandMetadataView view;
  typed_command_fixture = &fixture;
  fixture.app.on_run = &dispatch_typed_command_metadata_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  typed_command_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (view.entries.size() != 5 || view.typed_group.size() != 5 ||
      runtime.command_palette_entries().size() != 5) {
    return 2;
  }
  if (view.helper_entry.action_name != RuntimePaletteAction::name ||
      view.helper_entry.title != "Runtime Command" ||
      view.helper_entry.group != "Typed") {
    return 3;
  }

  const auto context_entry = find_entry(
      view.entries,
      ContextPaletteAction::name);
  const auto app_context_entry = find_entry(
      view.entries,
      AppContextPaletteAction::name);
  const auto view_entry = find_entry(view.entries, ViewPaletteAction::name);
  const auto focused_entry =
      find_entry(view.entries, FocusedPaletteAction::name);
  if (!context_entry.has_value() || !app_context_entry.has_value() ||
      !view_entry.has_value() || !focused_entry.has_value()) {
    return 4;
  }
  if (view_entry->scope != cgpui::ActionScope::view ||
      !view_entry->view_id.has_value() ||
      *view_entry->view_id != cgpui::ViewId{1}) {
    return 5;
  }
  if (focused_entry->scope != cgpui::ActionScope::focused_element ||
      !focused_entry->element_id.has_value() ||
      *focused_entry->element_id != view.focused_element_id) {
    return 6;
  }
  if (find_entry(view.entries, IgnoredPaletteAction::name).has_value()) {
    return 7;
  }

  if (view.runtime_count != 1 || view.context_count != 1 ||
      view.app_context_count != 1 || view.view_count != 1 ||
      view.focused_count != 1) {
    return 8;
  }
  if (!view.runtime_result.handled || !view.context_result.handled ||
      !view.app_context_result.handled || !view.view_result.handled ||
      !view.focused_result.handled) {
    return 9;
  }
  if (!view.view_result.scope.has_value() ||
      *view.view_result.scope != cgpui::ActionScope::view ||
      !view.focused_result.scope.has_value() ||
      *view.focused_result.scope != cgpui::ActionScope::focused_element) {
    return 10;
  }

  return 0;
}

} // namespace

int main() {
  return test_typed_action_command_metadata_binding();
}
