#include "dx12_state_internal.hpp"
#include "dx12_image_cache_internal.hpp"
#include "dx12_text_cache_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"

namespace cgpui {
Dx12State::Dx12State()
    : image_cache(std::make_unique<Dx12ImageCache>()),
      text_cache(std::make_unique<Dx12TextCache>()),
      texture_renderer(std::make_unique<Dx12TextureRenderer>()) {}

Dx12State::~Dx12State() {
  if (submitted && queue && fence && fence_event.value) (void)dx12_wait_idle(*this);
}
} // namespace cgpui
