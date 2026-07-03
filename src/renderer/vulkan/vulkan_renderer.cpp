#include "cgpui/renderer/renderer.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
#include <wayland-client.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#else
#include <vulkan/vulkan.h>
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <expected>
#include <cmath>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <variant>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

Error vulkan_error(ErrorCode code, std::string message) {
  return Error{.code = code, .message = std::move(message)};
}

Result<void> require_vk_success(VkResult result, std::string message) {
  if (result != VK_SUCCESS) {
    message += " (VkResult ";
    message += std::to_string(static_cast<int>(result));
    message += ")";
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed, std::move(message)));
  }

  return {};
}

Result<Win32SurfaceHandle> require_win32_surface(
    const NativeSurfaceHandle& native_surface) {
  const auto* surface = std::get_if<Win32SurfaceHandle>(&native_surface);
  if (surface == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires a Win32 native surface",
    });
  }

  if (surface->hinstance == nullptr || surface->hwnd == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires non-null Win32 handles",
    });
  }

  return *surface;
}

Result<WaylandSurfaceHandle> require_wayland_surface(
    const NativeSurfaceHandle& native_surface) {
  const auto* surface = std::get_if<WaylandSurfaceHandle>(&native_surface);
  if (surface == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires a Wayland native surface",
    });
  }

  if (surface->display == nullptr || surface->surface == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires non-null Wayland handles",
    });
  }

  return *surface;
}

[[nodiscard]] bool same_rect(Rect lhs, Rect rhs) {
  return lhs.origin.x == rhs.origin.x && lhs.origin.y == rhs.origin.y &&
         lhs.size.width == rhs.size.width && lhs.size.height == rhs.size.height;
}

[[nodiscard]] bool same_clip_rect(
    const std::optional<Rect>& lhs,
    const std::optional<Rect>& rhs) {
  if (lhs.has_value() != rhs.has_value()) {
    return false;
  }
  return !lhs.has_value() || same_rect(*lhs, *rhs);
}

[[nodiscard]] bool same_clip_stack(
    const RendererClipStackRecord& lhs,
    const RendererClipStackRecord& rhs) {
  if (lhs.full_depth != rhs.full_depth || lhs.truncated != rhs.truncated ||
      !same_clip_rect(lhs.current_clip_rect, rhs.current_clip_rect) ||
      lhs.clips.size() != rhs.clips.size()) {
    return false;
  }
  for (std::size_t index = 0; index < lhs.clips.size(); ++index) {
    if (!same_rect(lhs.clips[index], rhs.clips[index])) {
      return false;
    }
  }
  return true;
}

[[nodiscard]] bool same_composition_stack(
    const RendererCompositionStackRecord& lhs,
    const RendererCompositionStackRecord& rhs) {
  return lhs == rhs;
}

[[nodiscard]] bool same_batch_key(
    const RendererCommandBatchKey& lhs,
    const RendererCommandBatchKey& rhs) {
  return lhs.primitive_kind == rhs.primitive_kind &&
         same_clip_rect(lhs.clip_rect, rhs.clip_rect) &&
         same_clip_stack(lhs.clip_stack, rhs.clip_stack) &&
         same_composition_stack(lhs.composition_stack, rhs.composition_stack) &&
         lhs.metadata == rhs.metadata;
}

void append_command_batch(
    std::vector<RendererCommandBatch>& batches,
    RendererCommandBatchKey key,
    std::size_t command_index) {
  if (batches.empty() || !same_batch_key(batches.back().key, key)) {
    batches.push_back(RendererCommandBatch{.key = std::move(key)});
  }

  RendererCommandBatch& batch = batches.back();
  batch.command_count += 1;
  batch.command_indices.push_back(command_index);
}

struct TextDrawAtlasPageUsage {
  std::size_t page_index = 0;
  std::size_t glyph_quad_count = 0;
};

void append_text_draw_atlas_page_usage(
    std::vector<TextDrawAtlasPageUsage>& usages,
    std::size_t page_index) {
  for (TextDrawAtlasPageUsage& usage : usages) {
    if (usage.page_index == page_index) {
      usage.glyph_quad_count += 1;
      return;
    }
  }
  usages.push_back(TextDrawAtlasPageUsage{
      .page_index = page_index,
      .glyph_quad_count = 1,
  });
}

[[nodiscard]] TextSamplerPipelineDescriptor submission_pipeline_for(
    RendererPrimitiveKind primitive_kind,
    const TextSamplerPipelineDescriptor& text_sampler_pipeline) {
  if (primitive_kind == RendererPrimitiveKind::text) {
    return text_sampler_pipeline;
  }

  return TextSamplerPipelineDescriptor{
      .primitive_kind = primitive_kind,
      .uses_alpha_sampling = false,
      .uses_text_color = true,
  };
}

[[nodiscard]] bool same_submission_plan_key(
    const RendererSubmissionPlanKey& lhs,
    const RendererSubmissionPlanKey& rhs) {
  return lhs.primitive_kind == rhs.primitive_kind &&
         same_clip_rect(lhs.clip_rect, rhs.clip_rect) &&
         same_clip_stack(lhs.clip_stack, rhs.clip_stack) &&
         lhs.atlas_page_index == rhs.atlas_page_index;
}

void append_submission_plan_record(
    std::vector<RendererSubmissionPlanRecord>& records,
    RendererSubmissionPlanKey key,
    TextSamplerPipelineDescriptor pipeline,
    std::size_t batch_index,
    const RendererCommandBatch& batch,
    std::size_t glyph_quad_count) {
  if (records.empty() ||
      !same_submission_plan_key(records.back().key, key) ||
      records.back().pipeline != pipeline) {
    records.push_back(RendererSubmissionPlanRecord{
        .key = std::move(key),
        .pipeline = pipeline,
    });
  }

  RendererSubmissionPlanRecord& record = records.back();
  record.batch_count += 1;
  record.command_count += batch.command_count;
  record.glyph_quad_count += glyph_quad_count;
  record.batch_indices.push_back(batch_index);
  record.command_indices.insert(
      record.command_indices.end(),
      batch.command_indices.begin(),
      batch.command_indices.end());
}

[[nodiscard]] std::vector<RendererSubmissionPlanRecord>
build_renderer_submission_plan(
    std::span<const RendererCommandBatch> batches,
    std::span<const std::vector<TextDrawAtlasPageUsage>> text_draw_atlas_pages,
    const TextSamplerPipelineDescriptor& text_sampler_pipeline) {
  std::vector<RendererSubmissionPlanRecord> records;
  records.reserve(batches.size());

  for (std::size_t batch_index = 0; batch_index < batches.size();
       ++batch_index) {
    const RendererCommandBatch& batch = batches[batch_index];
    const TextSamplerPipelineDescriptor pipeline = submission_pipeline_for(
        batch.key.primitive_kind,
        text_sampler_pipeline);
    if (batch.key.primitive_kind != RendererPrimitiveKind::text) {
      append_submission_plan_record(
          records,
          RendererSubmissionPlanKey{
              .primitive_kind = batch.key.primitive_kind,
              .clip_rect = batch.key.clip_rect,
              .clip_stack = batch.key.clip_stack,
          },
          pipeline,
          batch_index,
          batch,
          0);
      continue;
    }

    bool appended_text_page = false;
    for (const std::size_t command_index : batch.command_indices) {
      if (command_index >= text_draw_atlas_pages.size()) {
        continue;
      }
      for (const TextDrawAtlasPageUsage& usage :
           text_draw_atlas_pages[command_index]) {
        append_submission_plan_record(
            records,
            RendererSubmissionPlanKey{
                .primitive_kind = batch.key.primitive_kind,
                .clip_rect = batch.key.clip_rect,
                .clip_stack = batch.key.clip_stack,
                .atlas_page_index = usage.page_index,
            },
            pipeline,
            batch_index,
            RendererCommandBatch{
                .key = batch.key,
                .command_count = 1,
                .command_indices = {command_index},
            },
            usage.glyph_quad_count);
        appended_text_page = true;
      }
    }

    if (!appended_text_page) {
      append_submission_plan_record(
          records,
          RendererSubmissionPlanKey{
              .primitive_kind = batch.key.primitive_kind,
              .clip_rect = batch.key.clip_rect,
              .clip_stack = batch.key.clip_stack,
          },
          pipeline,
          batch_index,
          batch,
          0);
    }
  }

  return records;
}

