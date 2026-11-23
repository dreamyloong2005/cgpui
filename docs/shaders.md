# Shaders

Shaders are written in [Slang](https://shader-slang.org): the Vulkan
pipelines in `src/renderer/vulkan/shaders/*.slang`, the Metal renderer's in
`src/renderer/metal/shaders/cgpui_metal.slang`, and helpers both use in
`src/renderer/shaders/cgpui_shared.slang`. The build does not compile them.
After editing one, run

```bash
python scripts/dev/compile_shaders.py           # writes the outputs below
python scripts/dev/compile_shaders.py --check   # exit 1 if one is stale
python scripts/dev/compile_shaders.py --metal   # Metal outputs only (no spirv-val)
python scripts/dev/compile_shaders.py --dxil    # Experimental DX12 rectangle outputs
```

with `slangc` and `spirv-val` from the Vulkan SDK (`VULKAN_SDK`, or `SLANGC` /
`SPIRV_VAL`; 2026.1 in SDK 1.4.341.1 here), and commit the outputs with the
sources:

- `src/renderer/vulkan/generated/<pipeline>_spirv.inc`: SPIR-V 1.3,
  validated for Vulkan 1.1, included by `vulkan_*_shader_binaries.cpp`;
- `src/renderer/metal/generated/<function>.metal`: one Metal source per
  function (slangc gives every entry point of one compile all global
  parameters), embedded by `packages/renderer-metal/build.mcpp`.
- `src/renderer/dx12/generated/rounded_rect_dxil.inc`: DXIL shader model 6.0,
  compiled from the same rectangle Slang source with `CGPUI_CLIP_Y_UP=1` and
  checked with `dxc -dumpbin` (`DXC` or the Vulkan SDK). The focused
  `compile_dx12_shaders.py` helper is included by the default/`--check` command;
  `--dxil --check` validates just this experimental slice. No runtime DXC is used.

Each output names its Slang sources with a hash of their text (FNV-1a 64,
LF line ends);
`tests/architecture/shader_source_structure_test.cpp` fails when a source
changed after its output was written. Arch Linux has no slangc package:
compile on Windows, or install the Vulkan SDK for Linux.

The Vulkan renderer asks for Vulkan 1.1, the version SPIR-V 1.3 needs:
slangc's SPIR-V backend starts at 1.3 (it warns that 1.0 to 1.2 are not
supported, or goes through GLSL with `-emit-spirv-via-glsl`, which is not
used). `vulkan_instance_surface.cpp` refuses a Vulkan 1.0 loader and
`vulkan_physical_device.cpp` skips devices below 1.1.

The Metal outputs were last generated on macOS with slangc 2026.1 (the
`shader-slang/slang` v2026.1 release, macOS arm64) and `--metal`. That
slangc rejects `fwidth` for the Metal target, so
`src/renderer/shaders/cgpui_box_shadow.slang` spells the pixel width with
`ddx` / `ddy` (`derivative_pixel_width`, the same value).
