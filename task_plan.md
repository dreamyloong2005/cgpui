# CGPUI GPUI-Core 60-Step Plan

## Goal

Reach the first 40 implementation steps toward a Windows/Linux GPUI-core-like API. Each step must be implemented as a small, verified slice that keeps `master` buildable.

## Current Baseline

- `master` starts at `779e62c feat: add pointer capture owner`.
- Windows and Linux use Vulkan.
- Linux platform target is Wayland.
- macOS/Metal is intentionally outside this 20-step goal except for not breaking existing source guards.

## Definition Of Done For This 20-Step Goal

- All 20 steps below are implemented, committed, merged to `master`, and verified.
- Windows full debug test suite passes after each merge.
- Linux/WSL full debug test suite passes for slices that touch shared UI, platform, renderer, or build surfaces.
- `git status --short --branch` on `master` has no tracked/staged changes from the work, aside from the pre-existing untracked `.vscode/`.

## Definition Of Done For The 40-Step Goal

- Steps 1-20 remain implemented and verified on `master`.
- Steps 21-40 below are implemented, committed, merged to `master`, and verified.
- Windows full debug test suite passes after each merge.
- Linux/WSL full debug test suite passes for slices that touch shared UI, platform, renderer, or build surfaces.
- `git status --short --branch` on `master` has no tracked/staged changes from the work, aside from the pre-existing untracked `.vscode/`.

## Definition Of Done For The 60-Step Goal

- Steps 1-40 remain implemented and verified on `master`.
- Steps 41-60 below are implemented, committed, merged to `master`, and verified.
- Windows full debug test suite passes after each merge.
- Linux/WSL full debug test suite passes for slices that touch shared UI, platform, renderer, or build surfaces.
- `git status --short --branch` on `master` has no tracked/staged changes from the work, aside from the pre-existing untracked `.vscode/`.

## Steps

1. [x] EventResult: make view event handlers return consumed/cancelled state.
2. [x] Event dispatch observability: expose last event result to runtime tests and callbacks where useful.
3. [x] Event routing shell: add a root-level dispatch function that can later route to element/view ids.
4. [x] EntityId and EntityStore skeleton: typed ids, insert/get/remove lifecycle.
5. [x] Context entity access: expose safe entity mutation/read helpers through a context object.
6. [x] View identity allocation: allocate stable ids beyond the root view.
7. [x] ElementId and Element base API: minimal polymorphic element contract.
8. [x] Element tree container: store children, parent links, and root element id.
9. [x] Element reconcile pass: preserve ids across simple rebuilds.
10. [x] LayoutInput/LayoutOutput primitives: constraints, size, origin.
11. [x] Layout pass for fixed-size leaf elements.
12. [x] Layout pass for simple vertical stack.
13. [x] Hit testing over laid-out element bounds.
14. [x] Pointer routing to hit-tested element id.
15. [x] Pointer capture routing to owner id.
16. [x] Keyboard routing to focus owner id.
17. [x] Action/Command primitive: named actions and dispatch result.
18. [x] Key binding table: map key events to actions.
19. [x] Basic text model: editable UTF-8 buffer with cursor.
20. [x] Text input routing into focused text model.
21. [x] Style primitives: colors, size, padding, and border fields.
22. [x] Element builder API skeleton for composing styled elements.
23. [x] Styled element paints a solid background rect.
24. [x] Padding participates in styled element layout.
25. [x] Border width/color and border radius primitives.
26. [x] Paint tree generation from element hierarchy.
27. [x] Clip rect and overflow primitives.
28. [x] Z-order and deterministic child paint order.
29. [x] Hover state tracking for routed pointer targets.
30. [x] Cursor shape API and routed cursor state.
31. [x] Scroll state/model primitive.
32. [x] Flex row/column layout basics.
33. [x] Runtime invalidation requests for layout and paint.
34. [x] Update scheduling that requests redraw after model/view changes.
35. [x] View/model subscription relation skeleton.
36. [x] Text selection range model.
37. [x] Text key editing actions for delete, movement, and selection.
38. [x] Clipboard abstraction for Win32 and Wayland.
39. [x] IME composition skeleton for Win32 and Wayland.
40. [x] Real interactive demo using the new API surface.
41. [x] ElementTree root layout helper.
42. [ ] ElementTree root hit-test helper.
43. [ ] Element tree owned runtime root installation.
44. [ ] Runtime lays out installed element trees on redraw.
45. [ ] Element event handler hook.
46. [ ] Runtime dispatches routed element events before view fallback.
47. [ ] Element builder flex row/column helpers.
48. [ ] Element builder vertical stack helper.
49. [ ] Element builder fixed-size helper.
50. [ ] Gap style primitive for stack and flex layout.
51. [ ] Margin style primitive and outer layout sizing.
52. [ ] Border paint emission from styled elements.
53. [ ] Hidden overflow clip metadata in paint commands.
54. [ ] Text element skeleton bound to TextModel.
55. [ ] Runtime text edit action routing for focused text models.
56. [ ] Runtime clipboard paste into focused text models.
57. [ ] Runtime clipboard copy from focused text selections.
58. [ ] Runtime clipboard cut from focused text selections.
59. [ ] ViewContext convenience wrapper for common runtime APIs.
60. [ ] Public app runner skeleton for GPUI-like startup.

## Active Step

Step 42: ElementTree root hit-test helper.

## Risks

- Keep old APIs source-compatible where practical while adding result-returning semantics.
- Avoid building a full view tree before the routing primitives exist.
- Keep each step small enough for Windows and WSL verification.
