#include "dx12_operation_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_image_cache_internal.hpp"
#include "dx12_text_cache_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_retirement_internal.hpp"
#include "dx12_native_retirement_internal.hpp"
#include "dx12_composition_internal.hpp"
#include "dx12_window_alpha_internal.hpp"
#include "dx12_geometry_upload_internal.hpp"
#include "dx12_texture_upload_internal.hpp"
#include "dx12_frame_pacing_internal.hpp"
#include "dx12_srv_descriptor_pool_internal.hpp"

namespace cgpui {
Dx12State::Dx12State(Dx12FramePacingApi* pacing_api, Dx12SynchronizationApi* synchronization)
    : image_cache(std::make_unique<Dx12ImageCache>()),
      text_cache(std::make_unique<Dx12TextCache>()),
      srv_pool(std::make_unique<Dx12SrvDescriptorPool>()),
      texture_renderer(std::make_unique<Dx12TextureRenderer>()),
      backdrop_blur(std::make_unique<Dx12BackdropBlurRenderer>()),
      geometry_uploads(std::make_unique<Dx12GeometryUploads>()),
      texture_uploads(std::make_unique<Dx12TextureUploads>()),
      frame_pacing(std::make_unique<Dx12FramePacing>(
          pacing_api ? *pacing_api : dx12_native_frame_pacing_api())),
      synchronization_api(synchronization) {}

Dx12State::~Dx12State() {
  Dx12OperationGuard operation(*this);
  dx12_retire_submitted_work(*this);
  dx12_release_native_objects(*this);
  window_alpha.reset();
}
} // namespace cgpui
