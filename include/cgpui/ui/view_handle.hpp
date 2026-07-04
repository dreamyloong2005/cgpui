#pragma once

#include "cgpui/ui/element_core.hpp"

namespace cgpui {

class WeakView {
 public:
  constexpr WeakView() = default;
  constexpr explicit WeakView(ViewId id) : id_(id) {}

  [[nodiscard]] constexpr ViewId id() const { return id_; }
  [[nodiscard]] constexpr bool empty() const { return id_.value == 0; }

  friend bool operator==(const WeakView&, const WeakView&) = default;

 private:
  ViewId id_;
};

template <typename T>
class WeakViewHandle {
 public:
  constexpr WeakViewHandle() = default;
  constexpr explicit WeakViewHandle(ViewId id) : id_(id) {}

  [[nodiscard]] constexpr ViewId id() const { return id_; }
  [[nodiscard]] constexpr bool empty() const { return id_.value == 0; }
  [[nodiscard]] constexpr WeakView untyped() const { return WeakView(id_); }

  friend bool operator==(
      const WeakViewHandle&,
      const WeakViewHandle&) = default;

 private:
  ViewId id_;
};

template <typename T>
class ViewHandle {
 public:
  constexpr ViewHandle() = default;
  constexpr explicit ViewHandle(ViewId id) : id_(id) {}

  [[nodiscard]] constexpr ViewId id() const { return id_; }
  [[nodiscard]] constexpr bool empty() const { return id_.value == 0; }

  [[nodiscard]] constexpr WeakViewHandle<T> downgrade() const {
    return WeakViewHandle<T>(id_);
  }

  template <typename Context>
  [[nodiscard]] const T* read(const Context& context) const {
    return context.template read_view<T>(*this);
  }

  friend bool operator==(const ViewHandle&, const ViewHandle&) = default;

 private:
  ViewId id_;
};

} // namespace cgpui
