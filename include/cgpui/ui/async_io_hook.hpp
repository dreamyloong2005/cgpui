#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/ui/runtime_ids.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

namespace cgpui {

class WindowRuntime;
struct WindowRuntimeContext;

namespace detail {
struct AsyncIoHookState;
}

using AsyncIoPayload = std::vector<std::byte>;
using AsyncIoResult = Result<AsyncIoPayload>;
using AsyncIoCompletionCallback = std::function<void(
    const WindowRuntimeContext&,
    const AsyncIoResult&)>;

class AsyncIoHook {
 public:
  AsyncIoHook() = default;

  [[nodiscard]] AsyncIoId id() const { return id_; }
  [[nodiscard]] bool active() const;
  [[nodiscard]] bool complete() const;
  [[nodiscard]] bool cancelled() const;
  [[nodiscard]] bool notify(AsyncIoPayload payload) const;
  [[nodiscard]] bool notify_error(Error error) const;
  [[nodiscard]] bool cancel() const;

 private:
  friend class WindowRuntime;

  AsyncIoHook(
      AsyncIoId id,
      std::shared_ptr<detail::AsyncIoHookState> state)
      : id_(id), state_(std::move(state)) {}

  AsyncIoId id_{};
  std::shared_ptr<detail::AsyncIoHookState> state_;
};

} // namespace cgpui
