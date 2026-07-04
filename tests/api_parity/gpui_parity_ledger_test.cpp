#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  const std::string root = source_root();
  std::ifstream source(root + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

int require_contains_all(
    const std::string& text,
    const std::vector<const char*>& values,
    int exit_code) {
  for (const char* value : values) {
    if (!contains(text, value)) {
      return exit_code;
    }
  }
  return 0;
}

} // namespace

int main() {
  const std::string pinned =
      read_source("docs/gpui-upstream-pinned-revision.md");
  if (pinned.empty()) {
    return 1;
  }
  if (const int result = require_contains_all(
          pinned,
          {
              "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0",
              "gpui = 0.2.2",
              "gpui_platform = 0.1.0",
              "https://github.com/zed-industries/zed/tree/5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0/crates/gpui",
              "README.md",
              "src/gpui.rs",
              "docs/contexts.md",
              "docs/key_dispatch.md",
              "examples/hello_world.rs",
          },
          2);
      result != 0) {
    return result;
  }

  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.md");
  if (ledger.empty()) {
    return 3;
  }
  if (const int result = require_contains_all(
          ledger,
          {
              "# Complete GPUI Parity Ledger",
              "Pinned upstream revision",
              "Required",
              "Adapted",
              "Deferred",
              "Non-goal",
              "Application and app context",
              "Entities and state",
              "Views and Render",
              "Elements and styling",
              "Actions and key dispatch",
              "Platform services",
              "Async executor",
              "Test support",
              "Examples",
              "Windows: Win32 + Vulkan",
              "Linux: Wayland + Vulkan",
              "macOS: Cocoa + Metal",
              "X11",
          },
          4);
      result != 0) {
    return result;
  }
  if (!contains(ledger, "| upstream_gpui | cgpui_target | status |") ||
      !contains(ledger, "| gpui::Application |") ||
      !contains(ledger, "| gpui::App |") ||
      !contains(ledger, "| gpui::Window |") ||
      !contains(ledger, "| gpui::Context<T> |") ||
      !contains(ledger, "| gpui::View<T> |") ||
      !contains(ledger, "| gpui::Render |") ||
      !contains(ledger, "| gpui::prelude |") ||
      !contains(ledger, "| gpui::div |") ||
      !contains(ledger, "| gpui::test |") ||
      !contains(ledger, "| gpui_platform::application |")) {
    return 5;
  }
  if (!contains(ledger, "`App` facade from `AppContext::app()`") ||
      !contains(ledger, "`Context<T>::app_context()` app-domain capability") ||
      !contains(ledger,
                "`Window` facade from `WindowRuntimeContext::window()`") ||
      !contains(ledger, "tests/api_parity/app_window_context_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/app_context_capability_test.cpp")) {
    return 15;
  }
  if (!contains(ledger, "`Context<T>` alias with app/window/entity helpers") ||
      !contains(ledger, "`Context<T>::app_context()`") ||
      !contains(ledger, "`Context<T>::view_context<T>()`") ||
      !contains(ledger, "`Context<T>::window_context()`") ||
      !contains(ledger, "`Context<T>::element_context(ElementId)`") ||
      !contains(ledger, "`Context<T>::async_context()`") ||
      !contains(ledger, "`Context<T>::test_context()`") ||
      !contains(ledger, "`ElementContextCapability`") ||
      !contains(ledger, "`AsyncContextCapability`") ||
      !contains(ledger, "`TestContextCapability`") ||
      !contains(ledger, "`EntityHandle<T>`") ||
      !contains(ledger, "`Context<T>::new_entity<T>(...)`") ||
      !contains(ledger, "weak upgrade/read semantics") ||
      !contains(ledger, "`observe_entity(...)` helpers") ||
      !contains(ledger, "entity observation helpers") ||
      !contains(ledger, "entity-to-entity observation helpers") ||
      !contains(ledger, "window/view observation helpers") ||
      !contains(ledger, "`update_entity(...)` transaction helpers") ||
      !contains(ledger, "value-returning update transactions") ||
      !contains(ledger, "`invalidate_entity(...)` helpers") ||
      !contains(ledger, "entity invalidation helpers") ||
      !contains(ledger, "entity deletion helpers") ||
      !contains(ledger, "deterministic subscription lifetime/unsubscribe behavior") ||
      !contains(ledger, "deterministic `Subscription` lifetime/unsubscribe behavior") ||
      !contains(ledger, "runtime-token cross-context boundaries") ||
      !contains(ledger, "`ViewContextCapability<T>`") ||
      !contains(ledger, "`ViewHandle<T>` and `WeakViewHandle<T>`") ||
      !contains(ledger, "`Render<T>` concept over") ||
      !contains(ledger, "`IntoElement` alias plus `into_element`") ||
      !contains(ledger, "include/cgpui/ui/render.hpp") ||
      !contains(ledger, "include/cgpui/ui/async_context.hpp") ||
      !contains(ledger, "include/cgpui/ui/test_context.hpp") ||
      !contains(ledger, "include/cgpui/ui/element_context.hpp") ||
      !contains(ledger, "include/cgpui/ui/view_handle.hpp") ||
      !contains(ledger, "include/cgpui/ui/window_context.hpp") ||
      !contains(ledger, "include/cgpui/prelude.hpp") ||
      !contains(ledger,
                "tests/api_parity/context_capabilities_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/view_handle_spelling_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/view_context_capability_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/window_context_capability_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/element_context_capability_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/async_context_capability_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/test_context_capability_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/public_authoring_surface_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_lifecycle_creation_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_weak_handle_semantics_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_observation_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_to_entity_observation_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/window_view_observation_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/subscription_lifetime_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_transaction_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_invalidation_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_deletion_test.cpp")) {
    return 16;
  }
  if (!contains(ledger, "| gpui_platform x11 feature | Deferred |") ||
      !contains(ledger, "| gpui_platform wayland feature | Required |") ||
      !contains(ledger, "| gpui Windows backend | Required |")) {
    return 6;
  }
  if (!contains(ledger, "| gpui::TestAppContext |") ||
      !contains(ledger, "`TestContextCapability` groups runtime id/view id") ||
      !contains(ledger,
                "Phase G fuller simulated input/test macro depth")) {
    return 18;
  }

  const std::string status_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  if (status_json.empty()) {
    return 7;
  }
  if (const int result = require_contains_all(
          status_json,
          {
              "\"upstream_revision\"",
              "\"5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0\"",
              "\"required\"",
              "\"adapted\"",
              "\"deferred\"",
              "\"non_goal\"",
              "\"examples\"",
              "\"hello_world\"",
              "\"uniform_list\"",
              "\"window_shadow\"",
              "\"gpui::prelude\"",
              "WindowContextCapability",
              "window_context",
              "AsyncContextCapability",
              "async_context",
              "TestContextCapability",
              "test_context",
              "gpui::TestAppContext",
              "deterministic subscription lifetime/unsubscribe behavior",
              "deterministic Subscription lifetime/unsubscribe behavior",
              "entity-to-entity observation helpers",
              "window/view observation helpers",
              "invalidate_entity helpers",
              "entity deletion helpers",
              "\"x11\"",
          },
          8);
      result != 0) {
    return result;
  }

  const std::string tool_readme = read_source("tools/gpui_parity/README.md");
  const std::string extractor =
      read_source("tools/gpui_parity/extract_upstream_symbols.py");
  if (tool_readme.empty() || extractor.empty()) {
    return 9;
  }
  if (!contains(tool_readme, "extract_upstream_symbols.py") ||
      !contains(tool_readme,
                "docs/gpui-complete-parity-ledger.json") ||
      !contains(extractor, "GPUI_UPSTREAM_REVISION") ||
      !contains(extractor, "extract_public_reexports") ||
      !contains(extractor, "extract_examples")) {
    return 10;
  }

  const std::string plan = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-upstream-parity-ledger-plan.md");
  if (plan.empty() ||
      !contains(plan, "Steps 219-258") ||
      !contains(plan, "Run RED") ||
      !contains(plan, "Run GREEN") ||
      !contains(plan, "Phase A complete")) {
    return 11;
  }

  const std::string example =
      read_source("examples/api_parity/hello_world/main.cpp");
  if (example.empty()) {
    return 12;
  }
  if (!contains(example, "class HelloWorldView") ||
      !contains(example, "cgpui::Context<HelloWorldView>&") ||
      !contains(example, "cgpui::IntoElement render") ||
      !contains(example, "cgpui::Render<HelloWorldView>") ||
      !contains(example, "cgpui::div()") ||
      !contains(example, "cgpui::Application::create") ||
      !contains(example, "app->run") ||
      !contains(example, "GPUI upstream hello_world.rs parity")) {
    return 13;
  }

  const std::string public_authoring =
      read_source("tests/api_parity/public_authoring_surface_test.cpp");
  if (public_authoring.empty()) {
    return 17;
  }
  if (!contains(public_authoring, "#include \"cgpui/prelude.hpp\"") ||
      !contains(public_authoring, "cgpui::Context<PublicAuthoringSurfaceView>&") ||
      !contains(public_authoring, "cgpui::IntoElement render") ||
      !contains(public_authoring, "cgpui::Render<PublicAuthoringSurfaceView>") ||
      !contains(public_authoring, "cgpui::ViewHandle<PublicAuthoringSurfaceView>") ||
      !contains(public_authoring, "cgpui::Application")) {
    return 18;
  }
  if (contains(public_authoring, "WindowRuntimeContext") ||
      contains(public_authoring, "WindowRuntime") ||
      contains(public_authoring, "AppContext") ||
      contains(public_authoring, "ViewContext") ||
      contains(public_authoring, "PlatformWindow")) {
    return 19;
  }

  const std::string entity_lifecycle =
      read_source("tests/api_parity/entity_lifecycle_creation_test.cpp");
  if (entity_lifecycle.empty()) {
    return 20;
  }
  if (!contains(entity_lifecycle, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_lifecycle, "context.new_entity<LifecycleState>") ||
      !contains(entity_lifecycle, "cgpui::EntityHandle<LifecycleState>") ||
      !contains(entity_lifecycle, "entity.update(context") ||
      !contains(entity_lifecycle, "cgpui::Render<EntityLifecycleCreationView>")) {
    return 21;
  }
  if (contains(entity_lifecycle, "WindowRuntimeContext") ||
      contains(entity_lifecycle, "WindowRuntime") ||
      contains(entity_lifecycle, "AppContext") ||
      contains(entity_lifecycle, "ViewContext") ||
      contains(entity_lifecycle, "PlatformWindow")) {
    return 22;
  }

  const std::string entity_weak_handles =
      read_source("tests/api_parity/entity_weak_handle_semantics_test.cpp");
  if (entity_weak_handles.empty()) {
    return 23;
  }
  if (!contains(entity_weak_handles, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_weak_handles, "weak.upgrade(context)") ||
      !contains(entity_weak_handles, "weak.read(context)") ||
      !contains(entity_weak_handles,
                "std::optional<cgpui::EntityHandle<WeakLifecycleState>>") ||
      !contains(entity_weak_handles,
                "cgpui::Render<EntityWeakHandleSemanticsView>")) {
    return 24;
  }
  if (contains(entity_weak_handles, "WindowRuntimeContext") ||
      contains(entity_weak_handles, "WindowRuntime") ||
      contains(entity_weak_handles, "AppContext") ||
      contains(entity_weak_handles, "ViewContext") ||
      contains(entity_weak_handles, "PlatformWindow")) {
    return 25;
  }

  const std::string entity_observation =
      read_source("tests/api_parity/entity_observation_test.cpp");
  if (entity_observation.empty()) {
    return 26;
  }
  if (!contains(entity_observation, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_observation, "entity.observe(") ||
      !contains(entity_observation, "entity.observe_subscription(") ||
      !contains(entity_observation, "context.observe_entity(") ||
      !contains(entity_observation, "context.observe_entity_subscription") ||
      !contains(entity_observation,
                "cgpui::EntityHandle<ObservedState>") ||
      !contains(entity_observation,
                "cgpui::Render<EntityObservationView>")) {
    return 27;
  }
  if (contains(entity_observation, "WindowRuntimeContext") ||
      contains(entity_observation, "WindowRuntime") ||
      contains(entity_observation, "AppContext") ||
      contains(entity_observation, "ViewContext") ||
      contains(entity_observation, "PlatformWindow")) {
    return 28;
  }

  const std::string entity_to_entity_observation =
      read_source("tests/api_parity/entity_to_entity_observation_test.cpp");
  if (entity_to_entity_observation.empty()) {
    return 53;
  }
  if (!contains(entity_to_entity_observation, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_to_entity_observation, "observer.observe_entity(") ||
      !contains(entity_to_entity_observation,
                "context.observe_entity_subscription") ||
      !contains(entity_to_entity_observation, "ObserverState& owner") ||
      !contains(entity_to_entity_observation,
                "cgpui::EntityHandle<ObservedState>") ||
      !contains(entity_to_entity_observation,
                "cgpui::Render<EntityToEntityObservationView>")) {
    return 54;
  }
  if (contains(entity_to_entity_observation, "WindowRuntimeContext") ||
      contains(entity_to_entity_observation, "WindowRuntime") ||
      contains(entity_to_entity_observation, "AppContext") ||
      contains(entity_to_entity_observation, "ViewContext") ||
      contains(entity_to_entity_observation, "PlatformWindow")) {
    return 55;
  }

  const std::string window_view_observation =
      read_source("tests/api_parity/window_view_observation_test.cpp");
  if (window_view_observation.empty()) {
    return 56;
  }
  if (!contains(window_view_observation, "#include \"cgpui/prelude.hpp\"") ||
      !contains(window_view_observation, "context.observe_window(") ||
      !contains(window_view_observation,
                "observe_window_subscription") ||
      !contains(window_view_observation, "window_context.observe") ||
      !contains(window_view_observation, "view.observe(") ||
      !contains(window_view_observation, "view_context.observe") ||
      !contains(window_view_observation,
                "cgpui::Render<WindowViewObservationView>")) {
    return 57;
  }
  if (contains(window_view_observation, "WindowRuntimeContext") ||
      contains(window_view_observation, "WindowRuntime") ||
      contains(window_view_observation, "AppContext") ||
      contains(window_view_observation, "PlatformWindow")) {
    return 58;
  }

  const std::string subscription_lifetime =
      read_source("tests/api_parity/subscription_lifetime_test.cpp");
  if (subscription_lifetime.empty()) {
    return 50;
  }
  if (!contains(subscription_lifetime, "#include \"cgpui/prelude.hpp\"") ||
      !contains(subscription_lifetime, "subscription.release()") ||
      !contains(subscription_lifetime, "context.observe_entity_subscription") ||
      !contains(subscription_lifetime, "std::move(moved_from)") ||
      !contains(subscription_lifetime,
                "cgpui::Render<SubscriptionLifetimeView>")) {
    return 51;
  }
  if (contains(subscription_lifetime, "WindowRuntimeContext") ||
      contains(subscription_lifetime, "WindowRuntime") ||
      contains(subscription_lifetime, "AppContext") ||
      contains(subscription_lifetime, "ViewContext") ||
      contains(subscription_lifetime, "PlatformWindow")) {
    return 52;
  }

  const std::string entity_transaction =
      read_source("tests/api_parity/entity_transaction_test.cpp");
  if (entity_transaction.empty()) {
    return 29;
  }
  if (!contains(entity_transaction, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_transaction, "entity.update(") ||
      !contains(entity_transaction, "context.update_entity(") ||
      !contains(entity_transaction, "std::optional<int>") ||
      !contains(entity_transaction,
                "const cgpui::Context<TransactionState>&") ||
      !contains(entity_transaction,
                "cgpui::Render<EntityUpdateTransactionView>")) {
    return 30;
  }
  if (contains(entity_transaction, "WindowRuntimeContext") ||
      contains(entity_transaction, "WindowRuntime") ||
      contains(entity_transaction, "AppContext") ||
      contains(entity_transaction, "ViewContext") ||
      contains(entity_transaction, "PlatformWindow")) {
    return 31;
  }

  const std::string entity_invalidation =
      read_source("tests/api_parity/entity_invalidation_test.cpp");
  if (entity_invalidation.empty()) {
    return 32;
  }
  if (!contains(entity_invalidation, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_invalidation, "entity.invalidate(context)") ||
      !contains(entity_invalidation, "context.invalidate_entity(entity)") ||
      !contains(entity_invalidation,
                "cgpui::EntityHandle<InvalidationStateModel>") ||
      !contains(entity_invalidation, "std::same_as") ||
      !contains(entity_invalidation,
                "cgpui::Render<EntityInvalidationView>")) {
    return 33;
  }
  if (contains(entity_invalidation, "WindowRuntimeContext") ||
      contains(entity_invalidation, "WindowRuntime") ||
      contains(entity_invalidation, "AppContext") ||
      contains(entity_invalidation, "ViewContext") ||
      contains(entity_invalidation, "PlatformWindow")) {
    return 34;
  }

  const std::string entity_deletion =
      read_source("tests/api_parity/entity_deletion_test.cpp");
  if (entity_deletion.empty()) {
    return 35;
  }
  if (!contains(entity_deletion, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_deletion, "entity.remove(context)") ||
      !contains(entity_deletion, "context.remove_entity(context_entity)") ||
      !contains(entity_deletion, "missing.remove(context)") ||
      !contains(entity_deletion,
                "cgpui::EntityHandle<DeletionState>") ||
      !contains(entity_deletion, "std::same_as") ||
      !contains(entity_deletion,
                "cgpui::Render<EntityDeletionView>")) {
    return 36;
  }
  if (contains(entity_deletion, "WindowRuntimeContext") ||
      contains(entity_deletion, "WindowRuntime") ||
      contains(entity_deletion, "AppContext") ||
      contains(entity_deletion, "ViewContext") ||
      contains(entity_deletion, "PlatformWindow")) {
    return 37;
  }

  const std::string app_context_capability =
      read_source("tests/api_parity/app_context_capability_test.cpp");
  if (app_context_capability.empty()) {
    return 38;
  }
  if (!contains(app_context_capability, "#include \"cgpui/prelude.hpp\"") ||
      !contains(app_context_capability, "context.app_context()") ||
      !contains(app_context_capability, "cgpui::AppContext") ||
      !contains(app_context_capability, "app_context.app()") ||
      !contains(app_context_capability,
                "app_context.global<CapabilityGlobal>()") ||
      !contains(app_context_capability, "std::same_as") ||
      !contains(app_context_capability,
               "cgpui::Render<AppContextCapabilityView>")) {
    return 39;
  }
  if (contains(app_context_capability, "WindowRuntimeContext") ||
      contains(app_context_capability, "WindowRuntime") ||
      contains(app_context_capability, "ViewContext") ||
      contains(app_context_capability, "PlatformWindow")) {
    return 40;
  }

  const std::string view_context_capability =
      read_source("tests/api_parity/view_context_capability_test.cpp");
  if (view_context_capability.empty()) {
    return 41;
  }
  if (!contains(view_context_capability, "#include \"cgpui/prelude.hpp\"") ||
      !contains(view_context_capability, "context.view_context") ||
      !contains(view_context_capability, "cgpui::ViewContextCapability") ||
      !contains(view_context_capability, "view_context.view()") ||
      !contains(view_context_capability, "view_context.weak_view()") ||
      !contains(view_context_capability, "view_context.current()") ||
      !contains(view_context_capability, "std::same_as") ||
      !contains(view_context_capability,
                "cgpui::Render<ViewContextCapabilityView>")) {
    return 42;
  }
  if (contains(view_context_capability, "WindowRuntimeContext") ||
      contains(view_context_capability, "WindowRuntime") ||
      contains(view_context_capability, "AppContext") ||
      contains(view_context_capability, "PlatformWindow")) {
    return 43;
  }

  const std::string window_context_capability =
      read_source("tests/api_parity/window_context_capability_test.cpp");
  if (window_context_capability.empty()) {
    return 44;
  }
  if (!contains(window_context_capability, "#include \"cgpui/prelude.hpp\"") ||
      !contains(window_context_capability, "context.window_context()") ||
      !contains(window_context_capability, "cgpui::WindowContextCapability") ||
      !contains(window_context_capability, "window_context.window()") ||
      !contains(window_context_capability,
                "window_context.current_window()") ||
      !contains(window_context_capability, "window_context.runtime_id()") ||
      !contains(window_context_capability, "window_context.request_render()") ||
      !contains(window_context_capability, "std::same_as") ||
      !contains(window_context_capability,
                "cgpui::Render<WindowContextCapabilityView>")) {
    return 45;
  }
  if (contains(window_context_capability, "WindowRuntimeContext") ||
      contains(window_context_capability, "PlatformWindow")) {
    return 46;
  }

  const std::string element_context_capability =
      read_source("tests/api_parity/element_context_capability_test.cpp");
  if (element_context_capability.empty()) {
    return 47;
  }
  if (!contains(element_context_capability, "#include \"cgpui/prelude.hpp\"") ||
      !contains(element_context_capability, "context.element_context") ||
      !contains(element_context_capability, "cgpui::ElementContextCapability") ||
      !contains(element_context_capability, "element_context.element_id()") ||
      !contains(element_context_capability,
                "element_context.request_keyboard_focus()") ||
      !contains(element_context_capability,
                "element_context.capture_pointer()") ||
      !contains(element_context_capability, "element_context.set_cursor") ||
      !contains(element_context_capability, "element_context.state") ||
      !contains(element_context_capability, "std::same_as") ||
      !contains(element_context_capability,
                "cgpui::Render<ElementContextCapabilityView>")) {
    return 48;
  }
  if (contains(element_context_capability, "WindowRuntimeContext") ||
      contains(element_context_capability, "PlatformWindow")) {
    return 49;
  }

  const std::string xmake = read_source("xmake.lua");
  if (!contains(xmake, "target(\"gpui_parity_ledger_test\")") ||
      !contains(xmake, "target(\"context_render_spelling_test\")") ||
      !contains(xmake, "target(\"context_capabilities_test\")") ||
      !contains(xmake, "target(\"app_context_capability_test\")") ||
      !contains(xmake, "target(\"view_context_capability_test\")") ||
      !contains(xmake, "target(\"window_context_capability_test\")") ||
      !contains(xmake, "target(\"element_context_capability_test\")") ||
      !contains(xmake, "target(\"view_handle_spelling_test\")") ||
      !contains(xmake, "target(\"public_authoring_surface_test\")") ||
      !contains(xmake, "target(\"entity_lifecycle_creation_test\")") ||
      !contains(xmake, "target(\"entity_weak_handle_semantics_test\")") ||
      !contains(xmake, "target(\"entity_observation_test\")") ||
      !contains(xmake, "target(\"entity_transaction_test\")") ||
      !contains(xmake, "target(\"entity_invalidation_test\")") ||
      !contains(xmake, "target(\"entity_deletion_test\")") ||
      !contains(xmake, "target(\"api_parity_hello_world\")") ||
      !contains(xmake, "tests/api_parity/gpui_parity_ledger_test.cpp") ||
      !contains(xmake, "tests/api_parity/context_render_spelling_test.cpp") ||
      !contains(xmake, "tests/api_parity/context_capabilities_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/app_context_capability_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/view_context_capability_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/window_context_capability_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/element_context_capability_test.cpp") ||
      !contains(xmake, "tests/api_parity/view_handle_spelling_test.cpp") ||
      !contains(xmake, "tests/api_parity/public_authoring_surface_test.cpp") ||
      !contains(xmake, "tests/api_parity/entity_lifecycle_creation_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/entity_weak_handle_semantics_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/entity_observation_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/entity_transaction_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/entity_invalidation_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/entity_deletion_test.cpp") ||
      !contains(xmake, "examples/api_parity/hello_world/main.cpp")) {
    return 14;
  }

  return 0;
}
