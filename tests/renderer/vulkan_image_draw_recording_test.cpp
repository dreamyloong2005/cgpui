#include "vulkan_frame_draw_order_internal.hpp"
#include "vulkan_image_draw_recording_internal.hpp"

#include "cgpui/ui/image_builder.hpp"
#include "cgpui/ui/paint.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <type_traits>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }
  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

bool approximately_equal(float lhs, float rhs) {
  return std::abs(lhs - rhs) < 0.0001F;
}

template <typename Handle>
Handle fake_handle(std::uintptr_t value) {
  if constexpr (std::is_pointer_v<Handle>) {
    return reinterpret_cast<Handle>(value);
  } else {
    return static_cast<Handle>(value);
  }
}

cgpui::ImageAssetDescriptor descriptor(
    std::uint64_t id,
    std::uint32_t width = 100,
    std::uint32_t height = 50) {
  return cgpui::ImageAssetDescriptor{
      .id = cgpui::ImageAssetId{id},
      .pixel_width = width,
      .pixel_height = height,
      .format = cgpui::ImageFormat::rgba8_unorm,
  };
}

cgpui::VulkanImagePipelineResources pipeline_resources() {
  return cgpui::VulkanImagePipelineResources{
      .layout = fake_handle<VkPipelineLayout>(10),
      .pipeline = fake_handle<VkPipeline>(11),
  };
}

cgpui::VulkanImageVertexBufferResources vertex_buffer(
    std::size_t vertex_count) {
  return cgpui::VulkanImageVertexBufferResources{
      .vertices =
          cgpui::VulkanFrameGeometryBufferResources{
              .buffer = fake_handle<VkBuffer>(12),
              .memory = fake_handle<VkDeviceMemory>(13),
              .byte_capacity =
                  vertex_count * sizeof(cgpui::VulkanImageVertex),
              .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
          },
      .vertex_count = vertex_count,
      .byte_size = vertex_count * sizeof(cgpui::VulkanImageVertex),
  };
}

int test_public_sampling_propagation() {
  cgpui::AnyElement element =
      cgpui::image(descriptor(1, 4, 4))
          .sampling(cgpui::ImageSamplingMode::nearest)
          .size(cgpui::Size{.width = 16.0F, .height = 16.0F})
          .build();
  auto* image = dynamic_cast<cgpui::ImageElement*>(element.get());
  if (image == nullptr ||
      image->sampling() != cgpui::ImageSamplingMode::nearest) {
    return 10;
  }
  (void)image->layout(cgpui::LayoutInput{});
  cgpui::PaintList paint_list;
  image->paint(paint_list);
  return paint_list.commands().size() == 1 &&
                 paint_list.commands()[0].image.sampling ==
                     cgpui::ImageSamplingMode::nearest
             ? 0
             : 11;
}

int test_image_vertices_preserve_bounds_and_source_uvs() {
  const cgpui::ImageDraw draw{
      .bounds = cgpui::Rect{
          .origin = {.x = 10.0F, .y = 20.0F},
          .size = {.width = 40.0F, .height = 20.0F},
      },
      .asset = descriptor(2),
      .source_rect = cgpui::Rect{
          .origin = {.x = 20.0F, .y = 10.0F},
          .size = {.width = 40.0F, .height = 20.0F},
      },
  };
  const std::array<cgpui::ImageDraw, 1> draws{draw};
  const std::vector<cgpui::VulkanImageVertex> vertices =
      cgpui::vulkan_build_image_vertices(draws);
  if (vertices.size() != cgpui::vulkan_image_vertices_per_quad ||
      !approximately_equal(vertices[0].position[0], 10.0F) ||
      !approximately_equal(vertices[0].position[1], 20.0F) ||
      !approximately_equal(vertices[2].position[0], 50.0F) ||
      !approximately_equal(vertices[2].position[1], 40.0F)) {
    return 20;
  }
  return approximately_equal(vertices[0].image_uv[0], 0.2F) &&
                 approximately_equal(vertices[0].image_uv[1], 0.2F) &&
                 approximately_equal(vertices[2].image_uv[0], 0.6F) &&
                 approximately_equal(vertices[2].image_uv[1], 0.6F)
             ? 0
             : 21;
}

