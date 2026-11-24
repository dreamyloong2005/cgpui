#pragma once
#include "dx12_command_reuse_signature_internal.hpp"
#include <memory>

namespace cgpui {
struct Dx12BundleReceipt { std::size_t group = 0, local = 0; };
class Dx12CommandBundle {
 public:
  static Result<std::unique_ptr<Dx12CommandBundle>> create(
      Dx12State&, Dx12ReuseScene, std::span<const Dx12DrawCommand>);
  ~Dx12CommandBundle();
  [[nodiscard]] const Dx12ReuseScene& scene() const noexcept;
  void execute(Dx12State&);
 private:
  struct Impl;
  explicit Dx12CommandBundle(Dx12State&, Dx12ReuseScene);
  Result<void> record(Dx12State&, std::span<const Dx12DrawCommand>);
  std::unique_ptr<Impl> impl_;
};
} // namespace cgpui
