#pragma once

#include "dx12_draw_commands_internal.hpp"
#include "cgpui/renderer/renderer_frame_diagnostic_snapshot.hpp"
#include <chrono>
#include <cstddef>
#include <span>
#include <vector>

namespace cgpui {
struct Dx12State;

// A candidate for one successful present, not a cumulative device counter.
// Native recording reports work here; destruction alone never publishes it.
class Dx12FrameDiagnostics {
 public:
  Dx12FrameDiagnostics(Dx12State& state, std::span<const Dx12DrawCommand> draws,
      std::size_t command_count, std::uint64_t admission_call_ns = 0);
  ~Dx12FrameDiagnostics();
  Dx12FrameDiagnostics(const Dx12FrameDiagnostics&) = delete;
  Dx12FrameDiagnostics& operator=(const Dx12FrameDiagnostics&) = delete;
  void begin_draw(std::size_t group_index);
  void submitted(std::size_t local_resource_index = 0);
  void uploaded(bool glyph, std::size_t bytes);
  void finish_stage(RendererFrameTimingStage stage);
  // Diagnostic-only ledger publication: no cache aging or native-success claim.
  void publish();
  // Production calls only after every fallible native presentation stage passes.
  void complete_successful_frame();

 private:
  struct Group {
    bool submitted = false;
  };
  using Clock = std::chrono::steady_clock;
  void plan();
  void prepare_publication();
  void publish_prepared() noexcept;
  Dx12State& state_;
  std::span<const Dx12DrawCommand> draws_;
  RendererFrameDiagnosticSnapshot snapshot_;
  RendererFrameWork planned_work_;
  RendererFrameWork submitted_work_;
  std::vector<Group> groups_;
  std::vector<RendererFrameResource> planned_resources_;
  std::vector<RendererFrameResource> submitted_resources_;
  std::vector<bool> submitted_commands_;
  std::size_t current_group_ = 0;
  Clock::time_point stage_started_ = Clock::now();
  bool published_ = false;
};
} // namespace cgpui
