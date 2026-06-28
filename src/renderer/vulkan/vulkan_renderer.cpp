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
#else
#include <vulkan/vulkan.h>
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <expected>
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

class VulkanRenderer;

class VulkanFrame final : public RenderFrame {
 public:
  explicit VulkanFrame(VulkanRenderer& renderer) : renderer_(renderer) {}
  void clear(Color color) override { clear_color_ = color; }
  void draw_rect(const SolidRect& rect) override { rect_ = rect; }
  Result<void> present() override;

 private:
  VulkanRenderer& renderer_;
  Color clear_color_{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F};
  SolidRect rect_{};
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

class VulkanRenderer final : public Renderer {
 public:
  ~VulkanRenderer() override {
    if (device_ != VK_NULL_HANDLE) {
      vkDeviceWaitIdle(device_);
      destroy_swapchain();
      if (in_flight_ != VK_NULL_HANDLE) {
        vkDestroyFence(device_, in_flight_, nullptr);
      }
      if (render_finished_ != VK_NULL_HANDLE) {
        vkDestroySemaphore(device_, render_finished_, nullptr);
      }
      if (image_available_ != VK_NULL_HANDLE) {
        vkDestroySemaphore(device_, image_available_, nullptr);
      }
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

  static Result<std::unique_ptr<VulkanRenderer>> create(
      RenderSurfaceDescriptor descriptor,
      Win32SurfaceHandle native_surface) {
    auto renderer =
        std::unique_ptr<VulkanRenderer>(new VulkanRenderer(std::move(descriptor)));
    if (auto result = renderer->create_instance(); !result) {
      return std::unexpected(result.error());
    }
    if (auto result = renderer->create_surface(native_surface); !result) {
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

  Result<void> resize(Size framebuffer_size, DpiScale scale) override {
    if (framebuffer_size.width <= 0.0F || framebuffer_size.height <= 0.0F) {
      return std::unexpected(Error{
          .code = ErrorCode::renderer_initialization_failed,
          .message = "Vulkan renderer requires a non-empty framebuffer",
      });
    }

    descriptor_.framebuffer_size = framebuffer_size;
    descriptor_.scale = scale;
    vkDeviceWaitIdle(device_);
    destroy_swapchain();
    return create_swapchain();
  }

  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    return std::make_unique<VulkanFrame>(*this);
  }

  Result<void> present_clear(Color color) {
    if (auto result = require_vk_success(
            vkWaitForFences(device_, 1, &in_flight_, VK_TRUE, UINT64_MAX),
            "vkWaitForFences failed");
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
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "vkAcquireNextImageKHR reported an out-of-date swapchain"));
    }
    if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR) {
      return std::unexpected(vulkan_error(
          ErrorCode::frame_acquisition_failed, "vkAcquireNextImageKHR failed"));
    }

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

    transition_image(
        command_buffer,
        swapchain_images_[image_index],
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT);

    const VkClearColorValue clear_color{{
        color.r,
        color.g,
        color.b,
        color.a,
    }};
    const VkImageSubresourceRange range{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    vkCmdClearColorImage(
        command_buffer,
        swapchain_images_[image_index],
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        &clear_color,
        1,
        &range);

    transition_image(
        command_buffer,
        swapchain_images_[image_index],
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        0,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

    if (auto result = require_vk_success(
            vkEndCommandBuffer(command_buffer), "vkEndCommandBuffer failed");
        !result) {
      return result;
    }

    if (auto result = require_vk_success(
            vkResetFences(device_, 1, &in_flight_), "vkResetFences failed");
        !result) {
      return result;
    }

    const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
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
      return result;
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
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "vkQueuePresentKHR reported an out-of-date swapchain"));
    }

    return require_vk_success(present_result, "vkQueuePresentKHR failed");
  }

 private:
  explicit VulkanRenderer(RenderSurfaceDescriptor descriptor)
      : descriptor_(std::move(descriptor)) {}

