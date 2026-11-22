#include "runtime_state_core_internal.hpp"

namespace cgpui {

WindowRuntime::RuntimeCoreState::RuntimeCoreState(
    PlatformApplication& application, View& view, RendererFactory factory)
    : application_(application), view_(view), renderer_factory_(std::move(factory)) {}

} // namespace cgpui