void record_submission_plan_statistics(RendererCommandReport& report) {
  report.submission_plan_record_count = report.submission_plan_records.size();
  report.submission_plan_command_count = 0;
  report.submission_plan_glyph_quad_count = 0;
  for (const RendererSubmissionPlanRecord& record :
       report.submission_plan_records) {
    report.submission_plan_command_count += record.command_count;
    report.submission_plan_glyph_quad_count += record.glyph_quad_count;
  }
}

[[nodiscard]] bool is_vulkan_supported_renderer_primitive(
    RendererPrimitiveKind primitive_kind) {
  return primitive_kind == RendererPrimitiveKind::solid_rect ||
         primitive_kind == RendererPrimitiveKind::rounded_rect ||
         primitive_kind == RendererPrimitiveKind::text ||
         primitive_kind == RendererPrimitiveKind::text_selection ||
         primitive_kind == RendererPrimitiveKind::text_caret ||
         primitive_kind == RendererPrimitiveKind::image;
}

[[nodiscard]] std::size_t positive_rounded_rect_corner_count(
    BorderRadii radius) {
  std::size_t count = 0;
  if (radius.top_left > 0.0F) {
    ++count;
  }
  if (radius.top_right > 0.0F) {
    ++count;
  }
  if (radius.bottom_right > 0.0F) {
    ++count;
  }
  if (radius.bottom_left > 0.0F) {
    ++count;
  }
  return count;
}

[[nodiscard]] Size atlas_page_size_for(
    const GlyphCache& glyph_cache,
    std::size_t page_index) {
  for (const GlyphAtlasPage& page : glyph_cache.atlas_pages()) {
    if (page.page_index == page_index) {
      return page.size;
    }
  }
  return Size{.width = 256.0F, .height = 256.0F};
}

[[nodiscard]] Rect normalized_atlas_bounds(Rect atlas_bounds, Size page_size) {
  const float width = page_size.width > 0.0F ? page_size.width : 1.0F;
  const float height = page_size.height > 0.0F ? page_size.height : 1.0F;
  return Rect{
      .origin =
          Point{
              .x = atlas_bounds.origin.x / width,
              .y = atlas_bounds.origin.y / height,
          },
      .size =
          Size{
              .width = atlas_bounds.size.width / width,
              .height = atlas_bounds.size.height / height,
          },
  };
}

[[nodiscard]] RendererUnsupportedCommandDiagnostic
make_unsupported_renderer_command_diagnostic(
    const RendererCommandStreamItem& command) {
  std::string message = "Vulkan renderer does not support ";
  message += renderer_primitive_kind_name(command.primitive_kind);
  message += " commands yet";
  return RendererUnsupportedCommandDiagnostic{
      .primitive_kind = command.primitive_kind,
      .command_index = command.command_index,
      .reason = RendererUnsupportedCommandReason::unsupported_primitive,
      .message = std::move(message),
  };
}

class VulkanRendererState;

class VulkanFrame final : public RenderFrame {
 public:
  explicit VulkanFrame(std::shared_ptr<VulkanRendererState> state)
      : state_(std::move(state)) {}
  void clear(Color color) override { clear_color_ = color; }
  void draw_rect(const SolidRect& rect) override { rects_.push_back(rect); }
  void draw_rounded_rect(const RoundedRectDraw& rect) override {
    rounded_rects_.push_back(rect);
  }
  void draw_text(const TextDraw& text) override { text_draws_.push_back(text); }
  void draw_text_selection(const TextSelectionDraw& selection) override {
    text_selections_.push_back(selection);
  }
  void draw_text_caret(const TextCaretDraw& caret) override {
    text_carets_.push_back(caret);
  }
  void draw_image(const ImageDraw& image) override {
    image_draws_.push_back(image);
  }
  Result<void> present() override;

 private:
  std::shared_ptr<VulkanRendererState> state_;
  Color clear_color_{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F};
  std::vector<SolidRect> rects_;
  std::vector<RoundedRectDraw> rounded_rects_;
  std::vector<TextDraw> text_draws_;
  std::vector<TextSelectionDraw> text_selections_;
  std::vector<TextCaretDraw> text_carets_;
  std::vector<ImageDraw> image_draws_;
};

struct QueueFamilies {
  std::uint32_t graphics = 0;
  std::uint32_t present = 0;
  bool has_graphics = false;
  bool has_present = false;
};

struct QueueFamilySearch {
  QueueFamilies families;
  bool suitable = false;
};

