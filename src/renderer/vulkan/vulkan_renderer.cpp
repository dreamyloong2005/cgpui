#include "cgpui/renderer/renderer.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_win32.h>
#else
#include <vulkan/vulkan.h>
#endif

#include <array>
#include <cstdint>
#include <cstring>
#include <expected>
#include <memory>
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

class VulkanFrame final : public RenderFrame {
 public:
  void clear(Color color) override { clear_color_ = color; }
  void draw_rect(const SolidRect& rect) override { rect_ = rect; }
  Result<void> present() override { return {}; }

 private:
  Color clear_color_{};
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
    return {};
  }

  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    return std::make_unique<VulkanFrame>();
  }

 private:
  explicit VulkanRenderer(RenderSurfaceDescriptor descriptor)
      : descriptor_(std::move(descriptor)) {}

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
};

} // namespace

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
