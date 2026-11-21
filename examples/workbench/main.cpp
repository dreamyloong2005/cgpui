// IDE workbench (C9): client title bar, a three-pane splitter (file tree |
// editor tabs | agent panel), a tree with a few hundred items, tabs with a
// dirty tab, a tooltip on a title-bar button and a context menu on the tree.
// Environment:
//   CGPUI_EXAMPLE_BACKDROP=1      Mica behind the window
//   CGPUI_WORKBENCH_THEME=dark    force dark (or light) instead of the system
//   CGPUI_WORKBENCH_BLUR=1        backdrop blur under the context menu
#include "cgpui/cgpui.hpp"
#include "cgpui/ui/workbench_widgets.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace {

using namespace cgpui;

constexpr float caption_height = 36.0F;
constexpr float button_width = 46.0F;
constexpr float header_height = 28.0F;
constexpr TooltipTargetId agent_button_tip = 1;

std::vector<TreeItem> folder_children(TreeItemId parent) {
  std::vector<TreeItem> items;
  if (parent == 1) {
    const char* folders[]{"app", "editor", "agent", "render", "text", "platform",
                          "settings", "search", "terminal", "git", "theme", "util"};
    for (TreeItemId index = 0; index < 12; ++index) {
      items.push_back({100 + index, folders[index], true});
    }
    return items;
  }
  for (TreeItemId index = 0; index < 25; ++index) {
    items.push_back({parent * 100 + index,
                     "module_" + std::to_string(index) + (index % 3 ? ".cpp" : ".hpp")});
  }
  return items;
}

std::vector<MenuItem> tree_menu() {
  return {MenuItem::action("file.new", "New File...", "Ctrl+N"),
          MenuItem::action("file.reveal", "Reveal in File Explorer", "Shift+Alt+R"),
          MenuItem::separator(),
          MenuItem::action("edit.copy_path", "Copy Path", "Shift+Alt+C"),
          MenuItem::action("edit.rename", "Rename", "F2"),
          MenuItem::action("edit.delete", "Delete", "Del", false),
          MenuItem::separator(),
          MenuItem::submenu("Open With", {MenuItem::action("open.text", "Text Editor"),
                                          MenuItem::action("open.hex", "Hex Editor")})};
}

class WorkbenchView final : public View {
 public:
  WorkbenchView()
      : splitter_(SplitterOptions{.name = "Workbench"},
                  {SplitterPaneOptions{.default_fraction = 0.2F, .min_size = 160.0F,
                                       .max_size = 480.0F, .collapsible = true},
                   SplitterPaneOptions{.default_fraction = 0.55F, .min_size = 320.0F},
                   SplitterPaneOptions{.default_fraction = 0.25F, .min_size = 220.0F,
                                       .collapsible = true}}),
        tree_(TreeViewOptions{.name = "Explorer"}),
        menu_(PopoverMenuOptions{.backdrop_blur =
                                     std::getenv("CGPUI_WORKBENCH_BLUR") != nullptr}) {
    tree_.on_event([this](const TreeViewEvent& event) {
      if (event.kind == TreeViewEventKind::children_requested) {
        tree_.set_children(event.item, folder_children(event.item));
      } else if (event.kind == TreeViewEventKind::context_menu_requested) {
        pending_menu_ = event.position;
      }
    });
    tree_.set_roots({{1, "codestudio-next", true}, {2, "docs", true}, {3, "tests", true}});
    tree_.expand(1);
    for (TreeItemId folder = 100; folder < 112; ++folder) tree_.expand(folder);
    tabs_.set_tabs({{1, "main.cpp"}, {2, "workbench.hpp", true}, {3, "agent_panel.cpp"},
                    {4, "tree_view.cpp"}, {5, "README.md"}, {6, "xmake.lua"}});
    tabs_.set_selected(1);
    tabs_.on_event([this](const TabStripEvent& event) { apply_tab_event(event); });
    splitter_.on_change([this](const SplitterChange& change) {
      // The app owns persistence; a real app would save finished ratios.
      if (change.finished) saved_fractions_.assign(change.fractions.begin(),
                                                   change.fractions.end());
    });
    menu_.on_activate([this](std::string_view id) { last_action_ = std::string(id); });
  }