class VulkanRendererState final {
 public:
  ~VulkanRendererState() {
    if (device_ != VK_NULL_HANDLE) {
      vkDeviceWaitIdle(device_);
      destroy_swapchain();
      destroy_sync_objects();
      if (command_pool_ != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device_, command_pool_, nullptr);
      }
      vkDestroyDevice(device_, nullptr);
    }
    if (surface_ != VK_NULL_HANDLE) {
      vkDestroySurfaceKHR(instance_, surface_, nullptr);
    }
    if (instance_ != VK_NULL_HANDLE) {
      vkDestroyInstance(instance_, nullptr);
    }
  }

  static Result<std::shared_ptr<VulkanRendererState>> create(
      RenderSurfaceDescriptor descriptor) {
    auto renderer = std::shared_ptr<VulkanRendererState>(
        new VulkanRendererState(std::move(descriptor)));
    if (auto result = renderer->create_instance(); !result) {
      return std::unexpected(result.error());
    }
    if (auto result = renderer->create_surface(); !result) {
      return std::unexpected(result.error());
    }
    if (auto result = renderer->select_physical_device(); !result) {
      return std::unexpected(result.error());
    }
    if (auto result = renderer->create_device(); !result) {
      return std::unexpected(result.error());
    }
    if (auto result = renderer->create_command_pool(); !result) {
      return std::unexpected(result.error());
    }
    if (auto result = renderer->create_swapchain(); !result) {
      return std::unexpected(result.error());
    }
    if (auto result = renderer->create_sync_objects(); !result) {
      return std::unexpected(result.error());
    }

    return renderer;
  }

  Result<void> resize(Size framebuffer_size, DpiScale scale) {
    if (framebuffer_size.width <= 0.0F || framebuffer_size.height <= 0.0F) {
      return std::unexpected(Error{
          .code = ErrorCode::renderer_initialization_failed,
          .message = "Vulkan renderer requires a non-empty framebuffer",
      });
    }

    const RenderSurfaceDescriptor previous_descriptor = descriptor_;
    descriptor_.framebuffer_size = framebuffer_size;
    descriptor_.scale = scale;
    vkDeviceWaitIdle(device_);
    bool retired_old_swapchain = false;
    auto resources =
        create_swapchain_resources(swapchain_, &retired_old_swapchain);
    if (!resources) {
      descriptor_ = previous_descriptor;
      if (retired_old_swapchain) {
        destroy_swapchain();
        presentation_blocked_ = true;
      }
      return std::unexpected(resources.error());
    }

    destroy_swapchain();
    install_swapchain(std::move(*resources));
    presentation_blocked_ = false;
    return {};
  }

  Result<void> present_frame(
      Color color,
      std::span<const SolidRect> rects,
      std::span<const RoundedRectDraw> rounded_rects,
      std::span<const TextDraw> text_draws,
      std::span<const TextSelectionDraw> text_selections,
      std::span<const TextCaretDraw> text_carets) {
    if (presentation_blocked_) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "Vulkan presentation requires swapchain recreation"));
    }

    for (const TextDraw& text_draw : text_draws) {
      vulkan_consume_text_draw(text_draw, glyph_cache_);
    }
    last_command_batches_ =
        vulkan_build_renderer_command_batches(
            rects,
            rounded_rects,
            text_draws,
            text_selections,
            text_carets);

    if (auto result = require_vk_success(
            vkWaitForFences(device_, 1, &in_flight_, VK_TRUE, UINT64_MAX),
            "vkWaitForFences failed");
        !result) {
      return result;
    }

    for (std::uint32_t image_index = 0;
         image_index < command_buffers_.size();
         ++image_index) {
      if (auto result = record_frame_command_buffer(image_index, color, rects);
          !result) {
        return result;
      }
    }

    if (auto result = require_vk_success(
            vkResetFences(device_, 1, &in_flight_), "vkResetFences failed");
        !result) {
      return result;
    }

    std::uint32_t image_index = 0;
    const VkResult acquire_result = vkAcquireNextImageKHR(
        device_,
        swapchain_,
        UINT64_MAX,
        image_available_,
        VK_NULL_HANDLE,
        &image_index);
    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
      return recover_after_failed_acquire(
          ErrorCode::renderer_initialization_failed,
          "vkAcquireNextImageKHR reported an out-of-date swapchain",
          true);
    }
    if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR) {
      return recover_after_failed_acquire(
          ErrorCode::frame_acquisition_failed,
          "vkAcquireNextImageKHR failed",
          false);
    }
    const bool acquired_suboptimal = acquire_result == VK_SUBOPTIMAL_KHR;

    VkCommandBuffer command_buffer = command_buffers_[image_index];

    const VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &image_available_,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &render_finished_,
    };
    if (auto result = require_vk_success(
            vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_),
            "vkQueueSubmit failed");
        !result) {
      return recover_after_failed_submit(result.error().message);
    }

    const VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &render_finished_,
        .swapchainCount = 1,
        .pSwapchains = &swapchain_,
        .pImageIndices = &image_index,
    };
    const VkResult present_result =
        vkQueuePresentKHR(present_queue_, &present_info);
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR ||
        present_result == VK_SUBOPTIMAL_KHR) {
      presentation_blocked_ = true;
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "vkQueuePresentKHR reported an out-of-date swapchain"));
    }
    if (auto result =
            require_vk_success(present_result, "vkQueuePresentKHR failed");
        !result) {
      return recover_after_failed_present(result.error().message);
    }
    if (acquired_suboptimal) {
      presentation_blocked_ = true;
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "vkAcquireNextImageKHR reported a suboptimal swapchain"));
    }

    return {};
  }

 private:
  explicit VulkanRendererState(RenderSurfaceDescriptor descriptor)
      : descriptor_(std::move(descriptor)) {}

  struct SwapchainResources {
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent{};
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkCommandBuffer> command_buffers;
  };

  void destroy_swapchain_resources(SwapchainResources& resources) {
    if (device_ != VK_NULL_HANDLE && command_pool_ != VK_NULL_HANDLE &&
        !resources.command_buffers.empty()) {
      vkFreeCommandBuffers(
          device_,
          command_pool_,
          static_cast<std::uint32_t>(resources.command_buffers.size()),
          resources.command_buffers.data());
    }
    resources.command_buffers.clear();

    for (auto framebuffer : resources.framebuffers) {
      vkDestroyFramebuffer(device_, framebuffer, nullptr);
    }
    resources.framebuffers.clear();

    if (resources.render_pass != VK_NULL_HANDLE) {
      vkDestroyRenderPass(device_, resources.render_pass, nullptr);
      resources.render_pass = VK_NULL_HANDLE;
    }

    for (auto image_view : resources.image_views) {
      vkDestroyImageView(device_, image_view, nullptr);
    }
    resources.image_views.clear();
    resources.images.clear();

    if (resources.swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device_, resources.swapchain, nullptr);
      resources.swapchain = VK_NULL_HANDLE;
    }
    resources.format = VK_FORMAT_UNDEFINED;
    resources.extent = VkExtent2D{};
  }

  void destroy_sync_objects() {
    if (in_flight_ != VK_NULL_HANDLE) {
      vkDestroyFence(device_, in_flight_, nullptr);
      in_flight_ = VK_NULL_HANDLE;
    }
    if (render_finished_ != VK_NULL_HANDLE) {
      vkDestroySemaphore(device_, render_finished_, nullptr);
      render_finished_ = VK_NULL_HANDLE;
    }
    if (image_available_ != VK_NULL_HANDLE) {
      vkDestroySemaphore(device_, image_available_, nullptr);
      image_available_ = VK_NULL_HANDLE;
    }
  }

  void destroy_swapchain() {
    SwapchainResources resources{
        .swapchain = swapchain_,
        .format = swapchain_format_,
        .extent = swapchain_extent_,
        .images = std::move(swapchain_images_),
        .image_views = std::move(swapchain_image_views_),
        .render_pass = render_pass_,
        .framebuffers = std::move(framebuffers_),
        .command_buffers = std::move(command_buffers_),
    };
    swapchain_ = VK_NULL_HANDLE;
    swapchain_format_ = VK_FORMAT_UNDEFINED;
    swapchain_extent_ = VkExtent2D{};
    render_pass_ = VK_NULL_HANDLE;
    destroy_swapchain_resources(resources);
  }

  void install_swapchain(SwapchainResources&& resources) {
    swapchain_ = resources.swapchain;
    swapchain_format_ = resources.format;
    swapchain_extent_ = resources.extent;
    swapchain_images_ = std::move(resources.images);
    swapchain_image_views_ = std::move(resources.image_views);
    render_pass_ = resources.render_pass;
    framebuffers_ = std::move(resources.framebuffers);
    command_buffers_ = std::move(resources.command_buffers);
    resources.swapchain = VK_NULL_HANDLE;
    resources.render_pass = VK_NULL_HANDLE;
    resources.format = VK_FORMAT_UNDEFINED;
    resources.extent = VkExtent2D{};
  }

  [[nodiscard]] VkSurfaceFormatKHR choose_surface_format(
      std::span<const VkSurfaceFormatKHR> formats) const {
    const auto preferred = std::ranges::find_if(
        formats, [](const VkSurfaceFormatKHR& format) {
          return format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                 format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        });
    return preferred != formats.end() ? *preferred : formats.front();
  }

  [[nodiscard]] VkPresentModeKHR choose_present_mode(
      std::span<const VkPresentModeKHR> present_modes) const {
    const auto preferred =
        std::ranges::find(present_modes, VK_PRESENT_MODE_MAILBOX_KHR);
    return preferred != present_modes.end() ? *preferred
                                            : VK_PRESENT_MODE_FIFO_KHR;
  }

  [[nodiscard]] VkCompositeAlphaFlagBitsKHR choose_composite_alpha(
      VkCompositeAlphaFlagsKHR supported_alpha) const {
    constexpr std::array preferred_alpha{
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (VkCompositeAlphaFlagBitsKHR alpha : preferred_alpha) {
      if ((supported_alpha & alpha) != 0) {
        return alpha;
      }
    }

    return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  }

  [[nodiscard]] VkExtent2D choose_extent(
      const VkSurfaceCapabilitiesKHR& capabilities) const {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<std::uint32_t>::max()) {
      return capabilities.currentExtent;
    }

    return VkExtent2D{
        .width = std::clamp(
            static_cast<std::uint32_t>(descriptor_.framebuffer_size.width),
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width),
        .height = std::clamp(
            static_cast<std::uint32_t>(descriptor_.framebuffer_size.height),
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height),
    };
  }

  Result<void> create_command_pool() {
    const VkCommandPoolCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphics_queue_family_,
    };
    return require_vk_success(
        vkCreateCommandPool(device_, &create_info, nullptr, &command_pool_),
        "vkCreateCommandPool failed");
  }

  Result<void> create_sync_objects() {
    destroy_sync_objects();

    const VkSemaphoreCreateInfo semaphore_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    const VkFenceCreateInfo fence_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    if (auto result = require_vk_success(
            vkCreateSemaphore(
                device_, &semaphore_info, nullptr, &image_available_),
            "vkCreateSemaphore failed");
        !result) {
      destroy_sync_objects();
      return result;
    }
    if (auto result = require_vk_success(
            vkCreateSemaphore(
                device_, &semaphore_info, nullptr, &render_finished_),
            "vkCreateSemaphore failed");
        !result) {
      destroy_sync_objects();
      return result;
    }
    if (auto result = require_vk_success(
            vkCreateFence(device_, &fence_info, nullptr, &in_flight_),
            "vkCreateFence failed");
        !result) {
      destroy_sync_objects();
      return result;
    }

    return {};
  }

  Result<void> recover_after_failed_submit(std::string message) {
    presentation_blocked_ = true;
    (void)vkDeviceWaitIdle(device_);
    if (auto result = create_sync_objects(); !result) {
      return std::unexpected(result.error());
    }
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed, std::move(message)));
  }

  Result<void> recover_after_failed_acquire(
      ErrorCode code,
      std::string message,
      bool block_presentation) {
    presentation_blocked_ = block_presentation;
    if (auto result = create_sync_objects(); !result) {
      return std::unexpected(result.error());
    }
    return std::unexpected(vulkan_error(code, std::move(message)));
  }

  Result<void> recover_after_failed_present(std::string message) {
    presentation_blocked_ = true;
    (void)vkDeviceWaitIdle(device_);
    if (auto result = create_sync_objects(); !result) {
      return std::unexpected(result.error());
    }
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed, std::move(message)));
  }

  [[nodiscard]] bool make_clear_rect(
      const SolidRect& solid_rect,
      VkClearRect& clear_rect) const {
    if (solid_rect.rect.size.width <= 0.0F ||
        solid_rect.rect.size.height <= 0.0F) {
      return false;
    }

    const float framebuffer_width =
        static_cast<float>(swapchain_extent_.width);
    const float framebuffer_height =
        static_cast<float>(swapchain_extent_.height);
    float left =
        std::clamp(solid_rect.rect.origin.x, 0.0F, framebuffer_width);
    float top =
        std::clamp(solid_rect.rect.origin.y, 0.0F, framebuffer_height);
    float right = std::clamp(
        solid_rect.rect.origin.x + solid_rect.rect.size.width,
        0.0F,
        framebuffer_width);
    float bottom = std::clamp(
        solid_rect.rect.origin.y + solid_rect.rect.size.height,
        0.0F,
        framebuffer_height);

    if (solid_rect.clip_rect.has_value()) {
      const Rect& clip = *solid_rect.clip_rect;
      const float clip_left =
          std::clamp(clip.origin.x, 0.0F, framebuffer_width);
      const float clip_top =
          std::clamp(clip.origin.y, 0.0F, framebuffer_height);
      const float clip_right = std::clamp(
          clip.origin.x + clip.size.width, 0.0F, framebuffer_width);
      const float clip_bottom = std::clamp(
          clip.origin.y + clip.size.height, 0.0F, framebuffer_height);
      left = std::max(left, clip_left);
      top = std::max(top, clip_top);
      right = std::min(right, clip_right);
      bottom = std::min(bottom, clip_bottom);
    }

    if (right <= left || bottom <= top) {
      return false;
    }

    const auto pixel_left = static_cast<std::int32_t>(std::floor(left));
    const auto pixel_top = static_cast<std::int32_t>(std::floor(top));
    const auto pixel_right = static_cast<std::int32_t>(std::ceil(right));
    const auto pixel_bottom = static_cast<std::int32_t>(std::ceil(bottom));
    if (pixel_right <= pixel_left || pixel_bottom <= pixel_top) {
      return false;
    }

    clear_rect = VkClearRect{
        .rect =
            VkRect2D{
                .offset =
                    VkOffset2D{
                        .x = pixel_left,
                        .y = pixel_top,
                    },
                .extent =
                    VkExtent2D{
                        .width = static_cast<std::uint32_t>(
                            pixel_right - pixel_left),
                        .height = static_cast<std::uint32_t>(
                            pixel_bottom - pixel_top),
                    },
            },
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    return true;
  }

  Result<void> record_frame_command_buffer(
      std::uint32_t image_index,
      Color color,
      std::span<const SolidRect> rects) {
    VkCommandBuffer command_buffer = command_buffers_[image_index];
    if (auto result = require_vk_success(
            vkResetCommandBuffer(command_buffer, 0),
            "vkResetCommandBuffer failed");
        !result) {
      return result;
    }

    const VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    if (auto result = require_vk_success(
            vkBeginCommandBuffer(command_buffer, &begin_info),
            "vkBeginCommandBuffer failed");
        !result) {
      return result;
    }

    VkClearValue clear_value{};
    clear_value.color = VkClearColorValue{{color.r, color.g, color.b, color.a}};
    const VkRenderPassBeginInfo render_pass_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass_,
        .framebuffer = framebuffers_[image_index],
        .renderArea =
            VkRect2D{
                .offset = VkOffset2D{.x = 0, .y = 0},
                .extent = swapchain_extent_,
            },
        .clearValueCount = 1,
        .pClearValues = &clear_value,
    };
    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE);

    for (const SolidRect& rect : rects) {
      VkClearRect clear_rect{};
      if (!make_clear_rect(rect, clear_rect)) {
        continue;
      }

      VkClearAttachment attachment{};
      attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      attachment.colorAttachment = 0;
      attachment.clearValue.color = VkClearColorValue{{
          rect.color.r,
          rect.color.g,
          rect.color.b,
          rect.color.a,
      }};
      vkCmdClearAttachments(
          command_buffer, 1, &attachment, 1, &clear_rect);
    }

    vkCmdEndRenderPass(command_buffer);

    return require_vk_success(
        vkEndCommandBuffer(command_buffer), "vkEndCommandBuffer failed");
  }

  Result<void> create_swapchain() {
    auto resources = create_swapchain_resources(VK_NULL_HANDLE);
    if (!resources) {
      return std::unexpected(resources.error());
    }

    install_swapchain(std::move(*resources));
    return {};
  }

  Result<SwapchainResources> create_swapchain_resources(
      VkSwapchainKHR old_swapchain,
      bool* retired_old_swapchain = nullptr) {
    if (retired_old_swapchain != nullptr) {
      *retired_old_swapchain = false;
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    if (auto result = require_vk_success(
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                physical_device_, surface_, &capabilities),
            "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
        !result) {
      return std::unexpected(result.error());
    }

    if ((capabilities.supportedUsageFlags &
         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "Vulkan surface does not support color-attachment swapchain images"));
    }

    std::uint32_t format_count = 0;
    if (auto result = require_vk_success(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device_, surface_, &format_count, nullptr),
            "vkGetPhysicalDeviceSurfaceFormatsKHR failed");
        !result) {
      return std::unexpected(result.error());
    }
    if (format_count == 0) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "No Vulkan surface formats available"));
    }

    std::vector<VkSurfaceFormatKHR> formats(format_count);
    if (auto result = require_vk_success(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device_, surface_, &format_count, formats.data()),
            "vkGetPhysicalDeviceSurfaceFormatsKHR failed");
        !result) {
      return std::unexpected(result.error());
    }

    std::uint32_t present_mode_count = 0;
    if (auto result = require_vk_success(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device_, surface_, &present_mode_count, nullptr),
            "vkGetPhysicalDeviceSurfacePresentModesKHR failed");
        !result) {
      return std::unexpected(result.error());
    }
    if (present_mode_count == 0) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "No Vulkan surface present modes available"));
    }

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    if (auto result = require_vk_success(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device_, surface_, &present_mode_count,
                present_modes.data()),
            "vkGetPhysicalDeviceSurfacePresentModesKHR failed");
        !result) {
      return std::unexpected(result.error());
    }

    const VkSurfaceFormatKHR surface_format = choose_surface_format(formats);
    const VkPresentModeKHR present_mode = choose_present_mode(present_modes);
    const VkExtent2D extent = choose_extent(capabilities);
    const VkCompositeAlphaFlagBitsKHR composite_alpha =
        choose_composite_alpha(capabilities.supportedCompositeAlpha);

    std::uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount != 0 &&
        image_count > capabilities.maxImageCount) {
      image_count = capabilities.maxImageCount;
    }

    const std::array<std::uint32_t, 2> queue_family_indices{
        graphics_queue_family_,
        present_queue_family_,
    };
    const bool separate_queue_families =
        graphics_queue_family_ != present_queue_family_;

    const VkSwapchainCreateInfoKHR create_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface_,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = separate_queue_families
            ? VK_SHARING_MODE_CONCURRENT
            : VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = separate_queue_families
            ? static_cast<std::uint32_t>(queue_family_indices.size())
            : 0U,
        .pQueueFamilyIndices =
            separate_queue_families ? queue_family_indices.data() : nullptr,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = composite_alpha,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = old_swapchain,
    };

    SwapchainResources resources{
        .format = surface_format.format,
        .extent = extent,
    };
    if (auto result = require_vk_success(
            vkCreateSwapchainKHR(
                device_, &create_info, nullptr, &resources.swapchain),
            "vkCreateSwapchainKHR failed");
        !result) {
      return std::unexpected(result.error());
    }
    if (retired_old_swapchain != nullptr &&
        old_swapchain != VK_NULL_HANDLE) {
      *retired_old_swapchain = true;
    }

    std::uint32_t swapchain_image_count = 0;
    if (auto result = require_vk_success(
            vkGetSwapchainImagesKHR(
                device_, resources.swapchain, &swapchain_image_count, nullptr),
            "vkGetSwapchainImagesKHR failed");
        !result) {
      destroy_swapchain_resources(resources);
      return std::unexpected(result.error());
    }
    resources.images.resize(swapchain_image_count);
    if (auto result = require_vk_success(
            vkGetSwapchainImagesKHR(
                device_, resources.swapchain, &swapchain_image_count,
                resources.images.data()),
            "vkGetSwapchainImagesKHR failed");
        !result) {
      destroy_swapchain_resources(resources);
      return std::unexpected(result.error());
    }

    resources.image_views.reserve(resources.images.size());
    for (VkImage image : resources.images) {
      VkImageView image_view = VK_NULL_HANDLE;
      const VkImageViewCreateInfo image_view_info{
          .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .image = image,
          .viewType = VK_IMAGE_VIEW_TYPE_2D,
          .format = resources.format,
          .components =
              VkComponentMapping{
                  .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .a = VK_COMPONENT_SWIZZLE_IDENTITY,
              },
          .subresourceRange =
              VkImageSubresourceRange{
                  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                  .baseMipLevel = 0,
                  .levelCount = 1,
                  .baseArrayLayer = 0,
                  .layerCount = 1,
              },
      };
      if (auto result = require_vk_success(
              vkCreateImageView(
                  device_, &image_view_info, nullptr, &image_view),
              "vkCreateImageView failed");
          !result) {
        destroy_swapchain_resources(resources);
        return std::unexpected(result.error());
      }
      resources.image_views.push_back(image_view);
    }

    const VkAttachmentDescription color_attachment{
        .format = resources.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    const VkAttachmentReference color_attachment_reference{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    const VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_reference,
    };
    const VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };
    const VkRenderPassCreateInfo render_pass_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };
    if (auto result = require_vk_success(
            vkCreateRenderPass(
                device_, &render_pass_info, nullptr, &resources.render_pass),
            "vkCreateRenderPass failed");
        !result) {
      destroy_swapchain_resources(resources);
      return std::unexpected(result.error());
    }

    resources.framebuffers.reserve(resources.image_views.size());
    for (VkImageView image_view : resources.image_views) {
      VkFramebuffer framebuffer = VK_NULL_HANDLE;
      const VkFramebufferCreateInfo framebuffer_info{
          .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .renderPass = resources.render_pass,
          .attachmentCount = 1,
          .pAttachments = &image_view,
          .width = resources.extent.width,
          .height = resources.extent.height,
          .layers = 1,
      };
      if (auto result = require_vk_success(
              vkCreateFramebuffer(
                  device_, &framebuffer_info, nullptr, &framebuffer),
              "vkCreateFramebuffer failed");
          !result) {
        destroy_swapchain_resources(resources);
        return std::unexpected(result.error());
      }
      resources.framebuffers.push_back(framebuffer);
    }

    resources.command_buffers.resize(resources.images.size());
    const VkCommandBufferAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool_,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount =
            static_cast<std::uint32_t>(resources.command_buffers.size()),
    };
    if (auto result = require_vk_success(
            vkAllocateCommandBuffers(
                device_, &allocate_info, resources.command_buffers.data()),
            "vkAllocateCommandBuffers failed");
        !result) {
      resources.command_buffers.clear();
      destroy_swapchain_resources(resources);
      return std::unexpected(result.error());
    }

    return resources;
  }

  Result<void> create_instance() {
#if defined(_WIN32) || defined(__linux__)
#if defined(_WIN32)
    const std::array<const char*, 2> extensions{
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };
#else
    const std::array<const char*, 2> extensions{
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
    };
#endif

    const VkApplicationInfo app_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "CGPUI",
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "CGPUI",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };
    const VkInstanceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount =
            static_cast<std::uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    return require_vk_success(
        vkCreateInstance(&create_info, nullptr, &instance_),
        "vkCreateInstance failed");
