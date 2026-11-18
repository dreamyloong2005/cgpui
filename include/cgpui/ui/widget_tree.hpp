#pragma once

#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/element_tree.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace cgpui {

struct WindowRuntimeContext;

// One routing / focus / accessibility node of a workbench widget. Bounds are
// window coordinates in logical pixels; widgets paint themselves.
struct WidgetNode {
  AccessibilityRole role = AccessibilityRole::generic;
  std::string name;
  std::string value;
  AccessibilityPatternState patterns;
  Rect bounds;
  bool focusable = false;
  bool hit_testable = false;
  bool enabled = true;
  int z_order = 0;
  // Applied with WindowRuntime::set_element_cursor by install().
  std::optional<CursorShape> cursor;
};

class WidgetNodeElement final : public Element {
 public:
  explicit WidgetNodeElement(WidgetNode node);

  [[nodiscard]] const WidgetNode& node() const;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  [[nodiscard]] ElementId hit_test(Point point) const override;
  [[nodiscard]] bool focusable() const override;
  [[nodiscard]] int z_index() const override;
  [[nodiscard]] AccessibilityRole accessibility_role() const override;
  [[nodiscard]] std::string accessibility_name() const override;
  [[nodiscard]] std::string accessibility_value() const override;
  [[nodiscard]] AccessibilityPatternState accessibility_patterns()
      const override;

 private:
  WidgetNode node_;
};

// Root of a widget tree: hit-tests the tree's nodes (children before
// parents, higher z_order first, later siblings first).
class WidgetRootElement final : public Element {
 public:
  explicit WidgetRootElement(Size viewport);

  void attach(const ElementTree* tree);
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  [[nodiscard]] ElementId hit_test(Point point) const override;

 private:
  [[nodiscard]] ElementId hit_test_children(ElementId id, Point point) const;

  Size viewport_;
  const ElementTree* tree_ = nullptr;
};

// Builds the window element tree for workbench widgets. Nodes marked
// `stable` get their ids first, so their ids do not move when the number of
// rows, tabs or menu items changes; focus, pointer capture and cursors are
// keyed by those ids.
class WidgetTreeBuilder {
 public:
  using NodeHandle = std::size_t;

  explicit WidgetTreeBuilder(
      Size viewport,
      std::optional<ElementId> focused_element = std::nullopt);
  explicit WidgetTreeBuilder(const WindowRuntimeContext& context);

  [[nodiscard]] NodeHandle root() const;
  [[nodiscard]] Size viewport() const;
  [[nodiscard]] std::optional<ElementId> focused_element() const;
  NodeHandle add(
      NodeHandle parent,
      WidgetNode node,
      bool stable = false,
      ElementId* id_out = nullptr);
  // Moves keyboard focus to `*id` once ids are assigned (install()).
  void request_focus(const ElementId* id);
  [[nodiscard]] std::unique_ptr<ElementTree> finish();
  // finish(), then WindowRuntime::set_element_tree for the root window,
  // cursors and the pending focus request.
  void install(const WindowRuntimeContext& context);

 private:
  struct Pending {
    NodeHandle parent = 0;
    WidgetNode node;
    bool stable = false;
    ElementId* id_out = nullptr;
  };

  Size viewport_;
  std::optional<ElementId> focused_element_;
  std::vector<Pending> pending_;
  std::vector<std::pair<ElementId, CursorShape>> cursors_;
  const ElementId* focus_request_ = nullptr;
};

} // namespace cgpui