int test_pipeline_contract_and_shader_binaries() {
  const VkVertexInputBindingDescription binding =
      cgpui::vulkan_image_vertex_binding_description();
  const auto attributes =
      cgpui::vulkan_image_vertex_attribute_descriptions();
  const VkPipelineColorBlendAttachmentState blend =
      cgpui::vulkan_image_pipeline_blend_attachment_state();
  const auto vertex_spirv = cgpui::vulkan_image_vertex_shader_spirv();
  const auto fragment_spirv = cgpui::vulkan_image_fragment_shader_spirv();
  return binding.stride == sizeof(cgpui::VulkanImageVertex) &&
                 attributes.size() == 3 &&
                 attributes[0].format == VK_FORMAT_R32G32_SFLOAT &&
                 attributes[1].format == VK_FORMAT_R32G32_SFLOAT &&
                 attributes[2].format == VK_FORMAT_R32G32B32A32_SFLOAT &&
                 blend.blendEnable == VK_TRUE && !vertex_spirv.empty() &&
                 !fragment_spirv.empty() &&
                 vertex_spirv.front() == 0x07230203U &&
                 fragment_spirv.front() == 0x07230203U
             ? 0
             : 30;
}

int test_sampling_modes_select_descriptor_draws() {
  const cgpui::ImageAssetDescriptor first = descriptor(3);
  const cgpui::ImageAssetDescriptor second = descriptor(4);
  cgpui::VulkanImageTextureResources textures;
  textures.textures = {
      cgpui::VulkanImageTextureResource{
          .descriptor = first,
          .nearest_descriptor_set = fake_handle<VkDescriptorSet>(20),
          .linear_descriptor_set = fake_handle<VkDescriptorSet>(21),
          .layout = VK_IMAGE_LAYOUT_UNDEFINED,
      },
      cgpui::VulkanImageTextureResource{
          .descriptor = second,
          .nearest_descriptor_set = fake_handle<VkDescriptorSet>(22),
          .linear_descriptor_set = fake_handle<VkDescriptorSet>(23),
          .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      },
  };
  const std::array<cgpui::ImageDraw, 2> draws{
      cgpui::ImageDraw{
          .asset = first,
          .sampling = cgpui::ImageSamplingMode::nearest,
      },
      cgpui::ImageDraw{
          .asset = second,
          .sampling = cgpui::ImageSamplingMode::linear,
      },
  };
  cgpui::VulkanImageTextureUploadResources uploads;
  uploads.uploads.push_back(cgpui::VulkanImageTextureStagingUpload{
      .asset_id = first.id,
  });
  const auto commands = cgpui::vulkan_plan_image_draw_commands(
      draws, textures, pipeline_resources(), vertex_buffer(12), &uploads);
  if (!commands || commands->size() != 2 ||
      (*commands)[0].image_draw_index != 0 ||
      (*commands)[0].descriptor_set !=
          textures.textures[0].nearest_descriptor_set ||
      (*commands)[0].first_vertex != 0 ||
      (*commands)[0].vertex_count != 6 ||
      (*commands)[1].image_draw_index != 1 ||
      (*commands)[1].descriptor_set !=
          textures.textures[1].linear_descriptor_set ||
      (*commands)[1].first_vertex != 6 ||
      (*commands)[1].vertex_count != 6) {
    return 40;
  }

  const std::array<cgpui::ImageDraw, 1> unready{
      cgpui::ImageDraw{.asset = first},
  };
  const auto skipped = cgpui::vulkan_plan_image_draw_commands(
      unready, textures, pipeline_resources(), vertex_buffer(6));
  if (!skipped || !skipped->empty()) {
    return 41;
  }

  const std::array<cgpui::ImageDraw, 1> missing{
      cgpui::ImageDraw{.asset = descriptor(99)},
  };
  if (cgpui::vulkan_plan_image_draw_commands(
          missing, textures, pipeline_resources(), vertex_buffer(6))) {
    return 42;
  }
  const std::span<const cgpui::ImageDraw> empty;
  const auto no_commands = cgpui::vulkan_plan_image_draw_commands(
      empty,
      cgpui::VulkanImageTextureResources{},
      cgpui::VulkanImagePipelineResources{},
      cgpui::VulkanImageVertexBufferResources{});
  return no_commands && no_commands->empty() ? 0 : 43;
}