#else
    return std::unexpected(vulkan_error(
        ErrorCode::unsupported_platform,
        "Vulkan renderer Win32 surface initialization requires Windows"));
#endif
  }

  Result<void> create_surface() {
#if defined(_WIN32)
    auto surface = require_win32_surface(descriptor_.native_surface);
    if (!surface) {
      return std::unexpected(surface.error());
    }
    const VkWin32SurfaceCreateInfoKHR create_info{
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = static_cast<HINSTANCE>(surface->hinstance),
        .hwnd = static_cast<HWND>(surface->hwnd),
    };

    return require_vk_success(
        vkCreateWin32SurfaceKHR(instance_, &create_info, nullptr, &surface_),
        "vkCreateWin32SurfaceKHR failed");
#elif defined(__linux__)
    auto surface = require_wayland_surface(descriptor_.native_surface);
    if (!surface) {
      return std::unexpected(surface.error());
    }
    const VkWaylandSurfaceCreateInfoKHR create_info{
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = static_cast<wl_display*>(surface->display),
        .surface = static_cast<wl_surface*>(surface->surface),
    };

    return require_vk_success(
        vkCreateWaylandSurfaceKHR(instance_, &create_info, nullptr, &surface_),
        "vkCreateWaylandSurfaceKHR failed");
#else
    return std::unexpected(vulkan_error(
        ErrorCode::unsupported_platform,
        "Vulkan renderer surface initialization requires Windows or Linux"));
#endif
  }

  Result<QueueFamilySearch> find_queue_families(VkPhysicalDevice device) const {
    std::uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queue_family_count, nullptr);
    if (queue_family_count == 0) {
      return QueueFamilySearch{};
    }

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queue_family_count, queue_families.data());

    QueueFamilies selected;
    for (std::uint32_t index = 0; index < queue_family_count; ++index) {
      const VkQueueFamilyProperties& properties = queue_families[index];
      if (!selected.has_graphics && properties.queueCount > 0 &&
          (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
        selected.graphics = index;
        selected.has_graphics = true;
      }

      VkBool32 present_supported = VK_FALSE;
      if (auto result = require_vk_success(
              vkGetPhysicalDeviceSurfaceSupportKHR(
                  device, index, surface_, &present_supported),
              "vkGetPhysicalDeviceSurfaceSupportKHR failed");
          !result) {
        return std::unexpected(result.error());
      }

      if (!selected.has_present && properties.queueCount > 0 &&
          present_supported == VK_TRUE) {
        selected.present = index;
        selected.has_present = true;
      }

      if (selected.has_graphics && selected.has_present) {
        return QueueFamilySearch{.families = selected, .suitable = true};
      }
    }

    return QueueFamilySearch{.families = selected, .suitable = false};
  }

  Result<bool> has_device_extension(
      VkPhysicalDevice device,
      const char* extension_name) const {
    std::uint32_t extension_count = 0;
    if (auto result = require_vk_success(
            vkEnumerateDeviceExtensionProperties(
                device, nullptr, &extension_count, nullptr),
            "vkEnumerateDeviceExtensionProperties failed");
        !result) {
      return std::unexpected(result.error());
    }

    std::vector<VkExtensionProperties> extensions(extension_count);
    if (auto result = require_vk_success(
            vkEnumerateDeviceExtensionProperties(
                device, nullptr, &extension_count, extensions.data()),
            "vkEnumerateDeviceExtensionProperties failed");
        !result) {
      return std::unexpected(result.error());
    }

    for (const auto& extension : extensions) {
      if (std::strcmp(extension.extensionName, extension_name) == 0) {
        return true;
      }
    }

    return false;
  }

  Result<void> select_physical_device() {
    std::uint32_t device_count = 0;
    if (auto result = require_vk_success(
            vkEnumeratePhysicalDevices(instance_, &device_count, nullptr),
            "vkEnumeratePhysicalDevices failed");
        !result) {
      return result;
    }

    if (device_count == 0) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "No Vulkan physical devices found"));
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    if (auto result = require_vk_success(
            vkEnumeratePhysicalDevices(instance_, &device_count, devices.data()),
            "vkEnumeratePhysicalDevices failed");
        !result) {
      return result;
    }

    for (VkPhysicalDevice device : devices) {
      auto queue_families = find_queue_families(device);
      if (!queue_families) {
        return std::unexpected(queue_families.error());
      }
      if (!queue_families->suitable) {
        continue;
      }

      auto supports_swapchain =
          has_device_extension(device, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      if (!supports_swapchain) {
        return std::unexpected(supports_swapchain.error());
      }
      if (!*supports_swapchain) {
        continue;
      }

      physical_device_ = device;
      graphics_queue_family_ = queue_families->families.graphics;
      present_queue_family_ = queue_families->families.present;
      return {};
    }

    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "No Vulkan physical device supports graphics, present, and swapchain"));
  }

  Result<void> create_device() {
    const float queue_priority = 1.0F;
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.reserve(
        graphics_queue_family_ == present_queue_family_ ? 1U : 2U);

    queue_create_infos.push_back(VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = graphics_queue_family_,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    });

    if (graphics_queue_family_ != present_queue_family_) {
      queue_create_infos.push_back(VkDeviceQueueCreateInfo{
          .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
          .queueFamilyIndex = present_queue_family_,
          .queueCount = 1,
          .pQueuePriorities = &queue_priority,
      });
    }

    const std::array<const char*, 1> device_extensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    const VkDeviceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount =
            static_cast<std::uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledExtensionCount =
            static_cast<std::uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
    };

    if (auto result = require_vk_success(
            vkCreateDevice(physical_device_, &create_info, nullptr, &device_),
            "vkCreateDevice failed");
        !result) {
      return result;
    }

    vkGetDeviceQueue(device_, graphics_queue_family_, 0, &graphics_queue_);
    vkGetDeviceQueue(device_, present_queue_family_, 0, &present_queue_);
    return {};
  }

  RenderSurfaceDescriptor descriptor_;
  VkInstance instance_ = VK_NULL_HANDLE;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  std::uint32_t graphics_queue_family_ = 0;
  std::uint32_t present_queue_family_ = 0;
  VkQueue graphics_queue_ = VK_NULL_HANDLE;
  VkQueue present_queue_ = VK_NULL_HANDLE;
  VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
  VkFormat swapchain_format_ = VK_FORMAT_UNDEFINED;
  VkExtent2D swapchain_extent_{};
  std::vector<VkImage> swapchain_images_;
  std::vector<VkImageView> swapchain_image_views_;
  VkRenderPass render_pass_ = VK_NULL_HANDLE;
  std::vector<VkFramebuffer> framebuffers_;
  VkCommandPool command_pool_ = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> command_buffers_;
  VkSemaphore image_available_ = VK_NULL_HANDLE;
  VkSemaphore render_finished_ = VK_NULL_HANDLE;
  VkFence in_flight_ = VK_NULL_HANDLE;
  GlyphCache glyph_cache_;
  std::vector<RendererCommandBatch> last_command_batches_;
  bool presentation_blocked_ = false;
};

