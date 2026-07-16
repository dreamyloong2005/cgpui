# Non-goals

The parity claim is scoped to the pinned GPUI revision and the desktop
GPUI-core-shaped surface recorded in the authority ledger. The following are
not implied by Phase J completion.

## Unsupported Platforms

- Web/WASM
- Android
- iOS

Adding one of these targets requires a separate platform, renderer, packaging,
and verification plan.

## Engine Integration

- Game-engine-specific integration
- Engine runtime embedding
- Reuse of an external engine's renderer, resource, asset, or command systems
- Game editor/runtime UI depth beyond the public desktop framework

These are consumer integrations, not framework parity rows.

## Compatibility Boundaries

- No Rust FFI compatibility layer
- No source or ABI compatibility with Rust crates
- No promise that an unpinned future GPUI revision is already covered
- No silent mutation of completed audit evidence after upstream drift

Upstream changes create a new parity delta plan under Steps 841+.

## Performance Boundaries

Checked-in performance budgets are regression tripwires, not cross-machine
rankings or real-time guarantees. Stress profiles prove bounded scenarios, not
unbounded resource use or immunity to hostile inputs.

## Native Policy

Platform-neutral APIs do not promise identical native capabilities. For
example, the non-macOS `mouse_pressure` adaptation covers pointer and capture
behavior without claiming force-touch data. Unsupported native services return
typed failures and diagnostics rather than fabricated success.
