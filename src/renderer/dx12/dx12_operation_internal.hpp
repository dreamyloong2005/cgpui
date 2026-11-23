#pragma once

#include <mutex>

namespace cgpui {
struct Dx12State;
class Dx12OperationGuard {
 public:
  explicit Dx12OperationGuard(Dx12State& state);
  ~Dx12OperationGuard();
  Dx12OperationGuard(const Dx12OperationGuard&) = delete;
  Dx12OperationGuard& operator=(const Dx12OperationGuard&) = delete;
 private:
  std::unique_lock<std::recursive_mutex> lock_;
  Dx12State& state_;
};
} // namespace cgpui