class VulkanRenderer final : public Renderer {
 public:
  explicit VulkanRenderer(std::shared_ptr<VulkanRendererState> state)
      : state_(std::move(state)) {}

  Result<void> resize(Size framebuffer_size, DpiScale scale) override {
    return state_->resize(framebuffer_size, scale);
  }

  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    return std::make_unique<VulkanFrame>(state_);
  }

 private:
  std::shared_ptr<VulkanRendererState> state_;
};

} // namespace

Result<void> VulkanFrame::present() {
  return state_->present_frame(
      clear_color_,
      rects_,
      rounded_rects_,
      text_draws_,
      text_selections_,
      text_carets_);
}

void vulkan_consume_text_draw(const TextDraw& text, GlyphCache& glyph_cache) {
  (void)vulkan_build_textured_glyph_quads(text, glyph_cache);
}

std::vector<GlyphAtlasUploadBatch> vulkan_plan_glyph_atlas_uploads(
    std::span<const GlyphUploadRecord> upload_records,
    std::span<const GlyphAtlasPage> atlas_pages) {
  std::vector<GlyphAtlasUploadBatch> batches;
  for (const GlyphUploadRecord& upload : upload_records) {
    auto batch = std::ranges::find_if(
        batches,
        [&](const GlyphAtlasUploadBatch& candidate) {
          return candidate.image.page_index == upload.page_index;
        });
    if (batch == batches.end()) {
      Size page_size{.width = 256.0F, .height = 256.0F};
      const auto page = std::ranges::find_if(
          atlas_pages,
          [&](const GlyphAtlasPage& candidate) {
            return candidate.page_index == upload.page_index;
          });
      if (page != atlas_pages.end()) {
        page_size = page->size;
      }
      batches.push_back(GlyphAtlasUploadBatch{
          .image =
              GlyphAtlasImageDescriptor{
                  .page_index = upload.page_index,
                  .size = page_size,
              },
      });
      batch = batches.end() - 1;
    }

    const std::size_t byte_offset = batch->alpha.size();
    batch->alpha.insert(
        batch->alpha.end(),
        upload.alpha.begin(),
        upload.alpha.end());
    batch->uploads.push_back(GlyphAtlasUploadRegion{
        .key = upload.key,
        .page_index = upload.page_index,
        .atlas_bounds = upload.atlas_bounds,
        .width = upload.width,
        .height = upload.height,
        .stride = upload.stride,
        .byte_offset = byte_offset,
        .byte_size = upload.alpha.size(),
    });
    batch->image.upload_count = batch->uploads.size();
  }

  return batches;
}

