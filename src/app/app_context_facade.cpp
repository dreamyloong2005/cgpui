#include "cgpui/app/app_facade.hpp"
#include "cgpui/ui/runtime_app_context.hpp"
#include "cgpui/ui/runtime_context.hpp"

namespace cgpui {

App AppContext::app() const {
  return App(runtime);
}

AppContext WindowRuntimeContext::app_context() const {
  return AppContext{.runtime = runtime};
}

App WindowRuntimeContext::app() const {
  return App(runtime);
}

Window WindowRuntimeContext::window() const {
  return current_window();
}

Window WindowRuntimeContext::current_window() const {
  return Window(runtime, window_runtime_id);
}

} // namespace cgpui
