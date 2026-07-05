# Public Authoring Vocabulary Freeze

Phase B Step 308 freezes the public authoring vocabulary that Windows/Linux
examples and API-parity tests may rely on before Phase C starts.

The frozen include boundary is `#include "cgpui/prelude.hpp"`.

This document is a compatibility guard, not a promise that every listed type is
feature-complete with upstream GPUI. It records the public names that should be
used from application-author code and the module boundary that future slices
must keep green.

## Application entry vocabulary

- `Application`
- `App`
- `Window`
- `WindowOptions`
- `AppContext`

These names are available through the prelude and the thin app aggregate. Public
examples should use them instead of constructing runtime internals directly.

## View authoring vocabulary

- `Context<T>`
- `Render<T>`
- `IntoElement`
- `ViewContext`
- `ViewContextCapability<T>`
- `WindowContextCapability`
- `ElementContextCapability`

These names define the author-facing render and context vocabulary. The prelude
is the public include entry; examples should not include focused UI leaves
directly just to access these names.

## Entity and view handle vocabulary

- `EntityHandle<T>`
- `WeakEntity<T>`
- `ViewHandle<T>`
- `WeakViewHandle<T>`

These names are the public typed-handle spelling for Phase B. They wrap the
current runtime storage boundaries without requiring application code to name
runtime ids or stores.

## Action and key vocabulary

- `Action<T>`
- `CommandPaletteEntry`
- `KeyBindingContext`

Typed actions, command-palette metadata, and key-binding context selection are
public authoring vocabulary. Macro-style action payload generation remains
outside this freeze.

## Async, test, and Result vocabulary

- `Result<T>`
- `ErrorCode`
- `AsyncContextCapability`
- `TestContextCapability`

Phase B public examples may use Result-returning async and test helpers. This
freeze does not add task priorities, structured task groups, or upstream test
macro equivalents.

## Platform service vocabulary

- `NativeMenuModel`
- `NativeFileDialogOptions`

Platform-service vocabulary stays public through Result-returning menu and file
dialog helpers. Production-depth native platform behavior remains later work.

## Out of scope for this freeze

- `ClipboardItem` payload parity
- upstream `gpui::test` macro equivalents
- action macro payloads
- task priorities
- structured task groups

These items remain future Phase B or later work and should not be introduced
while freezing the current public vocabulary.

## Forbidden from public examples

Public compatibility examples must not include private headers, source-relative
headers, platform or renderer internals, or direct `WindowRuntime` internals.
If a public example needs such a name, the missing author-facing facade belongs
in a focused public module first.