std::vector<ImageUploadBatch> vulkan_plan_image_uploads(
    std::span<const ImageAsset> assets) {
  std::vector<ImageUploadBatch> batches;
  batches.reserve(assets.size());

  for (const ImageAsset& asset : assets) {
    ImageUploadBatch batch{
        .image = describe_image_asset(asset),
    };
    batch.rgba = asset.bitmap.pixels;
    batch.uploads.push_back(ImageUploadRegion{
        .asset_id = asset.id,
        .width = asset.bitmap.width,
        .height = asset.bitmap.height,
        .stride = asset.bitmap.stride,
        .byte_offset = 0,
        .byte_size = asset.bitmap.pixels.size(),
    });
    batches.push_back(std::move(batch));
  }

  return batches;
}

GlyphAtlasTextureResourcePlan vulkan_update_glyph_atlas_texture_resources(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches) {
  GlyphAtlasTextureResourcePlan plan;
  std::vector<GlyphAtlasTextureResourceRecord> next_resources;
  next_resources.reserve(upload_batches.size());

  for (const GlyphAtlasUploadBatch& batch : upload_batches) {
    auto existing = std::ranges::find_if(
        state.resources_,
        [&](const GlyphAtlasTextureResourceRecord& resource) {
          return resource.page_index == batch.image.page_index;
        });

    GlyphAtlasTextureResourceRecord resource{
        .page_index = batch.image.page_index,
        .image = batch.image,
    };
    resource.image.upload_count = batch.uploads.size();

    if (existing == state.resources_.end()) {
      resource.status = GlyphAtlasTextureResourceStatus::created;
      resource.generation = state.next_generation_++;
      ++plan.created_count;
    } else {
      resource.status = GlyphAtlasTextureResourceStatus::reused;
      resource.generation = existing->generation;
      ++plan.reused_count;
    }

    plan.live_resources.push_back(resource);
    next_resources.push_back(resource);
  }

  for (GlyphAtlasTextureResourceRecord resource : state.resources_) {
    const bool still_live =
        std::ranges::any_of(upload_batches, [&](const auto& batch) {
          return batch.image.page_index == resource.page_index;
        });
    if (!still_live) {
      resource.status = GlyphAtlasTextureResourceStatus::dropped;
      plan.dropped_resources.push_back(resource);
      ++plan.dropped_count;
    }
  }

  state.resources_ = std::move(next_resources);
  return plan;
}

