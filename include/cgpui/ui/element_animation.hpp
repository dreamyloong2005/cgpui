#pragma once

#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/runtime_handles.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <vector>

namespace cgpui {

struct ElementAnimationStage;

struct ElementAnimationSnapshot {
  std::uint64_t scope_id = 0;
  ElementKey key;
  std::uint64_t elapsed_ms = 0;
  std::uint64_t duration_ms = 0;
  float linear_progress = 0.0F;
  float eased_progress = 0.0F;
  AnimationEasing easing = AnimationEasing::linear;
  std::size_t stage_index = 0;
  std::uint64_t iteration = 0;
  bool repeating = false;
  bool mounted = false;
  bool complete = false;
};

struct ElementAnimationFrameResult {
  std::size_t active_count = 0;
  std::size_t completed_count = 0;
  std::size_t unmounted_count = 0;
  std::uint64_t next_frame_delay_ms = 0;

  [[nodiscard]] bool requests_next_frame() const;
};

class ElementAnimationStateStore {
 public:
  ElementAnimationStateStore();
  ~ElementAnimationStateStore();

  ElementAnimationStateStore(const ElementAnimationStateStore&) = delete;
  ElementAnimationStateStore& operator=(
      const ElementAnimationStateStore&) = delete;
  ElementAnimationStateStore(ElementAnimationStateStore&&) noexcept;
  ElementAnimationStateStore& operator=(
      ElementAnimationStateStore&&) noexcept;

  void begin_frame(std::uint64_t scope_id, std::uint64_t now_ms);
  [[nodiscard]] ElementAnimationSnapshot resolve(
      const ElementKey& key,
      AnimationOptions options);
  [[nodiscard]] ElementAnimationSnapshot resolve(
      const ElementKey& key,
      std::span<const ElementAnimationStage> stages);
  [[nodiscard]] ElementAnimationFrameResult finish_frame(
      std::uint64_t scope_id);
  [[nodiscard]] bool contains(
      std::uint64_t scope_id,
      const ElementKey& key) const;
  [[nodiscard]] std::size_t size() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

using ElementAnimationCallback =
    std::function<void(Element&, const ElementAnimationSnapshot&)>;

class AnimationElement final : public Element {
 public:
  AnimationElement(
      ElementKey key,
      AnimationOptions options,
      AnyElement child,
      ElementAnimationCallback callback);
  AnimationElement(
      ElementKey key,
      std::vector<ElementAnimationStage> stages,
      AnyElement child,
      ElementAnimationCallback callback);
  ~AnimationElement() override;

  [[nodiscard]] Element* child();
  [[nodiscard]] const Element* child() const;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  void inherit_text_style(const Style& style) override;
  [[nodiscard]] ElementId hit_test(Point point) const override;
  void paint(PaintList& paint_list) const override;
  [[nodiscard]] bool focusable() const override;
  [[nodiscard]] AccessibilityRole accessibility_role() const override;
  [[nodiscard]] std::string accessibility_name() const override;
  [[nodiscard]] std::string accessibility_text() const override;
  [[nodiscard]] std::string accessibility_value() const override;
  [[nodiscard]] AccessibilityPatternState accessibility_patterns()
      const override;
  [[nodiscard]] EventResult handle_accessibility_action(
      const AccessibilityActionRequested& action,
      const ElementEventContext& context) override;
  void focus(const ElementFocusContext& context) override;
  void on_mount(const ElementLifecycleContext& context) override;
  void on_update(const ElementLifecycleContext& context) override;
  void on_unmount(const ElementLifecycleContext& context) override;
  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override;
  [[nodiscard]] int z_index() const override;
  [[nodiscard]] int layer() const override;

 private:
  ElementKey animation_key_;
  AnimationOptions options_;
  std::vector<ElementAnimationStage> stages_;
  AnyElement child_;
  ElementAnimationCallback callback_;
  bool sequence_ = false;
};

[[nodiscard]] AnyElement with_animation(
    ElementKey key,
    AnimationOptions options,
    AnyElement child,
    ElementAnimationCallback callback);

} // namespace cgpui
