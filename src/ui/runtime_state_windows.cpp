#include "runtime_state_windows_internal.hpp"
#include "runtime_window_store_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

WindowRuntime::RuntimeWindowState::RuntimeWindowState()
    : window_runtime_records_(std::make_unique<RuntimeWindowStore>()),
      window_lifetime_(std::make_unique<RuntimeWindowLifetime>()) {}

WindowRuntime::RuntimeWindowState::~RuntimeWindowState() = default;

} // namespace cgpui