std::vector<GlyphAtlasDirtyUploadRange> vulkan_plan_glyph_atlas_dirty_uploads(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches) {
  std::vector<GlyphAtlasDirtyUploadRange> dirty_ranges;
  for (const GlyphAtlasUploadBatch& batch : upload_batches) {
    auto existing = std::ranges::find_if(
        state.resources_,
        [&](const GlyphAtlasTextureResourceRecord& resource) {
          return resource.page_index == batch.image.page_index;
        });

    std::size_t first_upload_index = 0;
    if (existing != state.resources_.end()) {
      first_upload_index =
          std::min(existing->image.upload_count, batch.uploads.size());
    }
    if (first_upload_index >= batch.uploads.size()) {
      continue;
    }

    const GlyphAtlasUploadRegion& first_upload =
        batch.uploads[first_upload_index];
    const GlyphAtlasUploadRegion& last_upload = batch.uploads.back();
    dirty_ranges.push_back(GlyphAtlasDirtyUploadRange{
        .page_index = batch.image.page_index,
        .first_upload_index = first_upload_index,
        .upload_count = batch.uploads.size() - first_upload_index,
        .byte_offset = first_upload.byte_offset,
        .byte_size =
            last_upload.byte_offset + last_upload.byte_size -
            first_upload.byte_offset,
    });
  }

  (void)vulkan_update_glyph_atlas_texture_resources(state, upload_batches);
  return dirty_ranges;
}

std::vector<TexturedGlyphQuad> vulkan_build_textured_glyph_quads(
    const TextDraw& text,
    GlyphCache& glyph_cache) {
  std::vector<TexturedGlyphQuad> quads;
  quads.reserve(text.glyphs.size());

  for (const TextGlyphPaint& glyph : text.glyphs) {
    const GlyphCacheLookup lookup = glyph_cache.lookup(glyph.key);
    GlyphAtlasEntry entry;
    if (lookup.hit && lookup.entry.has_value()) {
      entry = *lookup.entry;
    } else {
      const GlyphAtlasAllocation allocation =
          glyph_cache.allocate(rasterize_fallback_glyph(glyph));
      entry = GlyphAtlasEntry{
          .key = allocation.key,
          .page_index = allocation.page_index,
          .atlas_bounds = allocation.atlas_bounds,
          .advance = glyph.device_advance,
      };
    }

    quads.push_back(TexturedGlyphQuad{
        .key = entry.key,
        .page_index = entry.page_index,
        .device_bounds =
            Rect{
                .origin = glyph.device_origin,
                .size = entry.atlas_bounds.size,
            },
        .atlas_bounds = entry.atlas_bounds,
        .atlas_uv_bounds = normalized_atlas_bounds(
            entry.atlas_bounds,
            atlas_page_size_for(glyph_cache, entry.page_index)),
        .color = text.color,
        .clip_rect = text.clip_rect,
        .clip_stack = text.clip_stack,
        .composition_stack = text.composition_stack,
        .metadata = text.metadata,
    });
  }

  return quads;
}

std::vector<RoundedRectTessellationRecord> vulkan_tessellate_rounded_rects(
    std::span<const RoundedRectDraw> rounded_rects) {
  std::vector<RoundedRectTessellationRecord> records;
  records.reserve(rounded_rects.size());

  for (const RoundedRectDraw& rounded_rect : rounded_rects) {
    constexpr std::size_t kCornerSegmentCount = 4;
    const std::size_t rounded_corner_count =
        positive_rounded_rect_corner_count(rounded_rect.radius);
    records.push_back(RoundedRectTessellationRecord{
        .rect = rounded_rect.rect,
        .color = rounded_rect.color,
        .radius = rounded_rect.radius,
        .clip_rect = rounded_rect.clip_rect,
        .clip_stack = rounded_rect.clip_stack,
        .composition_stack = rounded_rect.composition_stack,
        .metadata = rounded_rect.metadata,
        .corner_segment_count =
            rounded_corner_count == 0 ? 0 : kCornerSegmentCount,
        .vertex_count = 4 + rounded_corner_count * kCornerSegmentCount,
        .triangle_count = 2 + rounded_corner_count * kCornerSegmentCount,
    });
  }

  return records;
}

std::vector<TextSelectionGeometryRecord> vulkan_build_text_selection_geometry(
    std::span<const TextSelectionDraw> selections) {
  std::vector<TextSelectionGeometryRecord> records;
  records.reserve(selections.size());

  for (const TextSelectionDraw& selection : selections) {
    records.push_back(TextSelectionGeometryRecord{
        .rect = selection.rect,
        .color = selection.color,
        .range = selection.range,
        .font_size = selection.font_size,
        .clip_rect = selection.clip_rect,
        .clip_stack = selection.clip_stack,
        .composition_stack = selection.composition_stack,
        .metadata = selection.metadata,
        .vertex_count = 4,
        .triangle_count = 2,
    });
  }

  return records;
}

std::vector<TextCaretGeometryRecord> vulkan_build_text_caret_geometry(
    std::span<const TextCaretDraw> carets) {
  std::vector<TextCaretGeometryRecord> records;
  records.reserve(carets.size());

  for (const TextCaretDraw& caret : carets) {
    records.push_back(TextCaretGeometryRecord{
        .rect = caret.rect,
        .color = caret.color,
        .byte_offset = caret.byte_offset,
        .font_size = caret.font_size,
        .clip_rect = caret.clip_rect,
        .clip_stack = caret.clip_stack,
        .composition_stack = caret.composition_stack,
        .metadata = caret.metadata,
        .vertex_count = 4,
        .triangle_count = 2,
    });
  }

  return records;
}

std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets) {
  std::vector<RendererCommandStreamItem> commands;
  commands.reserve(
      rects.size() + rounded_rects.size() + text_draws.size() +
      selections.size() + carets.size());

  for (std::size_t index = 0; index < rects.size(); ++index) {
    const SolidRect& rect = rects[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::solid_rect,
        .command_index = index,
        .clip_rect = rect.clip_rect,
        .clip_stack = rect.clip_stack,
        .composition_stack = rect.composition_stack,
        .metadata = rect.metadata,
    });
  }

  for (std::size_t index = 0; index < rounded_rects.size(); ++index) {
    const RoundedRectDraw& rect = rounded_rects[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::rounded_rect,
        .command_index = index,
        .clip_rect = rect.clip_rect,
        .clip_stack = rect.clip_stack,
        .composition_stack = rect.composition_stack,
        .metadata = rect.metadata,
    });
  }

  for (std::size_t index = 0; index < text_draws.size(); ++index) {
    const TextDraw& text_draw = text_draws[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::text,
        .command_index = index,
        .clip_rect = text_draw.clip_rect,
        .clip_stack = text_draw.clip_stack,
        .composition_stack = text_draw.composition_stack,
        .metadata = text_draw.metadata,
    });
  }

  for (std::size_t index = 0; index < selections.size(); ++index) {
    const TextSelectionDraw& selection = selections[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::text_selection,
        .command_index = index,
        .clip_rect = selection.clip_rect,
        .clip_stack = selection.clip_stack,
        .composition_stack = selection.composition_stack,
        .metadata = selection.metadata,
    });
  }

  for (std::size_t index = 0; index < carets.size(); ++index) {
    const TextCaretDraw& caret = carets[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::text_caret,
        .command_index = index,
        .clip_rect = caret.clip_rect,
        .clip_stack = caret.clip_stack,
        .composition_stack = caret.composition_stack,
        .metadata = caret.metadata,
    });
  }

  return vulkan_build_renderer_command_report(commands).batches;
}

std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws) {
  return vulkan_build_renderer_command_batches(
      rects,
      rounded_rects,
      text_draws,
      std::span<const TextSelectionDraw>{},
      std::span<const TextCaretDraw>{});
}

