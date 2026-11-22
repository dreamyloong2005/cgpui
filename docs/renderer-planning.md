# Renderer-neutral CPU planning

The renderer package owns CPU planning independently of the selected GPU backend.
New code includes a focused public leaf under `cgpui/renderer/planning/`:

| Leaf | Responsibility |
| --- | --- |
| `image_uploads.hpp` | Image descriptors and upload payload batches |
| `glyph_uploads.hpp` | Atlas upload regions and payload batches |
| `glyph_resources.hpp` | Atlas generation, dirty-range and resource plans |
| `glyph_geometry.hpp` | Glyph-cache consumption and textured glyph quads |
| `draw_geometry.hpp` | Rounded-rectangle, selection and caret geometry reports |
| `command_reports.hpp` | Command batching, submission plans and frame reports |

`cgpui/renderer/planning.hpp` is a declaration-only convenience aggregate. The
implementation is split by the same responsibilities under
`src/renderer/planning/`; private report assembly has its own small modules.
Glyph geometry uses the renderer's text/rasterization service. It does not
allocate GPU resources, import Vulkan headers or depend on UI implementation.

Vulkan consumes these plans and remains responsible for native image allocation,
descriptor binding, synchronization, command recording and presentation. Metal
or another backend can reuse the same CPU plans without linking the Vulkan
package. Existing Metal encoding behavior is not changed by this ownership move.

Historical `vulkan_*` CPU-planner declarations remain available through their
original public type headers. Focused `compat_*.cpp` files in the renderer
package forward them to the neutral implementation; they contain no second
algorithm. New production calls use the neutral names, such as
`plan_image_uploads` and `build_textured_glyph_quads`.

`tests/renderer-neutral` depends only on `cgpui.renderer`. It exercises both
neutral and compatibility entry points, and independently includes
`renderer_frame.hpp` to protect its standalone-header contract. GPU-specific
behavior remains in the backend suites. Architecture guards check the source
ownership and preserve the pre-existing focused-module size limits.