  void destroy_swapchain() {
    for (auto image_view : swapchain_image_views_) {
      vkDestroyImageView(device_, image_view, nullptr);
    }
    swapchain_image_views_.clear();
    swapchain_images_.clear();
    command_buffers_.clear();
    if (swapchain_ != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device_, swapchain_, nullptr);
      swapchain_ = VK_NULL_HANDLE;
    }
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
      return result;
    }
    if (auto result = require_vk_success(
            vkCreateSemaphore(
                device_, &semaphore_info, nullptr, &render_finished_),
            "vkCreateSemaphore failed");
        !result) {
      return result;
    }
    return require_vk_success(
        vkCreateFence(device_, &fence_info, nullptr, &in_flight_),
        "vkCreateFence failed");
  }

  Result<void> create_swapchain() {
    VkSurfaceCapabilitiesKHR capabilities{};
    if (auto result = require_vk_success(
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                physical_device_, surface_, &capabilities),
            "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
        !result) {
      return result;
    }

    std::uint32_t format_count = 0;
    if (auto result = require_vk_success(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device_, surface_, &format_count, nullptr),
            "vkGetPhysicalDeviceSurfaceFormatsKHR failed");
        !result) {
      return result;
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
      return result;
    }

    std::uint32_t present_mode_count = 0;
    if (auto result = require_vk_success(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device_, surface_, &present_mode_count, nullptr),
            "vkGetPhysicalDeviceSurfacePresentModesKHR failed");
        !result) {
      return result;
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
      return result;
    }

    const VkSurfaceFormatKHR surface_format = choose_surface_format(formats);
    const VkPresentModeKHR present_mode = choose_present_mode(present_modes);
    const VkExtent2D extent = choose_extent(capabilities);

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
        .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = separate_queue_families
            ? VK_SHARING_MODE_CONCURRENT
            : VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = separate_queue_families
            ? static_cast<std::uint32_t>(queue_family_indices.size())
            : 0U,
        .pQueueFamilyIndices =
            separate_queue_families ? queue_family_indices.data() : nullptr,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    if (auto result = require_vk_success(
            vkCreateSwapchainKHR(device_, &create_info, nullptr, &swapchain_),
            "vkCreateSwapchainKHR failed");
        !result) {
      return result;
    }
    swapchain_format_ = surface_format.format;
    swapchain_extent_ = extent;

    std::uint32_t swapchain_image_count = 0;
    if (auto result = require_vk_success(
            vkGetSwapchainImagesKHR(
                device_, swapchain_, &swapchain_image_count, nullptr),
            "vkGetSwapchainImagesKHR failed");
        !result) {
      return result;
    }
    swapchain_images_.resize(swapchain_image_count);
    if (auto result = require_vk_success(
            vkGetSwapchainImagesKHR(
                device_, swapchain_, &swapchain_image_count,
                swapchain_images_.data()),
            "vkGetSwapchainImagesKHR failed");
        !result) {
      return result;
    }

    swapchain_image_views_.reserve(swapchain_images_.size());
    for (VkImage image : swapchain_images_) {
      VkImageView image_view = VK_NULL_HANDLE;
      const VkImageViewCreateInfo image_view_info{
          .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .image = image,
          .viewType = VK_IMAGE_VIEW_TYPE_2D,
          .format = swapchain_format_,
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
        return result;
      }
      swapchain_image_views_.push_back(image_view);
    }

    command_buffers_.resize(swapchain_images_.size());
    const VkCommandBufferAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool_,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount =
            static_cast<std::uint32_t>(command_buffers_.size()),
    };
    return require_vk_success(
        vkAllocateCommandBuffers(device_, &allocate_info, command_buffers_.data()),
        "vkAllocateCommandBuffers failed");
  }

  void transition_image(
      VkCommandBuffer command_buffer,
      VkImage image,
      VkImageLayout old_layout,
      VkImageLayout new_layout,
      VkAccessFlags src_access,
      VkAccessFlags dst_access,
      VkPipelineStageFlags src_stage,
      VkPipelineStageFlags dst_stage) {
    const VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = src_access,
        .dstAccessMask = dst_access,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange =
            VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    vkCmdPipelineBarrier(
        command_buffer,
        src_stage,
        dst_stage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier);
  }

  Result<void> create_instance() {
#if defined(_WIN32)
    const std::array<const char*, 2> extensions{
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };

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

  Result<void> create_surface(Win32SurfaceHandle surface) {
#if defined(_WIN32)
    const VkWin32SurfaceCreateInfoKHR create_info{
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = static_cast<HINSTANCE>(surface.hinstance),
        .hwnd = static_cast<HWND>(surface.hwnd),
    };

    return require_vk_success(
        vkCreateWin32SurfaceKHR(instance_, &create_info, nullptr, &surface_),
        "vkCreateWin32SurfaceKHR failed");
#else
    (void)surface;
    return std::unexpected(vulkan_error(
        ErrorCode::unsupported_platform,
        "Vulkan renderer Win32 surface initialization requires Windows"));
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
  VkCommandPool command_pool_ = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> command_buffers_;
  VkSemaphore image_available_ = VK_NULL_HANDLE;
  VkSemaphore render_finished_ = VK_NULL_HANDLE;
  VkFence in_flight_ = VK_NULL_HANDLE;
};

} // namespace

Result<void> VulkanFrame::present() {
  return renderer_.present_clear(clear_color_);
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

  if (auto surface = require_win32_surface(descriptor.native_surface);
      !surface) {
    return std::unexpected(surface.error());
  } else {
    auto renderer = VulkanRenderer::create(descriptor, *surface);
    if (!renderer) {
      return std::unexpected(renderer.error());
    }

    return std::unique_ptr<Renderer>(std::move(*renderer));
  }
}

} // namespace cgpui