  AnyElement render(ViewContext& context) override {
    theme_ = theme_for(context);
    viewport_ = context.viewport_size;
    const Rect body{{0, caption_height},
                    {viewport_.width, viewport_.height - caption_height}};
    splitter_.layout(body, context.scale);
    const Rect left = splitter_.pane_rect(0);
    const Rect middle = splitter_.pane_rect(1);
    tree_.layout(Rect{{left.origin.x, left.origin.y + header_height},
                      {left.size.width, left.size.height - header_height}},
                 context.scale);
    tabs_.layout(Rect{middle.origin, {middle.size.width, 34.0F}}, context.scale, theme_);
    tooltip_.set_target(agent_button_tip, agent_button(), "Toggle agent panel (Ctrl+Alt+B)");
    tooltip_.layout(viewport_, context.scale, theme_);
    menu_.layout(viewport_, context.scale, theme_);
    WidgetTreeBuilder builder(context);
    splitter_.append_elements(builder, builder.root());
    tree_.append_elements(builder, builder.root());
    tabs_.append_elements(builder, builder.root());
    builder.add(builder.root(),
                WidgetNode{.role = AccessibilityRole::button,
                           .name = "Toggle agent panel",
                           .bounds = agent_button(), .hit_testable = true},
                true);
    menu_.append_elements(builder, builder.root());
    tooltip_.append_elements(builder, builder.root());
    builder.install(context);
    return {};
  }

  void paint(PaintList& paint_list, Size viewport) override {
    paint_list.fill_rect(Rect{{0, 0}, viewport}, theme_.surface);
    paint_title_bar(paint_list, viewport);
    paint_panes(paint_list);
    tree_.paint(paint_list, theme_);
    tabs_.paint(paint_list, theme_);
    splitter_.paint(paint_list, theme_);
    menu_.paint(paint_list, theme_);
    tooltip_.paint(paint_list, theme_);
  }

  EventResult handle_event(const PlatformEvent& event,
                           const WindowRuntimeContext& context) override {
    if (const auto* moved = std::get_if<PointerMoved>(&event)) {
      pointer_ = moved->position;
      context.request_paint();
    }
    if (const EventResult result = menu_.handle_event(event, context); result.consumed) {
      return result;
    }
    if (const EventResult result = tooltip_.handle_event(event, context);
        result.consumed) {
      return result;
    }
    if (const auto* button = std::get_if<PointerButton>(&event);
        button != nullptr && !button->pressed && button->button == MouseButton::left &&
        contains(agent_button(), button->position)) {
      splitter_.set_collapsed(2, !splitter_.collapsed(2));
      context.request_render();
      return EventResult::consumed_event();
    }
    // Widgets never consume plain pointer moves, so each one sees hover.
    if (!splitter_.handle_event(event, context).consumed &&
        !tabs_.handle_event(event, context).consumed) {
      (void)tree_.handle_event(event, context);
    }
    if (pending_menu_.has_value()) {
      menu_.open_at(*pending_menu_, tree_menu(), context);
      pending_menu_.reset();
    }
    return EventResult::unhandled();
  }

  // Width the platform's own caption buttons keep at the left (macOS).
  // Returns true when it changed and the view must be painted again.
  bool set_leading_reserved(float width) {
    if (width == leading_reserved_) return false;
    leading_reserved_ = width;
    return true;
  }

 private:
  [[nodiscard]] bool native_buttons() const { return leading_reserved_ > 0.0F; }

  static WorkbenchTheme theme_for(const ViewContext& context) {
    SystemAppearance appearance = context.runtime.system_appearance();
    if (const char* forced = std::getenv("CGPUI_WORKBENCH_THEME")) {
      appearance.mode = std::string_view(forced) == "dark" ? SystemAppearanceMode::dark
                                                           : SystemAppearanceMode::light;
    }
    return workbench_theme(appearance);
  }

