# WindowRuntime state and lifetime boundaries

`WindowRuntime` exposes the existing public API with one owning opaque-state
pointer. This changes its object layout: rebuild the library and consumers
together; do not combine a pre-isolation static library with newer headers.

Installed `include/cgpui/ui/detail/window_runtime_*_declarations.hpp` files
contain only private method declarations, friends and incomplete nested types.
They never include repository `src/` headers or define runtime storage.

Complete state belongs to focused private `src/ui/runtime_state_*_internal.hpp`
modules: core, windows, views, elements, input, actions, text, entities,
scheduling, animations, tasks, platform and diagnostics. Small field-only
fragments remain private and are actually composed by their owning groups.
Non-template implementation accesses state explicitly; there are no field-name
alias macros. `runtime_state_internal.hpp` only composes the groups.

## Public templates

Public templates construct user types and typed callback adapters. Focused
non-template bridges own entity/global stores, observer registration, element
tree access and the cross-thread queue. Templates do not inspect state fields.

The already-public `EntityObserver`, `WindowObserver` and `ViewObserver` value
types remain available for source compatibility. They do not expose the
runtime's owning containers; template bridges no longer require clients to
construct these storage records. `EntitySubscription` remains public because
the query API returns its values.

## Shutdown ordering

Ordinary `run()` deactivation retains views and metadata needed for another
run. Actual runtime destruction is a separate terminal phase:

1. Mark terminal shutdown, disconnect platform callbacks, and close the
   cross-thread queue.
2. Cancel tasks under their mutex, move callback captures out of the lock,
   join the worker pool, and close asynchronous I/O.
3. Deactivate child surfaces and retire owned renderers before native windows.
4. Destroy coherent, detached batches of user resources, including views,
   element trees, timers, deferred work, globals, entities, observers, actions,
   animations and callbacks. Reentrant additions are drained before continuing.
5. Detach TaskGroup lifetime tokens only after these destructors have finished.

The task state group is constructed first and destroyed last as a final
safeguard. Terminal task, timer and deferred registration is rejected only
during actual destruction, not merely because a run has quit.

Window retirement preserves tree-before-view and renderer-before-native
ordering even when a destructor closes another window. Stable record storage,
callback scopes and collection guards prevent reclamation under a live stack.

TaskGroup's weak token protects late RAII destruction on the UI thread. It does
not make concurrent runtime deletion safe and does not extend the lifetime
contract of raw TaskHandle queries.

## Verification boundaries

Architecture tests enforce pointer-sized runtime representation, include-only
public closure, private state ownership and per-module line budgets. The
installed consumer instantiates the template families outside the checkout;
linking that consumer is also required to detect missing bridge definitions.
Behavior and lifecycle regressions remain separate acceptance requirements.
