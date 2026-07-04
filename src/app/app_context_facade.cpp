#include "cgpui/app/app_facade.hpp"
#include "cgpui/ui/runtime_app_context.hpp"
#include "cgpui/ui/runtime_context.hpp"

namespace cgpui {

App AppContext::app() const {
  return App(runtime);
}

App WindowRuntimeContext::app() const {
  return App(runtime);
}

Window WindowRuntimeContext::current_window() const {
  return Window(runtime, window_runtime_id);
}

} // namespace cgpui
