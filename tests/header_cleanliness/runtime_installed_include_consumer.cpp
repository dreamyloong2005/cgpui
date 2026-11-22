// Also compile this file outside the checkout with ONLY an installed include/
// directory. An in-tree header-cleanliness build alone cannot prove isolation.
#include "cgpui/ui/ui.hpp"
#include "cgpui/ui/element_tree.hpp"

namespace installed_runtime_consumer {
struct State { int value = 0; };
struct Action { static constexpr std::string_view name = "installed.consumer"; };
class View final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

void globals(cgpui::WindowRuntime& runtime, const cgpui::WindowRuntimeContext& context) {
  runtime.set_global(State{1});
  (void)runtime.global<State>();
  (void)std::as_const(runtime).global<State>();
  (void)runtime.update_global<State>([](State& state) { ++state.value; });
  context.set_global(State{2});
  (void)context.global<State>();
  (void)context.update_global<State>([](State& state) { ++state.value; });
  const auto app = context.app_context();
  app.set_global(State{3});
  (void)app.global<State>();
  (void)app.update_global<State>([](State& state) { ++state.value; });
}

void entities(cgpui::WindowRuntime& runtime, const cgpui::WindowRuntimeContext& context) {
  const auto inserted = runtime.insert_entity(State{1});
  const auto emplaced = runtime.emplace_entity<State>(2);
  (void)runtime.read_entity(inserted);
  (void)runtime.mutate_entity(inserted);
  (void)runtime.upgrade_entity(context.weak_entity(inserted));
  (void)runtime.notify_entity_changed(inserted);
  (void)runtime.invalidate_entity(inserted);
  runtime.subscribe_view_to_entity(context.view_id, inserted);
  (void)runtime.remove_entity(emplaced);
  const auto model = context.new_model<State>(3);
  const auto entity = context.new_entity<State>(4);
  const auto handle = context.insert_entity_handle(State{5});
  const auto context_id = context.insert_entity(State{6});
  const auto context_emplaced = context.emplace_entity<State>(7);
  (void)context.entity(context_id);
  (void)context.read_entity(context_id);
  (void)context.read_entity(handle);
  (void)context.mutate_entity(context_emplaced);
  (void)context.read_model(model);
  (void)context.update_model(model, [](State& state) { ++state.value; });
  (void)context.update_entity(entity, [](State& state) { ++state.value; });
  (void)context.update_entity(entity, [](State& state, const cgpui::WindowRuntimeContext&) {
    return ++state.value;
  });
  (void)context.upgrade_entity(entity.downgrade());
  (void)entity.read(context);
  (void)entity.update(context, [](State& state) { ++state.value; });
  (void)context.invalidate_entity(entity);
  (void)entity.invalidate(context);
  context.subscribe_view_to_entity(context.view_id, context_id);
  (void)context.remove_entity(context_id);
  (void)context.remove_entity(handle);
  (void)context.remove_model(model);
  (void)entity.remove(context);
}

void observers(cgpui::WindowRuntime& runtime, const cgpui::WindowRuntimeContext& context) {
  const auto model = context.new_model<State>(1);
  const auto entity = context.new_entity<State>(2);
  const auto observer = context.new_entity<State>(3);
  const auto model_callback = [](const cgpui::WindowRuntimeContext&, cgpui::Model<State>) {};
  const auto entity_callback = [](const cgpui::WindowRuntimeContext&, cgpui::EntityHandle<State>) {};
  const auto entity_to_entity = [](State&, cgpui::EntityHandle<State>,
                                   const cgpui::WindowRuntimeContext&) {};
  (void)runtime.observe_model(model, model_callback);
  auto runtime_model = runtime.observe_model_subscription(model, model_callback);
  (void)context.observe_model(model, model_callback);
  auto context_model = context.observe_model_subscription(model, model_callback);
  (void)context.observe_entity(entity, entity_callback);
  auto context_entity = context.observe_entity_subscription(entity, entity_callback);
  (void)entity.observe(context, entity_callback);
  auto handle_entity = entity.observe_subscription(context, entity_callback);
  (void)context.observe_entity(observer, entity, entity_to_entity);
  auto context_pair = context.observe_entity_subscription(observer, entity, entity_to_entity);
  (void)observer.observe_entity(context, entity, entity_to_entity);
  auto handle_pair = observer.observe_entity_subscription(context, entity, entity_to_entity);

  const auto runtime_window_callback = [](const cgpui::WindowRuntimeContext&) {};
  const auto window_callback = [](const cgpui::WindowRuntimeContext&,
                                  cgpui::WindowContextCapability) {};
  (void)runtime.observe_window(runtime_window_callback);
  auto runtime_window = runtime.observe_window_subscription(runtime_window_callback);
  (void)context.observe_window(window_callback);
  auto context_window = context.observe_window_subscription(window_callback);
  (void)context.window_context().observe(window_callback);
  auto capability_window = context.window_context().observe_subscription(window_callback);

  const auto view = context.view<View>();
  const auto capability = context.view_context<View>();
  const auto runtime_view_callback = [](const cgpui::WindowRuntimeContext&, cgpui::ViewId) {};
  const auto view_callback = [](const cgpui::WindowRuntimeContext&, cgpui::ViewHandle<View>) {};
  (void)runtime.observe_view(view.id(), runtime_view_callback);
  auto runtime_view = runtime.observe_view_subscription(view.id(), runtime_view_callback);
  (void)context.observe_view(view, view_callback);
  auto context_view = context.observe_view_subscription(view, view_callback);
  (void)view.observe(context, view_callback);
  auto handle_view = view.observe_subscription(context, view_callback);
  (void)capability.observe(view, view_callback);
  auto capability_view = capability.observe_subscription(view, view_callback);
  (void)context.read_view(view);
  (void)context.weak_view<View>();
  (void)context.upgrade_view(view.downgrade());
  (void)capability.view_id();
  (void)capability.view();
  (void)capability.weak_view();
  (void)capability.upgrade(view.downgrade());
  (void)capability.read(view);
  (void)capability.current();
  (void)view.read(context);
}

void cross_thread(const cgpui::WindowRuntimeContext& context) {
  const auto entity = context.new_entity<State>(1);
  const auto proxy = context.async_context().entity(entity);
  (void)proxy.read([](const State&, const cgpui::WindowRuntimeContext&) {},
      [](cgpui::CrossThreadEntityAccessStatus, const cgpui::WindowRuntimeContext&) {});
  (void)proxy.update([](State& state, const cgpui::WindowRuntimeContext&) { ++state.value; },
      [](cgpui::CrossThreadEntityAccessStatus, const cgpui::WindowRuntimeContext&) {});
}

void element_state(cgpui::WindowRuntime& runtime,
                   const cgpui::WindowRuntimeContext& context, cgpui::ElementId id) {
  (void)runtime.element_state<State>(id);
  (void)std::as_const(runtime).element_state<State>(id);
  (void)runtime.emplace_element_state<State>(id, 1);
  (void)runtime.element_state_or_init<State>(id, 2);
  (void)context.element_state<State>(id);
  (void)context.emplace_element_state<State>(id, 3);
  (void)context.element_state_or_init<State>(id, 4);
  const auto element = context.element_context(id);
  (void)element.state<State>();
  (void)element.emplace_state<State>(5);
  (void)element.state_or_init<State>(6);
}

void actions(cgpui::WindowRuntime& runtime, const cgpui::WindowRuntimeContext& context) {
  const auto callback = [](const cgpui::WindowRuntimeContext&) {
    return cgpui::EventResult::consumed_event();
  };
  runtime.register_action<Action>(callback);
  runtime.register_app_action<Action>(callback);
  runtime.register_window_action<Action>(callback);
  runtime.register_view_action<Action>(context.view_id, callback);
  runtime.register_focused_element_action<Action>(cgpui::ElementId{1}, callback);
  runtime.register_command_palette_entry<Action>({});
  (void)runtime.dispatch_action<Action>();
  context.register_action<Action>(callback);
  context.register_app_action<Action>(callback);
  context.register_window_action<Action>(callback);
  context.register_view_action<Action>(callback);
  context.register_focused_element_action<Action>(cgpui::ElementId{1}, callback);
  (void)context.dispatch_action<Action>();
}
} // namespace installed_runtime_consumer

int main() { return 0; }
