#include "runtime_state_internal.hpp"

namespace cgpui {

WindowRuntime::RuntimeState::RuntimeState(
    WindowRuntime& owner, PlatformApplication& application, View& view, RendererFactory factory)
    : tasks(owner), core(application, view, std::move(factory)) {}

WindowRuntime::RuntimeState::~RuntimeState() = default;

} // namespace cgpui
