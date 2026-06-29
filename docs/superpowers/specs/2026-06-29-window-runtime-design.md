# Window Runtime Design

## Goal

Move the common window event-to-render loop out of `examples/hello_window/main.cpp` and into the CGPUI UI layer.

## Design

Add `cgpui::WindowRuntime`, a small owner that holds a `PlatformApplication`, `PlatformWindow`, `Renderer`, and `View`. It creates a window, creates the renderer from the native surface, routes resize/redraw/close platform events, and then runs the platform application.

The runtime stays intentionally narrow. Pointer, keyboard, focus, and text input events are ignored for now; future work can add a view event sink without changing platform backends again.

## Testing

Use pure C++ fakes for `PlatformApplication`, `PlatformWindow`, `Renderer`, `RenderFrame`, and `View`. The tests verify resize updates renderer and viewport, redraw paints and presents, close quits the app, and render failures stop the app with a non-zero result.
