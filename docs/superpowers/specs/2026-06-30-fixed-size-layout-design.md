# Fixed Size Layout Design

## Goal

Add the first real element layout pass for leaf elements with a preferred fixed
size.

## API Shape

- `Element::layout(LayoutInput)` returns a `LayoutOutput`.
- The base `Element` implementation returns zero size at zero origin after
  applying constraints.
- `FixedSizeElement` is a leaf element that stores a preferred `Size`.
- `FixedSizeElement::layout` returns its preferred size clamped through the
  incoming `LayoutConstraints`.

## Behavior

- A fixed-size element reports its preferred width and height when constraints
  allow them.
- Min and max constraints clamp each axis independently.
- Layout output origin remains `{0, 0}` for this leaf-only pass.
- This step does not add tree traversal, child layout, bounds persistence,
  vertical stacking, hit testing, or event routing.
