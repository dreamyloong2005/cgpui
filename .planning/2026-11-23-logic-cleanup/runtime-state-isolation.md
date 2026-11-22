# WindowRuntime: installed-header and state-isolation slice

Historical design plan (the implementation is now present; final validation is
recorded in acceptance.md). Generated migration inventories/scripts have been
moved to `D:/Dev/shots/cgpui-logic-fixes-20261123/planning-history/`; they describe
the former layout and must not be reapplied. The maintained contract is in
`docs/runtime-state-isolation.md` and `docs/runtime-lifecycle.md`.

Initial status: RED tests/probe prepared; implementation deliberately waited for the
current lifecycle/task/renderer fixes to become GREEN. Parent executed the
external include-only consumer on 2026-11-23: `cl /c /Iinclude` in
`D:/Dev/shots/cgpui-logic-fixes-20261123/installed-headers-red`, with only copied
include headers and the consumer source. `installed-headers-red.log` records
C1083 for `../../../src/ui/window_runtime_internal.hpp` at public header line
258. The sizeof/complete-state structural check has not been run yet. No
successful compile or SDK packaging run is claimed by this plan.

## Acceptance, not a header relocation

- `WindowRuntime` has one owning opaque state pointer (pointer-sized object).
  All existing public method signatures remain source-compatible; this is an
  ABI/layout change, so libraries and consumers must be rebuilt together.
- Installed headers do not reach `src/`, contain complete runtime state/groups,
  or expose fields through templates. Moving the current private closure into
  `include/detail/` without changing the representation does not qualify.
- Public `include/cgpui/ui/detail/window_runtime_declarations.hpp` may declare
  opaque nested types, friends and non-template private bridges. It must not
  define state containers, storage or non-template implementation bodies.
- Actual definitions live in focused `src/ui/runtime_state_*_internal.hpp`
  files. Non-template implementation uses explicit `state_->group.field`;
  no field-name alias macros and no giant catch-all state group.
- New module budgets supplement, never increase, existing line-count caps.
  Behavior tests remain required: layout/sizeof alone is not proof of safety.

## Proposed field ownership map

The mapping covers the current private declaration and included fragments.
Methods stay in their existing focused implementation modules; only state
access and narrowly justified bridges change.

| Private state group / header | Existing fields and nested definitions |
| --- | --- |
| `RuntimeCoreState` / `runtime_state_core_internal.hpp` | `application_`, borrowed `view_`, `renderer_factory_`, `should_quit_`, `failed_`; after-frame/render/event, close-request and error callbacks |
| `RuntimeWindowState` / `runtime_state_windows_internal.hpp` | `window_`, `renderer_`, sizes/scale, root/next window IDs, keyboard popup, opened-window metadata, stable record store, native/retired native owners, renderer lifetime ledger; opaque lifetime/store/scope declarations remain on WindowRuntime |
| `RuntimeViewState` / `runtime_state_views_internal.hpp` | `RegisteredView`, root/next view IDs, view registry and removed IDs; ownership teardown is explicit before task managers disappear |
| `RuntimeElementState` / `runtime_state_elements_internal.hpp` | root owned ElementTree, rendered/tree-root/static-tree state, `WindowElementHost` and host map, element cursors; static rendering result type remains private |
| `RuntimeInputState` / `runtime_state_input_internal.hpp` | input snapshot, pointer capture, keyboard view/element focus, `TextPointerSelectionDrag`, hovered/active IDs, cursors, last event result, current event route, dispatching-view-event flag |
| `RuntimeActionState` / `runtime_state_actions_internal.hpp` | app/window/view/focused-element handler maps, registrations, command palette, key bindings/pending chord, action window ID, `ActionDispatchTarget` and `ActionWindowScope` (scope methods out of public headers) |
| `RuntimeTextState` / `runtime_state_text_internal.hpp` | text edit bindings/models, clipboard, last applied IME placement; no native font lookup/rasterization implementation ownership here |
| `RuntimeEntityState` / `runtime_state_entities_internal.hpp` | globals/entity stores (`type_index` to `any`), entity count, entity subscriptions, entity/window/view observer vectors, next subscription ID and subscription query buffer |
| `RuntimeScheduleState` / `runtime_state_scheduling_internal.hpp` | deferred callbacks, `RuntimeTimer` including window owner, timer IDs/times/clock flags, wake/defer/batch guards, invalidation, redraw/deferred redraw/rendering/next-frame flags |
| `RuntimeAnimationState` / `runtime_state_animations_internal.hpp` | `RuntimeAnimation`, animation vector/IDs/cancellation diagnostic, element animation state, frame timer/deadline/pending flags and pacing diagnostics |
| `RuntimeTaskState` / `runtime_state_tasks_internal.hpp` | task mutex, cross-thread entity queue, async-I/O registry, pool/group manager, `RuntimeTask`, completion records, diagnostics, vectors/IDs/drain guard; preserve parent task-core shutdown/capture fixes |
| `RuntimePlatformState` / `runtime_state_platform_internal.hpp` | native menu/dialog/URL/reopen results, accessibility cache, system appearance callback/bridge state, app/window themes |
| `RuntimeDiagnosticState` / `runtime_state_diagnostics_internal.hpp` | last render/frame/renderer/event/action records, frame/render/event/platform sequences and diagnostics; frame index stays authoritative in one group |

