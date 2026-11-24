#pragma once
#include "dx12_command_reuse_native_test_support.hpp"
#include <string>

namespace cgpui_test {
enum class TransitionChange { none, clear, geometry, order, common_clip, sampling, mixed_clip };
enum class TransitionUpload { none, initial, replace, invalidate, resupply, undrawn, undrawn_invalidate };
enum class TransitionRoute { direct, fresh, hit };
struct TransitionScene {TransitionChange change=TransitionChange::none;bool second_image=true;};
struct TransitionFixture {
  Dx12Window window;
  std::shared_ptr<cgpui::Dx12State> state;
  ReuseQueueTail* tail=nullptr;
  TransitionFixture();
};
struct TransitionFrame {
  ID3D12GraphicsCommandList* bundle=nullptr; // Observed identity only; never an owner.
  std::vector<std::uint8_t> pixels;
  std::size_t image_upload_bytes=0;
};
void transition_require(bool,const std::string&);
bool transition_present(const std::shared_ptr<cgpui::Dx12State>&,TransitionScene,TransitionUpload,bool capture);
TransitionFrame transition_frame(TransitionFixture&,TransitionScene,TransitionUpload,bool capture,
    TransitionRoute,ID3D12GraphicsCommandList* previous=nullptr,std::size_t images=2);
void transition_pixels(const TransitionFrame&,const TransitionFrame&,bool same,const char*);
} // namespace cgpui_test
