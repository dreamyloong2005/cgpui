# GPUI to C++23 Migration

CGPUI preserves GPUI's authoring concepts while spelling ownership, failure,
and dynamic behavior in C++23 terms. It does not wrap Rust GPUI through FFI.

| GPUI concept | CGPUI C++23 concept |
| --- | --- |
| `Application` / `App` | `Application`, `App`, and `AppContext` facades |
| `Window` / `WindowOptions` | `Window` and fluent `WindowOptions` |
| `Context<T>` | `Context<T>` plus scoped capability facades |
| `Entity<T>` / weak entity | `EntityHandle<T>` and `WeakEntity<T>` |
| `View::render` | `View::render(ViewContext&)` returning `AnyElement` |
| `div`, stacks, widgets | Free factories and fluent typed builders |
| `Task` / background executor | `TaskHandle`, priorities, groups, and cancellation |
| fallible APIs | `Result<T>`, implemented with `std::expected` |
| actions macro | `CGPUI_ACTIONS(...)` for unit actions; payload actions are types |
| test context | `TestApp`, `TestAppWindow`, and capability-driven controls |

## Ownership

Entities and Views remain runtime-owned. Handles carry runtime identity and
weak handles must be upgraded before access. Cross-thread work uses
`CrossThreadEntity<T>` to queue scoped reads or updates back to the owner.

## Errors

Use `Result<T>` and inspect `ErrorCode` plus the diagnostic message. Throwing
facades exist only where explicitly documented, such as selected TestApp
helpers. Platform and renderer failures do not cross a Rust panic boundary.

## Callbacks and Lifetime

Prefer owned `Subscription` tokens, `TaskHandle`, `TimerId`, and animation
handles. Cancellation is explicit. Avoid capturing raw View or entity pointers
in work that can outlive its owner.

## Static and Dynamic Rendering

Use typed builders and static rendering for stable trees. Use `AnyElement` and
dynamic rendering when runtime composition requires it. Dynamic behavior is an
explicit escape hatch, not the default representation for every node.

## Platform Differences

Authoring code is source-identical across desktop targets. Platform-native
pressure, menus, dialogs, accessibility, clipboard/DnD negotiation, and IME
details are exposed only through platform-neutral results and events. Consult
the official-example matrix when an upstream example requires adaptation.
