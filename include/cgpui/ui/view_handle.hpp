#pragma once

#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/runtime_handles.hpp"

#include <cstdint>

namespace cgpui {

class WeakView {
 public:
  constexpr WeakView() = default;
  constexpr explicit WeakView(ViewId id) : id_(id) {}
  constexpr WeakView(ViewId id, std::uintptr_t context_token)
      : id_(id), context_token_(context_token) {}

  [[nodiscard]] constexpr ViewId id() const { return id_; }
  [[nodiscard]] constexpr std::uintptr_t context_token() const {
    return context_token_;
  }
  [[nodiscard]] constexpr bool empty() const { return id_.value == 0; }
  [[nodiscard]] constexpr bool matches_context(
      std::uintptr_t context_token) const {
    return context_token_ == 0 || context_token_ == context_token;
  }

  friend bool operator==(const WeakView&, const WeakView&) = default;

 private:
  ViewId id_;
  std::uintptr_t context_token_ = 0;
};

template <typename T>
class WeakViewHandle {
 public:
  constexpr WeakViewHandle() = default;
  constexpr explicit WeakViewHandle(ViewId id) : id_(id) {}
  constexpr WeakViewHandle(ViewId id, std::uintptr_t context_token)
      : id_(id), context_token_(context_token) {}

  [[nodiscard]] constexpr ViewId id() const { return id_; }
  [[nodiscard]] constexpr std::uintptr_t context_token() const {
    return context_token_;
  }
  [[nodiscard]] constexpr bool empty() const { return id_.value == 0; }
  [[nodiscard]] constexpr bool matches_context(
      std::uintptr_t context_token) const {
    return context_token_ == 0 || context_token_ == context_token;
  }
  [[nodiscard]] constexpr WeakView untyped() const {
    return WeakView(id_, context_token_);
  }

  friend bool operator==(
      const WeakViewHandle&,
      const WeakViewHandle&) = default;

 private:
  ViewId id_;
  std::uintptr_t context_token_ = 0;
};

template <typename T>
class ViewHandle {
 public:
  constexpr ViewHandle() = default;
  constexpr explicit ViewHandle(ViewId id) : id_(id) {}
  constexpr ViewHandle(ViewId id, std::uintptr_t context_token)
      : id_(id), context_token_(context_token) {}

  [[nodiscard]] constexpr ViewId id() const { return id_; }
  [[nodiscard]] constexpr std::uintptr_t context_token() const {
    return context_token_;
  }
  [[nodiscard]] constexpr bool empty() const { return id_.value == 0; }
  [[nodiscard]] constexpr bool matches_context(
      std::uintptr_t context_token) const {
    return context_token_ == 0 || context_token_ == context_token;
  }

  [[nodiscard]] constexpr WeakViewHandle<T> downgrade() const {
    return WeakViewHandle<T>(id_, context_token_);
  }

  template <typename Context>
  [[nodiscard]] const T* read(const Context& context) const {
    return context.template read_view<T>(*this);
  }

  template <typename Context, typename Observer>
  bool observe(const Context& context, Observer&& observer) const;

  template <typename Context, typename Observer>
  [[nodiscard]] Subscription observe_subscription(
      const Context& context,
      Observer&& observer) const;

  friend bool operator==(const ViewHandle&, const ViewHandle&) = default;

 private:
  ViewId id_;
  std::uintptr_t context_token_ = 0;
};

} // namespace cgpui
