#include "dx12_state_internal.hpp"
#include "dx12_image_cache_internal.hpp"
#include "dx12_text_cache_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_retirement_internal.hpp"

namespace cgpui {
Dx12State::Dx12State()
    : image_cache(std::make_unique<Dx12ImageCache>()),
      text_cache(std::make_unique<Dx12TextCache>()),
      texture_renderer(std::make_unique<Dx12TextureRenderer>()),
      backdrop_blur(std::make_unique<Dx12BackdropBlurRenderer>()) {}

Dx12State::~Dx12State() {
  dx12_retire_submitted_work(*this);
}
} // namespace cgpui
