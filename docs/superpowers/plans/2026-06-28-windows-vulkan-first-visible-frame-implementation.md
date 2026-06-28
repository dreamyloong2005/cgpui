# Windows Vulkan First Visible Frame Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `examples/hello_window` present a real Vulkan-cleared frame on Windows.

**Architecture:** Keep the public renderer API unchanged and put all Vulkan/Win32 details inside `src/renderer/vulkan/vulkan_renderer.cpp`. Build the renderer in layers: validate Windows surfaces, create Vulkan instance/surface/device, create and recreate the swapchain, then acquire/clear/present a frame from `RenderFrame::present()`.

**Tech Stack:** C++23, Xmake, Win32 native handles, Vulkan 1.x through the existing `vulkansdk` package.

---

## File Structure

- `src/renderer/vulkan/vulkan_renderer.cpp`: replace the no-op skeleton with the Windows Vulkan WSI implementation. This file owns all Vulkan handles, helper structs, teardown order, swapchain recreation, frame acquisition, command recording, clear, submit, and present.
- `tests/renderer/vulkan_surface_validation_test.cpp`: new Windows/Linux-gated renderer validation test. On Windows it verifies that the Vulkan renderer rejects unsupported native surface variants and null Win32 handles before creating Vulkan objects.
- `tests/renderer/vulkan_resize_test.cpp`: update the existing resize validation test to create a hidden Win32 window, so it can construct a valid Windows renderer after native surface validation is added.
- `xmake.lua`: register `vulkan_surface_validation_test` under the Windows/Linux Vulkan target gate.

## Task 1: Add Vulkan Surface Validation Tests

**Files:**
- Create: `tests/renderer/vulkan_surface_validation_test.cpp`
- Modify: `tests/renderer/vulkan_resize_test.cpp`
- Modify: `xmake.lua`

- [ ] **Step 1: Write the failing validation test**

Create `tests/renderer/vulkan_surface_validation_test.cpp`:

```cpp
#include "cgpui/renderer/renderer.hpp"

namespace {

int expect_renderer_initialization_failure(
    const cgpui::RenderSurfaceDescriptor& descriptor) {
  const auto renderer = cgpui::create_renderer(descriptor);
  if (renderer) {
    return 1;
  }

  return renderer.error().code ==
          cgpui::ErrorCode::renderer_initialization_failed
      ? 0
      : 2;
}

} // namespace

int main() {
  const auto framebuffer = cgpui::Size{64.0F, 64.0F};
  const auto scale = cgpui::DpiScale{1.0F};

  const int non_win32_result = expect_renderer_initialization_failure(
      cgpui::RenderSurfaceDescriptor{
          .native_surface = cgpui::WaylandSurfaceHandle{},
          .framebuffer_size = framebuffer,
          .scale = scale});
  if (non_win32_result != 0) {
    return non_win32_result;
  }

  const int null_win32_result = expect_renderer_initialization_failure(
      cgpui::RenderSurfaceDescriptor{
          .native_surface = cgpui::Win32SurfaceHandle{},
          .framebuffer_size = framebuffer,
          .scale = scale});
  if (null_win32_result != 0) {
    return 10 + null_win32_result;
  }

  return 0;
}
```

- [ ] **Step 2: Update the resize test to use a real hidden Win32 surface**

Replace `tests/renderer/vulkan_resize_test.cpp` with:

