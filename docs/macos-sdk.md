# Using the macOS Phase J SDK

The SDK supports only Apple Silicon (arm64). There is no Intel or Universal build.
Extract the entire `cgpui-macos-arm64-release.tar.gz` archive. Keep `src/ui/`
alongside `include/`: some public headers include these implementation headers.
Use a C++23-capable Xcode toolchain, libc++, and the architecture named in
`manifest.json`. The SDK is built with a macOS 13.0 deployment target.
The demo and consumer were verified on macOS 27.2.1 (arm64), not on macOS 13.

To compile the included consumer in a directory outside the checkout:

```sh
sdk="$PWD/cgpui-macos-arm64-release"  # extracted package
cmake -S "$sdk/share/cgpui/sdk_consumer" -B consumer-build \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
  -Dcgpui_DIR="$sdk/lib/cmake/cgpui"
cmake --build consumer-build
CGPUI_EXIT_AFTER_FIRST_FRAME=1 ./consumer-build/sdk_consumer
```

The smoke run requires a graphical session with Metal. It checks an SVG pixel
before opening a Cocoa window and prints `sdk_consumer: first frame presented`
from the frame callback. Your own CMake project can use
`find_package(cgpui 0.1 REQUIRED CONFIG)` and link `cgpui::cgpui`. The
imported target supplies the headers, static archives (including LunaSVG and
plutovg) and Apple frameworks. For other build systems the link order and
framework names are in `manifest.json`; `release-manifest.json` lists file
sizes, modes and SHA-256 digests.

For xmake, copy `share/cgpui/sdk_consumer` into a working directory, change
into it, export `CGPUI_SDK` pointing at the extracted package, and run
`xmake f -m release --target_minver=13.0 && xmake`. Run the smoke with
`CGPUI_EXIT_AFTER_FIRST_FRAME=1 xmake run sdk_consumer`.