int test_stable_order_resolves_image_draws() {
  const std::array<cgpui::VulkanFrameDrawOrderEntry, 1> order{
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::image,
          .command_index = 0,
      },
  };
  const std::array<cgpui::VulkanImageDrawCommand, 1> image_commands{
      cgpui::VulkanImageDrawCommand{.image_draw_index = 0},
  };
  cgpui::VulkanFrameDrawOrderCursor cursor(
      order, {}, {}, {}, image_commands);
  const auto resolved = cursor.next();
  return resolved.has_value() &&
                 resolved->resource_kind ==
                     cgpui::VulkanFrameDrawResourceKind::image &&
                 resolved->resource_index == 0 && !cursor.next().has_value()
             ? 0
             : 50;
}

int test_structure_and_documentation() {
  const std::string pipeline_header = read_source(
      "src/renderer/vulkan/vulkan_image_pipeline_internal.hpp");
  const std::string pipeline_resources = read_source(
      "src/renderer/vulkan/vulkan_image_pipeline_resources.cpp");
  const std::string shaders = read_source(
      "src/renderer/vulkan/vulkan_image_shader_binaries.cpp");
  const std::string vertices = read_source(
      "src/renderer/vulkan/vulkan_image_vertex_buffer.cpp");
  const std::string recording = read_source(
      "src/renderer/vulkan/vulkan_image_draw_recording.cpp");
  const std::string ordered = read_source(
      "src/renderer/vulkan/vulkan_frame_draw_recording.cpp");
  const std::string command =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string swapchain =
      read_source("src/renderer/vulkan/vulkan_swapchain_create.cpp");
  const std::string builder =
      read_source("src/ui/widgets/image_builder.cpp");
  const std::string render_view = read_source("src/ui/render_view_commands.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (pipeline_header.empty() || pipeline_resources.empty() || shaders.empty() ||
      vertices.empty() || recording.empty() || ordered.empty() ||
      command.empty() || swapchain.empty() || builder.empty() ||
      render_view.empty() || structure.empty()) {
    return 60;
  }
  if (!contains(pipeline_resources, "vkCreateGraphicsPipelines") ||
      !contains(shaders, "vulkan_image_fragment_shader_spirv") ||
      !contains(vertices, "vulkan_apply_composed_transform") ||
      !contains(recording, "vkCmdBindDescriptorSets") ||
      !contains(recording, "vkCmdDraw") ||
      !contains(ordered, "VulkanFrameDrawResourceKind::image") ||
      !contains(command, "vulkan_plan_image_draw_commands") ||
      !contains(swapchain, "vulkan_create_image_pipeline_resources") ||
      !contains(builder, "ImageBuilder::sampling(") ||
      !contains(render_view, ".sampling = image.sampling") ||
      !contains(structure, "vulkan_image_draw_recording.cpp")) {
    return 61;
  }

  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 494",
      "image graphics pipeline",
      "sampling descriptor selection",
      "actual Vulkan image draw recording",
      "Step 495",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 70;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_public_sampling_propagation(); result != 0) {
    return result;
  }
  if (const int result = test_image_vertices_preserve_bounds_and_source_uvs();
      result != 0) {
    return result;
  }
  if (const int result = test_pipeline_contract_and_shader_binaries();
      result != 0) {
    return result;
  }
  if (const int result = test_sampling_modes_select_descriptor_draws();
      result != 0) {
    return result;
  }
  if (const int result = test_stable_order_resolves_image_draws(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