```cpp
#include "cgpui/renderer/renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {

class HiddenWindow {
 public:
  HiddenWindow() : instance_(GetModuleHandleW(nullptr)) {
    const wchar_t* class_name = L"CGPUIVulkanResizeTestWindow";

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.lpfnWndProc = DefWindowProcW;
    window_class.hInstance = instance_;
    window_class.lpszClassName = class_name;
    RegisterClassExW(&window_class);

    hwnd_ = CreateWindowExW(
        0,
        class_name,
        L"CGPUI Vulkan Resize Test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        64,
        64,
        nullptr,
        nullptr,
        instance_,
        nullptr);
  }

  ~HiddenWindow() {
    if (hwnd_ != nullptr) {
      DestroyWindow(hwnd_);
    }
  }

  [[nodiscard]] bool valid() const { return instance_ != nullptr && hwnd_ != nullptr; }

  [[nodiscard]] cgpui::Win32SurfaceHandle surface() const {
    return cgpui::Win32SurfaceHandle{.hinstance = instance_, .hwnd = hwnd_};
  }

 private:
  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
};

} // namespace

int main() {
  HiddenWindow window;
  if (!window.valid()) {
    return 4;
  }

  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = cgpui::Size{64.0F, 64.0F},
      .scale = cgpui::DpiScale{1.0F}});
  if (!renderer) {
    return 1;
  }

  const auto resized =
      (*renderer)->resize(cgpui::Size{0.0F, 64.0F}, cgpui::DpiScale{1.0F});
  if (resized) {
    return 2;
  }

  return resized.error().code == cgpui::ErrorCode::renderer_initialization_failed
      ? 0
      : 3;
}
```

- [ ] **Step 3: Register the validation test target**

In `xmake.lua`, move the existing `vulkan_resize_test` target into a Windows-only block and add `vulkan_surface_validation_test` beside it:

```lua
if is_plat("windows") then
    target("vulkan_resize_test")
        set_kind("binary")
        add_files("tests/renderer/vulkan_resize_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(public_includedirs)
        add_tests("default")

    target("vulkan_surface_validation_test")
        set_kind("binary")
        add_files("tests/renderer/vulkan_surface_validation_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(public_includedirs)
        add_tests("default")
end
```

- [ ] **Step 4: Run the validation test and verify it fails for the current reason**

Run:

```bash
xmake build vulkan_surface_validation_test
xmake run vulkan_surface_validation_test
```

Expected before implementation: build succeeds and the run fails because `create_renderer()` still accepts the unsupported/null native surfaces.

- [ ] **Step 5: Implement minimal surface validation**

In `src/renderer/vulkan/vulkan_renderer.cpp`, add a helper:

```cpp
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
```

Call it from `create_renderer()` after framebuffer validation and return its error on failure.

- [ ] **Step 6: Run validation and existing tests**

Run:

```bash
xmake run vulkan_surface_validation_test
xmake run vulkan_resize_test
xmake test
```

Expected: `vulkan_surface_validation_test`, `vulkan_resize_test`, and the full test suite pass.

- [ ] **Step 7: Commit**

```bash
git add xmake.lua tests/renderer/vulkan_surface_validation_test.cpp tests/renderer/vulkan_resize_test.cpp src/renderer/vulkan/vulkan_renderer.cpp
git commit -m "test: validate vulkan native surfaces"
```

## Task 2: Create Vulkan Instance, Win32 Surface, And Device

