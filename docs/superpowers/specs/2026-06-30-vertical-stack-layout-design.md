# Vertical Stack Layout Design

## Goal

Add the first simple container layout element: a vertical stack that lays out
owned children from top to bottom.

## API Shape

- `VerticalStackElement` derives from `Element`.
- `VerticalStackElement::append_child(std::unique_ptr<Element>)` owns children
  in insertion order and ignores null children.
- `VerticalStackElement::children()` exposes read-only child pointers for tests
  and future traversal work.
- `VerticalStackElement::layout(LayoutInput)` returns the max child width and
  summed child height, clamped by the stack input constraints.

## Behavior

- Empty stacks lay out to constrained zero size at zero origin.
- Each child receives unconstrained default `LayoutInput` in this first pass.
- Child output origins and persistent bounds are not stored yet.
- The stack output origin remains `{0, 0}`.
- This step does not add hit testing, retained layout boxes, pointer routing, or
  element-tree traversal.
