# Layout Primitives Design

## Goal

Add the first layout value types needed by later element layout passes.

## API Shape

- `LayoutConstraints` stores minimum and maximum allowed sizes.
- `LayoutInput` carries the constraints passed into a future element layout
  call.
- `LayoutOutput` stores the element's resolved size and origin.
- `constrain_size(Size, LayoutConstraints)` clamps a preferred size between the
  supplied minimum and maximum sizes.

## Behavior

- Default constraints allow zero-sized output and have no finite maximum.
- The clamp helper operates independently on width and height.
- Layout output defaults to a zero origin and zero size.
- This step does not add element layout methods, layout tree traversal, bounds
  storage, hit testing, or renderer integration.