  [[nodiscard]] Rect agent_button() const {
    const float buttons = native_buttons() ? 0.0F : button_width * 3.0F;
    return Rect{{viewport_.width - buttons - 44.0F, 4.0F},
                {36.0F, caption_height - 8.0F}};
  }

  void apply_tab_event(const TabStripEvent& event) {
    std::vector<TabItem> tabs(tabs_.tabs().begin(), tabs_.tabs().end());
    if (event.kind == TabStripEventKind::select) {
      tabs_.set_selected(event.tab);
      return;
    }
    if (event.kind == TabStripEventKind::close) {
      tabs.erase(tabs.begin() + static_cast<long>(event.from_index));
    } else {
      const TabItem moved = tabs[event.from_index];
      tabs.erase(tabs.begin() + static_cast<long>(event.from_index));
      tabs.insert(tabs.begin() + static_cast<long>(event.to_index), moved);
    }
    tabs_.set_tabs(std::move(tabs));
  }

  void text(PaintList& paint_list, Point at, std::string_view value, Color color,
            float size, FontDescriptor font = {}) const {
    paint_list.fill_text(Rect{at, {4096.0F, size * 1.4F}}, color, value, font, size);
  }

  void paint_title_bar(PaintList& paint_list, Size viewport) const {
    const Rect bar{{0, 0}, {viewport.width, caption_height}};
    paint_list.fill_rect(bar, theme_.tab_strip);
    paint_list.add_window_control_region(WindowControlArea::caption, bar);
    text(paint_list, {14 + leading_reserved_, 9},
         "CodeStudio Next \xE2\x80\x94 cgpui workbench", theme_.text,
         theme_.font_size);
    const Rect agent = agent_button();
    paint_list.add_window_control_region(WindowControlArea::client, agent);
    if (contains(agent, pointer_)) {
      paint_list.fill_rounded_rect(agent, theme_.hover, BorderRadii::all(4.0F));
    }
    // Panel icon: an outlined window with its right third filled.
    const Rect icon{{agent.origin.x + 10, agent.origin.y + 7}, {16, 14}};
    paint_list.stroke_rounded_rect(icon, theme_.text_muted, BorderRadii::all(2.0F), 1.0F);
    paint_list.fill_rect(Rect{{icon.origin.x + 10, icon.origin.y}, {6, 14}},
                         splitter_.collapsed(2) ? theme_.text_muted : theme_.accent);
    paint_list.fill_rect(Rect{{0, caption_height - 1}, {viewport.width, 1}}, theme_.border);
    // macOS keeps its native traffic lights at the left of the caption row.
    if (native_buttons()) return;
    const WindowControlArea areas[]{WindowControlArea::minimize_button,
                                    WindowControlArea::maximize_button,
                                    WindowControlArea::close_button};
    for (int index = 0; index < 3; ++index) {
      const Rect bounds{{viewport.width - button_width * static_cast<float>(3 - index), 0},
                        {button_width, caption_height}};
      const bool hovered = contains(bounds, pointer_);
      if (hovered) {
        paint_list.fill_rect(bounds, index == 2 ? rgb(196, 43, 28) : theme_.hover);
      }
      const Point mid{bounds.origin.x + button_width / 2.0F, caption_height / 2.0F};
      if (index == 0) {
        paint_list.fill_rect(Rect{{mid.x - 5, mid.y}, {10, 1}}, theme_.text);
      } else if (index == 1) {
        paint_list.stroke_rounded_rect(Rect{{mid.x - 5, mid.y - 5}, {10, 10}}, theme_.text,
                                       BorderRadii{}, 1.0F);
      } else {
        text(paint_list, {mid.x - 5, mid.y - 10}, "\xC3\x97", theme_.text, 16.0F);
      }
      paint_list.add_window_control_region(areas[index], bounds);
    }
  }

