#pragma once

#include "cgpui/ui/window_runtime.hpp"
#include "runtime_state_animations_types_internal.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeAnimationState {
  #include "runtime_animation_frame_pacing_internal.hpp"
  #include "runtime_animation_state_internal.hpp"
};

} // namespace cgpui