**Files:**
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`

- [ ] **Step 1: Add Vulkan ownership fields and teardown helpers**

Replace the current trivial `VulkanRenderer` storage with fields:

```cpp
RenderSurfaceDescriptor descriptor_;
VkInstance instance_ = VK_NULL_HANDLE;
VkSurfaceKHR surface_ = VK_NULL_HANDLE;
VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
VkDevice device_ = VK_NULL_HANDLE;
std::uint32_t graphics_queue_family_ = 0;
std::uint32_t present_queue_family_ = 0;
VkQueue graphics_queue_ = VK_NULL_HANDLE;
VkQueue present_queue_ = VK_NULL_HANDLE;
```

Add a destructor that destroys device, surface, and instance in reverse order:

```cpp
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
```

- [ ] **Step 2: Add error helpers**

Add helpers:

```cpp
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
```

Include `<string>`.

- [ ] **Step 3: Create the Vulkan instance**

Add a `create_instance()` method:

```cpp
Result<void> create_instance() {
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
      .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
  };

  return require_vk_success(
      vkCreateInstance(&create_info, nullptr, &instance_),
      "vkCreateInstance failed");
}
```

Include `<array>`.

- [ ] **Step 4: Create the Win32 surface**

Add a `create_surface(Win32SurfaceHandle surface)` method:

```cpp
Result<void> create_surface(Win32SurfaceHandle surface) {
  const VkWin32SurfaceCreateInfoKHR create_info{
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hinstance = static_cast<HINSTANCE>(surface.hinstance),
      .hwnd = static_cast<HWND>(surface.hwnd),
  };

  return require_vk_success(
      vkCreateWin32SurfaceKHR(instance_, &create_info, nullptr, &surface_),
      "vkCreateWin32SurfaceKHR failed");
}
```

Define `WIN32_LEAN_AND_MEAN` before including `<windows.h>`, and include `<vulkan/vulkan_win32.h>` instead of only `<vulkan/vulkan.h>` on Windows.

- [ ] **Step 5: Select queue families and physical device**

Add a helper struct:

```cpp
struct QueueFamilies {
  std::uint32_t graphics = 0;
  std::uint32_t present = 0;
};
```

Add `find_queue_families(VkPhysicalDevice device)` that:

- calls `vkGetPhysicalDeviceQueueFamilyProperties`.
- picks the first queue family with `VK_QUEUE_GRAPHICS_BIT`.
- calls `vkGetPhysicalDeviceSurfaceSupportKHR` to pick a present-capable family.
- returns an error if either family is missing.

Add `select_physical_device()` that:

- calls `vkEnumeratePhysicalDevices`.
- scans devices for queue families.
- stores `physical_device_`, `graphics_queue_family_`, and `present_queue_family_`.
- verifies `VK_KHR_SWAPCHAIN_EXTENSION_NAME` is available on the selected device.

- [ ] **Step 6: Create logical device and queues**

Add `create_device()` that:

- creates one `VkDeviceQueueCreateInfo` when graphics and present families are equal.
- creates two queue infos when they differ.
- enables `VK_KHR_SWAPCHAIN_EXTENSION_NAME`.
- calls `vkCreateDevice`.
- calls `vkGetDeviceQueue` for both queue handles.

- [ ] **Step 7: Wire initialization through a factory method**

Make `VulkanRenderer` construction private to a static factory:

```cpp
static Result<std::unique_ptr<VulkanRenderer>> create(
    RenderSurfaceDescriptor descriptor,
    Win32SurfaceHandle native_surface) {
  auto renderer = std::unique_ptr<VulkanRenderer>(
      new VulkanRenderer(std::move(descriptor)));
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
```

Call this from `create_renderer()`.

- [ ] **Step 8: Build**

Run:

```bash
xmake build cgpui_renderer_vulkan
xmake build hello_window
xmake test
```

Expected: build succeeds and all registered tests pass.

- [ ] **Step 9: Commit**

```bash
git add src/renderer/vulkan/vulkan_renderer.cpp tests/renderer/vulkan_resize_test.cpp
git commit -m "feat: create windows vulkan device"
```

## Task 3: Add Swapchain, Command Recording, Clear, And Present

**Files:**
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`

- [ ] **Step 1: Add swapchain fields**

Add fields:

```cpp
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
```

Include `<vector>`, `<algorithm>`, and `<limits>`.

- [ ] **Step 2: Add swapchain cleanup**

Add:

```cpp
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
```

Destroy synchronization and command pool in the destructor before destroying the device:

```cpp
if (device_ != VK_NULL_HANDLE) {
  vkDeviceWaitIdle(device_);
  destroy_swapchain();
  if (in_flight_ != VK_NULL_HANDLE) vkDestroyFence(device_, in_flight_, nullptr);
  if (render_finished_ != VK_NULL_HANDLE) vkDestroySemaphore(device_, render_finished_, nullptr);
  if (image_available_ != VK_NULL_HANDLE) vkDestroySemaphore(device_, image_available_, nullptr);
  if (command_pool_ != VK_NULL_HANDLE) vkDestroyCommandPool(device_, command_pool_, nullptr);
  vkDestroyDevice(device_, nullptr);
}
```

- [ ] **Step 3: Query swapchain support and choose settings**

Add helpers:

- `VkSurfaceFormatKHR choose_surface_format(std::span<const VkSurfaceFormatKHR>)`: prefer `VK_FORMAT_B8G8R8A8_SRGB` with `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR`, otherwise first format.
- `VkPresentModeKHR choose_present_mode(std::span<const VkPresentModeKHR>)`: prefer `VK_PRESENT_MODE_MAILBOX_KHR`, otherwise `VK_PRESENT_MODE_FIFO_KHR`.
- `VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR&)`: use `currentExtent` unless it is `std::numeric_limits<std::uint32_t>::max()`, otherwise clamp descriptor framebuffer size.

Include `<span>`.

- [ ] **Step 4: Create swapchain and image views**

Add `create_swapchain()` that:

- calls `vkGetPhysicalDeviceSurfaceCapabilitiesKHR`.
- calls `vkGetPhysicalDeviceSurfaceFormatsKHR`.
- calls `vkGetPhysicalDeviceSurfacePresentModesKHR`.
- chooses image count as `minImageCount + 1`, capped by `maxImageCount` if non-zero.
- uses `VK_IMAGE_USAGE_TRANSFER_DST_BIT` because this pass clears with `vkCmdClearColorImage`.
- uses concurrent sharing if graphics and present queues differ, otherwise exclusive.
- calls `vkCreateSwapchainKHR`.
- retrieves swapchain images.
- creates one `VkImageView` per image.
- allocates one primary command buffer per swapchain image from `command_pool_`.

- [ ] **Step 5: Create command pool and synchronization**

Add:

```cpp
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
          vkCreateSemaphore(device_, &semaphore_info, nullptr, &image_available_),
          "vkCreateSemaphore failed"); !result) return result;
  if (auto result = require_vk_success(
          vkCreateSemaphore(device_, &semaphore_info, nullptr, &render_finished_),
          "vkCreateSemaphore failed"); !result) return result;
  return require_vk_success(
      vkCreateFence(device_, &fence_info, nullptr, &in_flight_),
      "vkCreateFence failed");
}
```

Call `create_command_pool()`, `create_swapchain()`, and `create_sync_objects()` in the factory after device creation.

- [ ] **Step 6: Add a frame object that can present through the renderer**

Change `VulkanFrame` to store a reference:

```cpp
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
```

Make `begin_frame()` return `std::make_unique<VulkanFrame>(*this)`.

- [ ] **Step 7: Implement image layout transitions and clear**

Add helpers in `VulkanRenderer`:

```cpp
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
      .subresourceRange = VkImageSubresourceRange{
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
```

- [ ] **Step 8: Implement `present_clear(Color)`**

Add:

```cpp
Result<void> present_clear(Color color) {
  vkWaitForFences(device_, 1, &in_flight_, VK_TRUE, UINT64_MAX);
  vkResetFences(device_, 1, &in_flight_);

  std::uint32_t image_index = 0;
  const VkResult acquire_result = vkAcquireNextImageKHR(
      device_, swapchain_, UINT64_MAX, image_available_, VK_NULL_HANDLE, &image_index);
  if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "vkAcquireNextImageKHR reported an out-of-date swapchain"));
  }
  if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR) {
    return std::unexpected(vulkan_error(
        ErrorCode::frame_acquisition_failed,
        "vkAcquireNextImageKHR failed"));
  }

  VkCommandBuffer command_buffer = command_buffers_[image_index];
  vkResetCommandBuffer(command_buffer, 0);
  const VkCommandBufferBeginInfo begin_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  if (auto result = require_vk_success(
          vkBeginCommandBuffer(command_buffer, &begin_info),
          "vkBeginCommandBuffer failed"); !result) return result;

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
          vkEndCommandBuffer(command_buffer), "vkEndCommandBuffer failed"); !result) {
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
          "vkQueueSubmit failed"); !result) {
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
  const VkResult present_result = vkQueuePresentKHR(present_queue_, &present_info);
  if (present_result == VK_ERROR_OUT_OF_DATE_KHR ||
      present_result == VK_SUBOPTIMAL_KHR) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "vkQueuePresentKHR reported an out-of-date swapchain"));
  }
  return require_vk_success(present_result, "vkQueuePresentKHR failed");
}
```

Implement `VulkanFrame::present()` as `return renderer_.present_clear(clear_color_);`.

- [ ] **Step 9: Recreate swapchain on resize**

Update `resize()`:

```cpp
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
```

- [ ] **Step 10: Build and run automated checks**

Run:

```bash
xmake build cgpui_renderer_vulkan
xmake build hello_window
xmake test
```

Expected: all commands pass.

- [ ] **Step 11: Manual visible-frame verification**

Run:

```bash
xmake run hello_window
```

Expected: a Win32 window opens and shows a visible cleared color. Close the window. Confirm no `hello_window.exe` process remains:

```powershell
Get-Process hello_window -ErrorAction SilentlyContinue
```

Expected: no output.

- [ ] **Step 12: Commit**

```bash
git add src/renderer/vulkan/vulkan_renderer.cpp
git commit -m "feat: present first vulkan frame"
```

## Task 4: Final Verification And README Note

**Files:**
- Modify: `README.md`

- [ ] **Step 1: Update README current foundation wording**

Change the Windows wording in `README.md` so it says Windows now presents a Vulkan-cleared frame, while rectangle drawing remains deferred:

```markdown
The current foundation defines architecture boundaries and compiles backend targets with Xmake. Windows now presents a Vulkan-cleared frame through the Win32 demo path. Linux and macOS still use renderer skeletons in this milestone, and Linux Wayland surface completion is planned for a later pass.

Solid rectangle GPU drawing is planned separately; this milestone keeps draw commands behind backend interfaces while proving the Windows Vulkan present path.
```

- [ ] **Step 2: Run final verification**

Run:

```bash
xmake f -c -m debug
xmake build core_header_cleanliness
xmake build ui_header_cleanliness
xmake build hello_window_lifetime_test
xmake build vulkan_resize_test
xmake build vulkan_surface_validation_test
xmake run core_header_cleanliness
xmake run ui_header_cleanliness
xmake run render_view_test
xmake run hello_window_lifetime_test
xmake run vulkan_resize_test
xmake run vulkan_surface_validation_test
xmake test
xmake show -t hello_window
xmake build hello_window
git diff --check
```

Expected:

- all builds pass.
- all tests pass.
- `hello_window` target still depends on `cgpui_platform_win32` and `cgpui_renderer_vulkan` on Windows.
- whitespace check is clean.

- [ ] **Step 3: Manual visible-frame verification**

Run:

```bash
xmake run hello_window
```

Expected:

- a native Win32 window opens.
- the content area displays the Vulkan clear color.
- closing the window exits the process.

Then run:

```powershell
Get-Process hello_window -ErrorAction SilentlyContinue
```

Expected: no output.

- [ ] **Step 4: Commit**

```bash
git add README.md
git commit -m "docs: note windows vulkan first frame"
```

## Plan Self-Review

- Spec coverage: Tasks cover native surface validation, Windows Vulkan instance/surface/device, swapchain, command buffers, synchronization, clear, present, resize recreation, README update, automated checks, and manual visible-frame verification.
- Placeholder scan: no placeholder markers remain.
- Type consistency: all tasks use existing `RenderSurfaceDescriptor`, `Win32SurfaceHandle`, `Renderer`, `RenderFrame`, `Color`, `Size`, and `DpiScale` names.
- Scope check: rectangle drawing, Wayland WSI, validation layers, and advanced swapchain policies remain deferred as specified.