  void paint_panes(PaintList& paint_list) const {
    const Rect left = splitter_.pane_rect(0);
    paint_list.fill_rect(Rect{left.origin, {left.size.width, header_height}},
                         theme_.surface_alt);
    text(paint_list, {left.origin.x + 12, left.origin.y + 6}, "EXPLORER", theme_.text_muted,
         11.0F);
    const Rect middle = splitter_.pane_rect(1);
    const Rect editor{{middle.origin.x, middle.origin.y + 34},
                      {middle.size.width, middle.size.height - 34}};
    paint_list.fill_rect(editor, theme_.tab_active);
    paint_list.push_clip(editor);
    const char* code[]{
        "#include \"cgpui/ui/workbench_widgets.hpp\"", "",
        "int main() {", "  cgpui::Splitter splitter{options, panes};",
        "  cgpui::TreeView tree;", "  cgpui::TabStrip tabs;",
        "  // dirty tab: workbench.hpp", "  return run(splitter, tree, tabs);", "}"};
    const FontDescriptor mono{.family = "monospace"};
    for (int line = 0; line < 9; ++line) {
      const float y = editor.origin.y + 12.0F + static_cast<float>(line) * 20.0F;
      text(paint_list, {editor.origin.x + 12, y}, std::to_string(line + 1),
           theme_.text_disabled, 13.0F, mono);
      text(paint_list, {editor.origin.x + 48, y}, code[line], theme_.text, 13.0F, mono);
    }
    paint_list.pop_clip();
    const Rect right = splitter_.pane_rect(2);
    if (right.size.width <= 0.0F) return;
    paint_list.push_clip(right);
    paint_list.fill_rect(right, theme_.surface_alt);
    text(paint_list, {right.origin.x + 12, right.origin.y + 6}, "AGENT", theme_.text_muted,
         11.0F);
    const char* messages[]{"Refactor the tree view to load children lazily.",
                           "Done: 3 files changed, tests pass.",
                           "Last menu action: "};
    for (int index = 0; index < 3; ++index) {
      const Rect bubble{{right.origin.x + 12, right.origin.y + 40 + index * 56.0F},
                        {right.size.width - 24, 44}};
      paint_list.fill_rounded_rect(bubble, index == 1 ? theme_.selection : theme_.surface,
                                   BorderRadii::all(8.0F));
      std::string message = messages[index];
      if (index == 2) message += last_action_.empty() ? "none" : last_action_;
      text(paint_list, {bubble.origin.x + 10, bubble.origin.y + 13}, message, theme_.text,
           12.0F);
    }
    paint_list.pop_clip();
  }

  Splitter splitter_;
  TreeView tree_;
  TabStrip tabs_;
  Tooltip tooltip_;
  PopoverMenu menu_;
  WorkbenchTheme theme_ = workbench_theme(SystemAppearance{});
  Size viewport_;
  Point pointer_{-1, -1};
  float leading_reserved_ = 0.0F;
  std::optional<Point> pending_menu_;
  std::vector<float> saved_fractions_;
  std::string last_action_;
};

} // namespace

int main() {
  auto app = Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }
  WorkbenchView view;
  AppRunnerOptions options;
  options.window = WindowOptions{}
                       .title("CGPUI workbench")
                       .size(Size{1280.0F, 800.0F})
                       .title_bar(WindowTitleBarMode::client)
                       .backdrop(std::getenv("CGPUI_EXAMPLE_BACKDROP") != nullptr
                                     ? WindowBackdrop::mica
                                     : WindowBackdrop::none)
                       .to_descriptor();
  options.setup_context = [&view](AppContext& app_context) {
    app_context.runtime.set_after_frame_callback([&view](const ViewContext& context) {
      if (view.set_leading_reserved(context.platform_window.frame_metrics()
                                        .caption_buttons_leading_reserved_width)) {
        context.request_paint();
        return;
      }
      if (std::getenv("CGPUI_EXIT_AFTER_FIRST_FRAME") != nullptr) {
        context.application.quit();
      }
    });
  };
  return app->run(
      view,
      [](const RenderSurfaceDescriptor& descriptor) { return create_renderer(descriptor); },
      options);
}
