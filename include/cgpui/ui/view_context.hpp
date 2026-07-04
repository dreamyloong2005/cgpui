#pragma once

#include "cgpui/ui/view_handle.hpp"

#include <optional>

namespace cgpui {

struct WindowRuntimeContext;

template <typename T>
class ViewContextCapability {
 public:
  constexpr ViewContextCapability() = default;
  constexpr explicit ViewContextCapability(
      const WindowRuntimeContext& context)
      : context_(&context) {}

  [[nodiscard]] ViewId view_id() const;
  [[nodiscard]] ViewHandle<T> view() const;
  [[nodiscard]] WeakViewHandle<T> weak_view() const;
  [[nodiscard]] std::optional<ViewHandle<T>> upgrade(
      WeakViewHandle<T> view) const;
  [[nodiscard]] const T* read(ViewHandle<T> view) const;
  [[nodiscard]] const T* current() const;

 private:
  const WindowRuntimeContext* context_ = nullptr;
};

} // namespace cgpui