std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws) {
  return vulkan_build_renderer_command_batches(
      rects,
      std::span<const RoundedRectDraw>{},
      text_draws);
}

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const RendererCommandStreamItem> commands) {
  RendererCommandReport report;
  report.batches.reserve(commands.size());

  for (const RendererCommandStreamItem& command : commands) {
    if (!is_vulkan_supported_renderer_primitive(command.primitive_kind)) {
      report.unsupported_commands.push_back(
          make_unsupported_renderer_command_diagnostic(command));
      report.unsupported_command_count += 1;
      continue;
    }

    append_command_batch(
        report.batches,
        RendererCommandBatchKey{
            .primitive_kind = command.primitive_kind,
            .clip_rect = command.clip_rect,
            .clip_stack = command.clip_stack,
            .composition_stack = command.composition_stack,
            .metadata = command.metadata,
        },
        command.command_index);
    report.supported_command_count += 1;
    if (!command.clip_stack.empty()) {
      report.clip_stack_record_count += 1;
      report.max_clip_stack_depth = std::max(
          report.max_clip_stack_depth,
          command.clip_stack.full_depth);
    }
    if (!command.composition_stack.empty()) {
      report.composition_stack_record_count += 1;
      report.max_composition_stack_depth = std::max(
          report.max_composition_stack_depth,
          command.composition_stack.full_depth);
    }
  }

  report.submission_plan_records = build_renderer_submission_plan(
      report.batches,
      std::span<const std::vector<TextDrawAtlasPageUsage>>{},
      report.text_render.text_sampler_pipeline);
  record_submission_plan_statistics(report);
  return report;
}

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache) {
  return vulkan_build_renderer_command_report(
      rects,
      rounded_rects,
      text_draws,
      selections,
      carets,
      std::span<const ImageDraw>{},
      glyph_cache);
}

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    std::span<const ImageDraw> image_draws,
    GlyphCache& glyph_cache) {
  std::vector<RendererCommandStreamItem> commands;
  commands.reserve(
      rects.size() + rounded_rects.size() + text_draws.size() +
      selections.size() + carets.size() + image_draws.size());

  for (std::size_t index = 0; index < rects.size(); ++index) {
    const SolidRect& rect = rects[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::solid_rect,
        .command_index = index,
        .clip_rect = rect.clip_rect,
        .clip_stack = rect.clip_stack,
        .composition_stack = rect.composition_stack,
        .metadata = rect.metadata,
    });
  }

  for (std::size_t index = 0; index < rounded_rects.size(); ++index) {
    const RoundedRectDraw& rect = rounded_rects[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::rounded_rect,
        .command_index = index,
        .clip_rect = rect.clip_rect,
        .clip_stack = rect.clip_stack,
        .composition_stack = rect.composition_stack,
        .metadata = rect.metadata,
    });
  }

  for (std::size_t index = 0; index < text_draws.size(); ++index) {
    const TextDraw& text_draw = text_draws[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::text,
        .command_index = index,
        .clip_rect = text_draw.clip_rect,
        .clip_stack = text_draw.clip_stack,
        .composition_stack = text_draw.composition_stack,
        .metadata = text_draw.metadata,
    });
  }

  for (std::size_t index = 0; index < selections.size(); ++index) {
    const TextSelectionDraw& selection = selections[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::text_selection,
        .command_index = index,
        .clip_rect = selection.clip_rect,
        .clip_stack = selection.clip_stack,
        .composition_stack = selection.composition_stack,
        .metadata = selection.metadata,
    });
  }

  for (std::size_t index = 0; index < carets.size(); ++index) {
    const TextCaretDraw& caret = carets[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::text_caret,
        .command_index = index,
        .clip_rect = caret.clip_rect,
        .clip_stack = caret.clip_stack,
        .composition_stack = caret.composition_stack,
        .metadata = caret.metadata,
    });
  }

  for (std::size_t index = 0; index < image_draws.size(); ++index) {
    const ImageDraw& image = image_draws[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::image,
        .command_index = index,
        .clip_rect = image.clip_rect,
        .clip_stack = image.clip_stack,
        .composition_stack = image.composition_stack,
        .metadata = image.metadata,
    });
  }

  RendererCommandReport report = vulkan_build_renderer_command_report(commands);
  report.rounded_rect_tessellations =
      vulkan_tessellate_rounded_rects(rounded_rects);
  report.rounded_rect_tessellation_count =
      report.rounded_rect_tessellations.size();
  report.text_selection_geometries =
      vulkan_build_text_selection_geometry(selections);
  report.text_selection_geometry_count =
      report.text_selection_geometries.size();
  report.text_caret_geometries = vulkan_build_text_caret_geometry(carets);
  report.text_caret_geometry_count = report.text_caret_geometries.size();
  std::vector<std::vector<TextDrawAtlasPageUsage>> text_draw_atlas_pages(
      text_draws.size());
  for (std::size_t text_index = 0; text_index < text_draws.size();
       ++text_index) {
    const TextDraw& text_draw = text_draws[text_index];
    report.text_render.text_draw_count += 1;
    const std::size_t lookup_begin = glyph_cache.lookups().size();
    const std::size_t upload_begin = glyph_cache.upload_records().size();
    const std::vector<TexturedGlyphQuad> quads =
        vulkan_build_textured_glyph_quads(text_draw, glyph_cache);
    const std::size_t upload_count =
        glyph_cache.upload_records().size() - upload_begin;
    for (const TexturedGlyphQuad& quad : quads) {
      append_text_draw_atlas_page_usage(
          text_draw_atlas_pages[text_index],
          quad.page_index);
    }

    for (std::size_t index = lookup_begin; index < glyph_cache.lookups().size();
         ++index) {
      if (glyph_cache.lookups()[index].hit) {
        report.text_render.glyph_cache_hit_count += 1;
      }
    }

    report.text_render.rasterized_glyph_count += upload_count;
    report.text_render.glyph_upload_record_count += upload_count;
    report.text_render.textured_glyph_quad_count += quads.size();
    if (quads.empty()) {
      report.text_render.metadata_only_text_draw_count += 1;
    } else {
      report.text_render.glyph_backed_text_draw_count += 1;
      report.text_render.text_sampler_pipeline_descriptor_count = 1;
      if (report.text_render.text_sampler_pipeline.ready()) {
        report.text_render.text_sampler_pipeline_ready_text_draw_count += 1;
      } else {
        report.text_render.text_sampler_pipeline_pending_text_draw_count += 1;
      }
    }
  }

  for (const ImageDraw& image_draw : image_draws) {
    report.image_render.image_draw_count += 1;
    report.image_render.image_upload_plan_count += 1;
    report.image_render.image_upload_byte_count += image_draw.asset.byte_size;
  }

  report.submission_plan_records = build_renderer_submission_plan(
      report.batches,
      text_draw_atlas_pages,
      report.text_render.text_sampler_pipeline);
  record_submission_plan_statistics(report);
  return report;
}

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  return vulkan_build_renderer_command_report(
      rects,
      rounded_rects,
      text_draws,
      std::span<const TextSelectionDraw>{},
      std::span<const TextCaretDraw>{},
      glyph_cache);
}

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  return vulkan_build_renderer_command_report(
      rects,
      std::span<const RoundedRectDraw>{},
      text_draws,
      std::span<const TextSelectionDraw>{},
      std::span<const TextCaretDraw>{},
      glyph_cache);
}

RendererFrameReport vulkan_build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache) {
  return renderer_frame_report_from_command_report(
      vulkan_build_renderer_command_report(
          rects,
          rounded_rects,
          text_draws,
          selections,
          carets,
          glyph_cache));
}

RendererFrameReport vulkan_build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache) {
  return vulkan_build_renderer_frame_report(
      rects,
      std::span<const RoundedRectDraw>{},
      text_draws,
      std::span<const TextSelectionDraw>{},
      std::span<const TextCaretDraw>{},
      glyph_cache);
}

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor) {
  if (descriptor.framebuffer_size.width <= 0.0F ||
      descriptor.framebuffer_size.height <= 0.0F) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires a non-empty framebuffer",
    });
  }

  auto state = VulkanRendererState::create(descriptor);
  if (!state) {
    return std::unexpected(state.error());
  }

  return std::make_unique<VulkanRenderer>(std::move(*state));
}

} // namespace cgpui
