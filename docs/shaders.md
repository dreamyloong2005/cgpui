# Shaders

Shaders are written in [Slang](https://shader-slang.org): the Vulkan
pipelines in `src/renderer/vulkan/shaders/*.slang`, the Metal renderer's in
`src/renderer/metal/shaders/cgpui_metal.slang`, and helpers both use in
`src/renderer/shaders/cgpui_shared.slang`. The build does not compile them.
After editing one, run

```bash
python scripts/dev/compile_shaders.py           # writes the outputs below
python scripts/dev/compile_shaders.py --check   # exit 1 if one is stale
```

with `slangc` and `spirv-val` from the Vulkan SDK (`VULKAN_SDK`, or `SLANGC` /
`SPIRV_VAL`; 2026.1 in SDK 1.4.341.1 here), and commit the outputs with the
sources:

- `src/renderer/vulkan/generated/<pipeline>_spirv.inc`: SPIR-V 1.3,
  validated for Vulkan 1.1, included by `vulkan_*_shader_binaries.cpp`;
- `src/renderer/metal/generated/<function>.metal`: one Metal source per
  function (slangc gives every entry point of one compile all global
  parameters), embedded by `packages/renderer-metal/build.mcpp`.

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