`runtime_state_internal.hpp` composes these groups and declares construction
and destruction; it is a private aggregate with no behavior dump. Any group
with incomplete `unique_ptr` pointees gets an out-of-line destructor in its
focused `.cpp`. Keep nested types whose public private method signatures use
them forward-declared; do not force complete definitions into public headers.

## Template bridge, before bulk state conversion

`runtime_templates.hpp` currently touches storage directly. Introduce focused
non-template methods first, while fields are still in their original location:

| Template family | Required private non-template operation |
| --- | --- |
| Entity insert/emplace/read/mutate/remove | find/ensure type-erased store by type index, lazy store factory, count mutation and existing change notification |
| Global set/read/mutate/update | get mutable/const `any`, set `any` plus existing invalidation semantics |
| Model/entity observers and subscription variants | validate entity in template, create typed callback in template, append erased observer / allocate subscription in implementation |
| Window/view observers and subscription variants | implementation validates ID and inserts observer with origin; do not change current observer semantics accidentally during layout migration |
| View-to-entity subscriptions | erased `(view_id, type_index, entity_id)` registration/deduplication |
| Element state/emplace/state-or-init | mutable/const root ElementTree accessor; typed construction/access stays template code |
| Cross-thread entity capability | return opaque shared queue owner through a non-template accessor, no direct queue field access |

Public template leafs still own construction of user `T`, `any_cast<T>`, typed
callback wrappers and return type deduction. No generic storage object or
private state reference is exposed as a public accessor.

Friend-access migration includes TaskGroup, TestAppState, cross-thread queue,
AsyncContextCapability, AsyncIoHook, TaskHandle and RuntimePopups. Search all
`WindowRuntime::` definitions (~100 source files at planning time) and friend
field accesses, not just the main runtime files.

## RED regression files and execution handoff

- `tests/architecture/runtime_public_state_isolation_test.cpp`: runtime sizeof
  assertion (buildable before migration), installed include closure, no complete
  runtime state or field-dependent templates anywhere under public include.
- `tests/header_cleanliness/runtime_installed_include_consumer.cpp`: concrete,
  non-template functions instantiate globals, entities/models, observations and
  subscriptions, view/window capabilities, cross-thread read/update, element
  state and typed action entry points. `main` is a no-op: this is a compile
  consumer, not a fake behavior test.
- Parent registers the filenames in their existing `[test] discover` tables.
  Parent owns all serial mcpp builds and captures RED/GREEN logs.
- In addition, create a fresh external scratch directory, copy ONLY `include/`
  and the consumer source, and compile as an mcpp static-library target with
  `include_dirs = ["include"]`, `sources = ["consumer.cpp"]`, C++23. Do not add
  checkout `src`, repo include directories, precompiled headers or source symlinks.
  Current code must fail on `../../../src/ui/window_runtime_internal.hpp`.
- After migration, repeat the same independent compile and link a consumer
  against packaged libraries; missing out-of-line bridge symbols require a
  link smoke test, not just successful object compilation.

## SDK packaging follow-up (parent-owned)

Current `docs/sdk-packaging.md` explicitly documents shipping a private source
header closure. `scripts/ci/sdk-package.py` currently implements that workaround:

1. Replace `private_header_closure` / `copy_private_headers` (around lines
   226-260) with validation that installed quoted header dependencies remain in
   `include/`. Do not silently retain a copier that masks future regressions.
2. `stage_macos` currently calls the copier while producing `private_headers`
   (around line 337); require no source headers in the staged package.
3. `finalize` copies that closure (around line 363), populates `private_headers`
   and `private_headers_reason` (around lines 384-387). Remove the reason and
   keep an empty field only if schema compatibility requires it; otherwise bump
   the manifest schema deliberately. Update matching manifest tests.
4. Update docs' package tree and the paragraph explaining the workaround to the
   new include+libraries contract. Update manifest field docs and macOS SDK
   guidance/fixtures if they assume private source headers are present.
5. Existing output directories may contain stale `src/` from older packaging.
   Stage in a clean package directory or explicitly reject those contents;
   avoid accepting a contaminated package that hides a missing-public-header bug.
6. Run the installed include consumer outside the checkout for Windows and Linux
   SDKs, plus native macOS when available. WSL does not establish native macOS
   correctness. Do not claim a clean committed SDK release from this dirty tree.

## Safe sequence

1. Finish current behavior/architecture GREEN before state migration.
2. Establish RED isolation and external include-only compile evidence.
3. Add focused template bridges and keep all current behavior suites GREEN.
4. Move one private field group at a time with explicit accesses; finish the
   single opaque pointer layout without changing ownership policies mid-move.
5. Update architecture assertions to check real ownership, not obsolete field
   locations; preserve old line limits and add group/bridge budgets.
6. Update packaging validation/docs, compile+link the external consumer, and run
   full relevant Windows/WSL suites. Report native-platform limits explicitly.
