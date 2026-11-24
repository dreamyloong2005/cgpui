#pragma once
#include "dx12_native_internal.hpp"
#include <cstdint>
#include <memory>
#include <span>

namespace cgpui {
struct Dx12State;
enum class Dx12CommandEventKind {
  parent_reset, parent_closed, parent_released, parent_target, scissor,
  bundle_created, bundle_closed, bundle_reset, bundle_released,
  pipeline, root, constants, topology, vertex, index, heaps, table,
  draw, indexed_draw, descriptor_write, execute_bundle,
  producer_execute, tail_execute, completed, removed, frozen_released
};
struct Dx12CommandEvent {
  Dx12CommandEventKind kind{};
  ID3D12GraphicsCommandList* list = nullptr;
  ID3D12CommandAllocator* allocator = nullptr;
  void* object = nullptr;
  void* object2 = nullptr;
  std::uint64_t parent = 0, a = 0, b = 0, c = 0, d = 0;
  D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  HRESULT result = S_OK;
};
// Private, state-owned test attachment. Preparation handles/latches its own errors.
// No callback occurs between producer Execute and submitted ownership marking.
class Dx12CommandVerifier {
 public:
  virtual ~Dx12CommandVerifier() = default;
  virtual void prepare(Dx12State&, ID3D12Resource&, std::uint64_t parent) noexcept = 0;
  virtual ID3D12CommandList* prepared_list() const noexcept = 0;
  virtual void completed(Dx12State&, std::uint64_t parent, std::uint64_t ticket) noexcept = 0;
  virtual void retired(bool removed) noexcept = 0;
 protected:
  [[nodiscard]] bool submission_pending() const noexcept;
 private:
  friend class Dx12CommandObservation;
  bool submitted_ = false;
};
class Dx12CommandObservation {
 public:
  explicit Dx12CommandObservation(std::size_t capacity);
  ~Dx12CommandObservation();
  void record(Dx12CommandEvent event) noexcept;
  void parent_reset(ID3D12GraphicsCommandList&, ID3D12CommandAllocator&) noexcept;
  ID3D12CommandList* prepare_tail(Dx12State&, ID3D12Resource&) noexcept;
  void tail_executed() noexcept;
  void completed(Dx12State&, std::uint64_t ticket) noexcept;
  void retire(bool removed) noexcept;
  [[nodiscard]] std::uint64_t parent_epoch() const noexcept;
  [[nodiscard]] std::span<const Dx12CommandEvent> events() const noexcept;
  [[nodiscard]] bool overflowed() const noexcept;
  std::unique_ptr<Dx12CommandVerifier> verifier;
 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
void dx12_observe_command(Dx12State&, Dx12CommandEvent) noexcept;
} // namespace cgpui
