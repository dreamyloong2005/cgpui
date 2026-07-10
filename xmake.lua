set_project("cgpui")
set_version("0.1.0")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

add_requires("lunasvg")

local public_includedirs = "include"

if is_plat("windows") then
    add_requires("vulkansdk")
end

if is_plat("linux") then
    add_requires("wayland")
    add_requires("libxkbcommon", {configs = {x11 = false, wayland = false, tools = false}})
    add_requires("fontconfig", {system = true, optional = true})
end

target("cgpui_core")
    set_kind("static")
    add_files("src/core/*.cpp")
    add_includedirs(public_includedirs, {public = true})

target("cgpui_platform")
    set_kind("static")
    add_files("src/platform/*.cpp")
    add_files("src/ui/text_font.cpp")
    remove_files("src/platform/platform_unimplemented.cpp")
    add_deps("cgpui_core")
    add_includedirs(public_includedirs, {public = true})
    if is_plat("windows") then
        add_syslinks("user32")
    elseif is_plat("linux") then
        add_packages("wayland")
    end

target("cgpui_platform_fallback")
    set_kind("static")
    add_files("src/platform/platform_unimplemented.cpp")
    add_deps("cgpui_core", "cgpui_platform")
    add_includedirs(public_includedirs, {public = true})

if is_plat("windows") then
    target("cgpui_platform_win32")
        set_kind("static")
        add_files("src/platform/win32/*.cpp")
        add_deps("cgpui_core", "cgpui_platform")
        add_includedirs(public_includedirs, {public = true})
        add_syslinks("user32", "gdi32", "shell32", "imm32", "ole32", "dwrite")

    target("win32_input_event_test")
        set_kind("binary")
        add_files("tests/platform/win32_input_event_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(public_includedirs)
        add_syslinks("user32")
        add_tests("default")

    target("win32_dpi_scale_test")
        set_kind("binary")
        add_files("tests/platform/win32_dpi_scale_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(public_includedirs)
        add_syslinks("user32")
        add_tests("default")

    target("win32_focus_event_test")
        set_kind("binary")
        add_files("tests/platform/win32_focus_event_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(public_includedirs)
        add_syslinks("user32")
        add_tests("default")

    target("win32_text_input_test")
        set_kind("binary")
        add_files("tests/platform/win32_text_input_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(public_includedirs)
        add_syslinks("user32", "imm32")
        add_tests("default")

    target("win32_font_discovery_test")
        set_kind("binary")
        add_files("tests/platform/win32_font_discovery_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(public_includedirs)
        add_syslinks("user32", "dwrite")
        add_tests("default")
end

if is_plat("linux") then
    target("cgpui_platform_linux_wayland")
        set_kind("static")
        add_files("src/platform/linux/*.cpp")
        add_deps("cgpui_core", "cgpui_platform")
        add_packages("wayland", "libxkbcommon")
        if has_package("fontconfig") then
            add_packages("fontconfig")
            add_defines("CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND")
        end
        add_includedirs(public_includedirs, {public = true})

    target("wayland_compositor_close_test")
        set_kind("binary")
        add_files("tests/platform/wayland_compositor_close_test.cpp")
        add_files("tests/platform/wayland_test_compositor.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland")
        add_packages("wayland")
        add_syslinks("wayland-server")
        add_includedirs(public_includedirs)
        add_includedirs("tests/platform")
        add_tests("default")

    target("wayland_compositor_resize_test")
        set_kind("binary")
        add_files("tests/platform/wayland_compositor_resize_test.cpp")
        add_files("tests/platform/wayland_test_compositor.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland")
        add_packages("wayland")
        add_syslinks("wayland-server")
        add_includedirs(public_includedirs)
        add_includedirs("tests/platform")
        add_tests("default")

    target("wayland_pointer_button_test")
        set_kind("binary")
        add_files("tests/platform/wayland_pointer_button_test.cpp")
        add_files("tests/platform/wayland_test_compositor.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland")
        add_packages("wayland")
        add_syslinks("wayland-server")
        add_includedirs(public_includedirs)
        add_includedirs("tests/platform")
        add_tests("default")

    target("wayland_pointer_scroll_test")
        set_kind("binary")
        add_files("tests/platform/wayland_pointer_scroll_test.cpp")
        add_files("tests/platform/wayland_test_compositor.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland")
        add_packages("wayland")
        add_syslinks("wayland-server")
        add_includedirs(public_includedirs)
        add_includedirs("tests/platform")
        add_tests("default")

    target("wayland_keyboard_test")
        set_kind("binary")
        add_files("tests/platform/wayland_keyboard_test.cpp")
        add_files("tests/platform/wayland_test_compositor.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland")
        add_packages("wayland")
        add_syslinks("wayland-server")
        add_includedirs(public_includedirs)
        add_includedirs("tests/platform")
        add_tests("default")

    target("wayland_font_discovery_test")
        set_kind("binary")
        add_files("tests/platform/wayland_font_discovery_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland")
        add_includedirs(public_includedirs)
        add_tests("default")

    target("wayland_vulkan_surface_test")
        set_kind("binary")
        add_files("tests/renderer/wayland_vulkan_surface_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("wayland")
        add_syslinks("vulkan")
        add_includedirs(public_includedirs)
        add_tests("default")
end

if is_plat("macosx") then
    target("cgpui_platform_macos")
        set_kind("static")
        add_files("src/platform/macos/*.mm")
        add_deps("cgpui_core", "cgpui_platform")
        add_frameworks("AppKit", "QuartzCore")
        add_includedirs(public_includedirs, {public = true})
end

target("cgpui_renderer")
    set_kind("static")
    add_files("src/renderer/*.cpp")
    add_files("src/ui/text_font.cpp")
    add_files("src/ui/text_shaping_backend.cpp")
    add_files("src/ui/text_shaping_dispatch.cpp")
    add_files("src/ui/text_shaping_harfbuzz.cpp")
    add_files("src/ui/text_shaping_fallback.cpp")
    add_files("src/ui/text_shape.cpp")
    add_files("src/ui/text_glyph_raster.cpp")
    add_files("src/ui/text_measurement.cpp")
    add_files("src/ui/text_line_metrics.cpp")
    add_files("src/ui/text_paragraph_layout.cpp")
    add_files("src/ui/text_measurement_grapheme.cpp")
    add_files("src/ui/text_measurement_bidi.cpp")
    add_files("src/ui/text_wrapping.cpp")
    add_files("src/ui/text_hit_testing.cpp")
    remove_files("src/renderer/renderer_unimplemented.cpp")
    add_deps("cgpui_core")
    add_packages("lunasvg", {public = true})
    add_includedirs(public_includedirs, {public = true})

target("cgpui_renderer_fallback")
    set_kind("static")
    add_files("src/renderer/renderer_unimplemented.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
    add_includedirs(public_includedirs, {public = true})

if is_plat("windows", "linux") then
    target("cgpui_renderer_vulkan")
        set_kind("static")
        add_files("src/renderer/vulkan/*.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_includedirs(public_includedirs, {public = true})
end

if is_plat("macosx") then
    target("cgpui_renderer_metal")
        set_kind("static")
        add_files("src/renderer/metal/*.mm")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
        add_frameworks("Metal", "QuartzCore")
        add_includedirs(public_includedirs, {public = true})
end

target("core_header_cleanliness")
    set_kind("binary")
    add_files("tests/header_cleanliness/core_header_cleanliness.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
    add_includedirs(public_includedirs)
    add_tests("default")

target("entity_store_test")
    set_kind("binary")
    add_files("tests/core/entity_store_test.cpp")
    add_deps("cgpui_core")
    add_includedirs(public_includedirs)
    add_tests("default")

target("clipboard_test")
    set_kind("binary")
    add_files("tests/platform/clipboard_test.cpp")
    if is_plat("linux") then
        add_files("tests/platform/wayland_test_compositor.cpp")
        add_packages("wayland")
        add_syslinks("wayland-server")
        add_includedirs("tests/platform")
    end
    add_deps("cgpui_core", "cgpui_platform")
    add_includedirs(public_includedirs)
    if is_plat("windows") then
        add_syslinks("user32")
    end
    add_tests("default")

target("cgpui_ui")
    set_kind("static")
    add_files("src/ui/*.cpp")
    add_files("src/ui/widgets/*.cpp")
    remove_files("src/ui/text_shaping_backend.cpp")
    remove_files("src/ui/text_shaping_dispatch.cpp")
    remove_files("src/ui/text_shaping_harfbuzz.cpp")
    remove_files("src/ui/text_shaping_fallback.cpp")
    remove_files("src/ui/text_shape.cpp")
    remove_files("src/ui/text_font.cpp")
    remove_files("src/ui/text_glyph_raster.cpp")
    remove_files("src/ui/text_measurement.cpp")
    remove_files("src/ui/text_line_metrics.cpp")
    remove_files("src/ui/text_paragraph_layout.cpp")
    remove_files("src/ui/text_measurement_grapheme.cpp")
    remove_files("src/ui/text_measurement_bidi.cpp")
    remove_files("src/ui/text_wrapping.cpp")
    remove_files("src/ui/text_hit_testing.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
    add_includedirs(public_includedirs, {public = true})

target("cgpui_app")
    set_kind("static")
    add_files("src/app/*.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos")
    else
        add_deps("cgpui_platform_fallback")
    end
    add_includedirs(public_includedirs, {public = true})

target("ui_header_cleanliness")
    set_kind("binary")
    add_files("tests/header_cleanliness/ui_header_cleanliness.cpp")
    add_deps("cgpui_core", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("prelude_header_cleanliness")
    set_kind("binary")
    add_files("tests/header_cleanliness/prelude_header_cleanliness.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("app_header_cleanliness")
    set_kind("binary")
    add_files("tests/header_cleanliness/app_header_cleanliness.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("gpui_parity_ledger_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/gpui_parity_ledger_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("application_facade_test")
    set_kind("binary")
    add_files("tests/api_parity/application_facade_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("app_window_context_test")
    set_kind("binary")
    add_files("tests/api_parity/app_window_context_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("public_result_conventions_test")
    set_kind("binary")
    add_files("tests/api_parity/public_result_conventions_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("renderer_result_conventions_test")
    set_kind("binary")
    add_files("tests/api_parity/renderer_result_conventions_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("renderer_resize_result_conventions_test")
    set_kind("binary")
    add_files("tests/api_parity/renderer_resize_result_conventions_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("renderer_frame_result_conventions_test")
    set_kind("binary")
    add_files("tests/api_parity/renderer_frame_result_conventions_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("platform_service_result_conventions_test")
    set_kind("binary")
    add_files("tests/api_parity/platform_service_result_conventions_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("platform_font_discovery_test")
    set_kind("binary")
    add_files("tests/platform/platform_font_discovery_test.cpp")
    add_deps("cgpui_core", "cgpui_platform")
    add_includedirs(public_includedirs)
    add_tests("default")

target("async_spawn_result_conventions_test")
    set_kind("binary")
    add_files("tests/api_parity/async_spawn_result_conventions_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("context_render_spelling_test")
    set_kind("binary")
    add_files("tests/api_parity/context_render_spelling_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("context_capabilities_test")
    set_kind("binary")
    add_files("tests/api_parity/context_capabilities_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("app_context_capability_test")
    set_kind("binary")
    add_files("tests/api_parity/app_context_capability_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("view_context_capability_test")
    set_kind("binary")
    add_files("tests/api_parity/view_context_capability_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_context_capability_test")
    set_kind("binary")
    add_files("tests/api_parity/window_context_capability_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("element_context_capability_test")
    set_kind("binary")
    add_files("tests/api_parity/element_context_capability_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("async_context_capability_test")
    set_kind("binary")
    add_files("tests/api_parity/async_context_capability_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("test_context_capability_test")
    set_kind("binary")
    add_files("tests/api_parity/test_context_capability_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("view_handle_spelling_test")
    set_kind("binary")
    add_files("tests/api_parity/view_handle_spelling_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("public_authoring_surface_test")
    set_kind("binary")
    add_files("tests/api_parity/public_authoring_surface_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("public_api_compatibility_examples_test")
    set_kind("binary")
    add_files("tests/api_parity/public_api_compatibility_examples_test.cpp")
    add_tests("default")

target("public_api_example_expansion_test")
    set_kind("binary")
    add_files("tests/api_parity/public_api_example_expansion_test.cpp")
    add_tests("default")

target("public_context_capability_example_test")
    set_kind("binary")
    add_files("tests/api_parity/public_context_capability_example_test.cpp")
    add_tests("default")

target("public_async_test_workflow_example_test")
    set_kind("binary")
    add_files("tests/api_parity/public_async_test_workflow_example_test.cpp")
    add_tests("default")

target("public_phase_b_surface_closure_example_test")
    set_kind("binary")
    add_files("tests/api_parity/public_phase_b_surface_closure_example_test.cpp")
    add_tests("default")

target("public_authoring_vocabulary_freeze_test")
    set_kind("binary")
    add_files("tests/api_parity/public_authoring_vocabulary_freeze_test.cpp")
    add_tests("default")

target("public_phase_b_completion_audit_test")
    set_kind("binary")
    add_files("tests/api_parity/public_phase_b_completion_audit_test.cpp")
    add_tests("default")

target("phase_c_style_cascade_depth_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_focusable_interactable_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_focusable_interactable_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_uniform_list_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_uniform_list_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_window_examples_public_api_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_window_examples_public_api_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_window_examples_workflow_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_window_examples_workflow_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_window_examples_widget_catalog_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_window_examples_widget_catalog_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_window_examples_interaction_states_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_window_examples_interaction_states_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_window_examples_service_matrix_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_window_examples_service_matrix_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_window_examples_closeout_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_window_examples_closeout_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_svg_image_front_end_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_svg_image_front_end_test.cpp")
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_svg_image_asset_registration_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_svg_image_asset_registration_test.cpp")
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_svg_image_public_examples_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_svg_image_public_examples_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_svg_image_closeout_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_svg_image_closeout_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_widget_family_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_widget_family_structure_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_c_final_ledger_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_c_final_ledger_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("pre_phase_d_entry_gate_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/pre_phase_d_entry_gate_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_text_shaping_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_text_shaping_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_font_fallback_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_font_fallback_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_fallback_splitting_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_fallback_splitting_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_text_measurement_wrapping_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_text_measurement_wrapping_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_selection_caret_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_selection_caret_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_edit_history_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_edit_history_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_ime_platform_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_ime_platform_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_rich_text_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_rich_text_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_text_input_public_examples_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_text_input_public_examples_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_text_wrapper_public_examples_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_text_wrapper_public_examples_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_text_input_workflow_examples_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_text_input_workflow_examples_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_rich_text_public_examples_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_rich_text_public_examples_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_harfbuzz_backend_audit_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_harfbuzz_backend_audit_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_text_examples_closeout_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_text_examples_closeout_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_d_final_closeout_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_d_final_closeout_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_e_glyph_atlas_production_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_e_glyph_atlas_production_test.cpp")
    add_deps("cgpui_core", "cgpui_renderer", "cgpui_renderer_vulkan")
    add_includedirs(public_includedirs)
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_e_glyph_atlas_integration_closeout_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_e_glyph_atlas_integration_closeout_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_e_text_pipeline_integration_closeout_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_e_text_pipeline_integration_closeout_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_e_clip_composition_integration_closeout_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_e_clip_composition_integration_closeout_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_e_image_integration_closeout_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_e_image_integration_closeout_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("svg_rasterization_plan_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/renderer/svg_rasterization_plan_test.cpp")
    add_deps("cgpui_core", "cgpui_renderer")
    add_includedirs(public_includedirs)
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("svg_lunasvg_rasterization_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/renderer/svg_lunasvg_rasterization_test.cpp")
    add_deps("cgpui_core", "cgpui_renderer")
    add_includedirs(public_includedirs)
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("svg_raster_cache_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/renderer/svg_raster_cache_test.cpp")
    add_deps("cgpui_core", "cgpui_renderer")
    add_includedirs(public_includedirs)
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("svg_viewport_scaling_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/renderer/svg_viewport_scaling_test.cpp")
    add_deps("cgpui_core", "cgpui_renderer")
    add_includedirs(public_includedirs)
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("svg_recolor_tint_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/renderer/svg_recolor_tint_test.cpp")
    add_deps("cgpui_core", "cgpui_renderer")
    add_includedirs(public_includedirs)
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("svg_image_upload_integration_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/renderer/svg_image_upload_integration_test.cpp")
    add_deps("cgpui_core", "cgpui_renderer")
    add_includedirs(public_includedirs)
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_e_svg_public_example_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_e_svg_public_example_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_e_svg_integration_closeout_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_e_svg_integration_closeout_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_e_batching_scheduling_integration_closeout_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/api_parity/phase_e_batching_scheduling_integration_closeout_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("renderer_frame_diagnostics_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/renderer/renderer_frame_diagnostics_test.cpp")
    add_deps("cgpui_core", "cgpui_renderer")
    add_includedirs(public_includedirs)
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

if is_plat("windows", "linux") then
    target("vulkan_frame_geometry_buffer_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_frame_geometry_buffer_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_frame_command_reuse_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_frame_command_reuse_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_pipeline_switch_batching_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_pipeline_switch_batching_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_upload_barrier_batching_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_upload_barrier_batching_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_swapchain_recovery_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_swapchain_recovery_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_present_pacing_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_present_pacing_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_glyph_atlas_descriptor_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_glyph_atlas_descriptor_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_glyph_atlas_upload_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_glyph_atlas_upload_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_glyph_atlas_multi_page_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_glyph_atlas_multi_page_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_glyph_atlas_draw_binding_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_glyph_atlas_draw_binding_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_glyph_atlas_draw_data_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_glyph_atlas_draw_data_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_glyph_atlas_frame_lifecycle_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_glyph_atlas_frame_lifecycle_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs)
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_text_pipeline_state_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_text_pipeline_state_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_text_shader_module_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_text_shader_module_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_text_pipeline_resources_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_text_pipeline_resources_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_text_vertex_buffer_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_text_vertex_buffer_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_text_draw_recording_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_text_draw_recording_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_text_positioning_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_text_positioning_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_text_coverage_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_text_coverage_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_rounded_rect_geometry_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_rounded_rect_geometry_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_rounded_rect_buffer_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_rounded_rect_buffer_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_rounded_rect_pipeline_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_rounded_rect_pipeline_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_rounded_rect_draw_recording_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_rounded_rect_draw_recording_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_rounded_rect_antialiasing_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_rounded_rect_antialiasing_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_rounded_rect_radii_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_rounded_rect_radii_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_rounded_rect_stroke_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_rounded_rect_stroke_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_rounded_rect_fill_variants_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_rounded_rect_fill_variants_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan", "cgpui_ui")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_clip_stack_recording_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_clip_stack_recording_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_nested_opacity_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_nested_opacity_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_solid_rect_geometry_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_solid_rect_geometry_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_composed_transform_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_composed_transform_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_transform_clip_interaction_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_transform_clip_interaction_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan", "cgpui_ui")
        add_includedirs(public_includedirs, "src/ui", "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_stable_draw_order_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_stable_draw_order_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_layer_ordering_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_layer_ordering_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan", "cgpui_ui")
        add_includedirs(public_includedirs, "src/ui", "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_image_texture_resources_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_image_texture_resources_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_image_texture_upload_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_image_texture_upload_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_image_texture_descriptor_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_image_texture_descriptor_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_image_draw_recording_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_image_draw_recording_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan", "cgpui_ui")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_image_tint_opacity_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_image_tint_opacity_test.cpp")
        add_deps("cgpui_core", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_image_texture_cache_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_image_texture_cache_test.cpp")
        add_deps("cgpui_core", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

    target("vulkan_image_texture_invalidation_test")
        set_kind("binary")
        set_rundir(os.projectdir())
        add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
        add_files("tests/renderer/vulkan_image_texture_invalidation_test.cpp")
        add_deps("cgpui_core", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(public_includedirs, "src/renderer/vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        elseif is_plat("linux") then
            add_syslinks("vulkan")
        end
        add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})
end

target("entity_lifecycle_creation_test")
    set_kind("binary")
    add_files("tests/api_parity/entity_lifecycle_creation_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("entity_weak_handle_semantics_test")
    set_kind("binary")
    add_files("tests/api_parity/entity_weak_handle_semantics_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("entity_observation_test")
    set_kind("binary")
    add_files("tests/api_parity/entity_observation_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("subscription_lifetime_test")
    set_kind("binary")
    add_files("tests/api_parity/subscription_lifetime_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("entity_to_entity_observation_test")
    set_kind("binary")
    add_files("tests/api_parity/entity_to_entity_observation_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_view_observation_test")
    set_kind("binary")
    add_files("tests/api_parity/window_view_observation_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("entity_transaction_test")
    set_kind("binary")
    add_files("tests/api_parity/entity_transaction_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("entity_invalidation_test")
    set_kind("binary")
    add_files("tests/api_parity/entity_invalidation_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("entity_deletion_test")
    set_kind("binary")
    add_files("tests/api_parity/entity_deletion_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("typed_action_surface_test")
    set_kind("binary")
    add_files("tests/api_parity/typed_action_surface_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("typed_action_dispatch_test")
    set_kind("binary")
    add_files("tests/ui/typed_action_dispatch_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("action_scope_metadata_test")
    set_kind("binary")
    add_files("tests/ui/action_scope_metadata_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("action_enablement_metadata_test")
    set_kind("binary")
    add_files("tests/ui/action_enablement_metadata_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("action_bubbling_test")
    set_kind("binary")
    add_files("tests/ui/action_bubbling_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("key_binding_grammar_test")
    set_kind("binary")
    add_files("tests/ui/key_binding_grammar_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("key_binding_platform_modifier_test")
    set_kind("binary")
    add_files("tests/ui/key_binding_platform_modifier_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("keymap_context_test")
    set_kind("binary")
    add_files("tests/ui/keymap_context_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("key_binding_partial_match_test")
    set_kind("binary")
    add_files("tests/ui/key_binding_partial_match_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("key_binding_disabled_scope_test")
    set_kind("binary")
    add_files("tests/ui/key_binding_disabled_scope_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("command_palette_key_integration_test")
    set_kind("binary")
    add_files("tests/ui/command_palette_key_integration_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("test_context_keystroke_simulation_test")
    set_kind("binary")
    add_files("tests/ui/test_context_keystroke_simulation_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("test_context_pointer_simulation_test")
    set_kind("binary")
    add_files("tests/ui/test_context_pointer_simulation_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("test_context_focus_activation_test")
    set_kind("binary")
    add_files("tests/ui/test_context_focus_activation_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("test_context_clipboard_test")
    set_kind("binary")
    add_files("tests/ui/test_context_clipboard_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("test_context_time_async_test")
    set_kind("binary")
    add_files("tests/ui/test_context_time_async_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("test_context_frame_pump_test")
    set_kind("binary")
    add_files("tests/ui/test_context_frame_pump_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("typed_action_command_metadata_test")
    set_kind("binary")
    add_files("tests/ui/typed_action_command_metadata_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(public_includedirs)
    add_tests("default")

target("app_source_structure_test")
    set_kind("binary")
    add_files("tests/architecture/app_source_structure_test.cpp")
    add_tests("default")

target("api_parity_hello_world")
    set_kind("binary")
    add_files("examples/api_parity/hello_world/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_api_compatibility")
    set_kind("binary")
    add_files("examples/api_parity/public_api_compatibility/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_authoring_workflow")
    set_kind("binary")
    add_files("examples/api_parity/public_authoring_workflow/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_svg_image_sources")
    set_kind("binary")
    add_files("examples/api_parity/public_svg_image_sources/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_svg_raster_upload")
    set_kind("binary")
    add_files("examples/api_parity/public_svg_raster_upload/main.cpp")
    add_deps("cgpui_core", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)

target("api_parity_public_context_capabilities")
    set_kind("binary")
    add_files("examples/api_parity/public_context_capabilities/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_async_test_workflow")
    set_kind("binary")
    add_files("examples/api_parity/public_async_test_workflow/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_phase_b_surface_closure")
    set_kind("binary")
    add_files("examples/api_parity/public_phase_b_surface_closure/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_window_examples")
    set_kind("binary")
    add_files("examples/api_parity/public_window_examples/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_window_examples_workflow")
    set_kind("binary")
    add_files("examples/api_parity/public_window_examples_workflow/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_text_input_examples")
    set_kind("binary")
    add_files("examples/api_parity/public_text_input_examples/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_text_wrapper_examples")
    set_kind("binary")
    add_files("examples/api_parity/public_text_wrapper_examples/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_text_input_workflow")
    set_kind("binary")
    add_files("examples/api_parity/public_text_input_workflow/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_rich_text_examples")
    set_kind("binary")
    add_files("examples/api_parity/public_rich_text_examples/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_window_examples_widget_catalog")
    set_kind("binary")
    add_files("examples/api_parity/public_window_examples_widget_catalog/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_window_examples_interaction_states")
    set_kind("binary")
    add_files("examples/api_parity/public_window_examples_interaction_states/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("api_parity_public_window_examples_service_matrix")
    set_kind("binary")
    add_files("examples/api_parity/public_window_examples_service_matrix/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)

target("render_view_test")
    set_kind("binary")
    add_files("tests/ui/render_view_test.cpp")
    add_deps("cgpui_core", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("element_test")
    set_kind("binary")
    add_files("tests/ui/element_test.cpp")
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("static_element_tree_test")
    set_kind("binary")
    add_files("tests/ui/static_element_tree_test.cpp")
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("static_render_runtime_test")
    set_kind("binary")
    add_files("tests/ui/static_render_runtime_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_includedirs("tests/ui")
    add_tests("default")

target("builtin_widget_test")
    set_kind("binary")
    add_files("tests/ui/builtin_widget_test.cpp")
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("layout_test")
    set_kind("binary")
    add_files("tests/ui/layout_test.cpp")
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("style_test")
    set_kind("binary")
    add_files("tests/ui/style_test.cpp")
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("text_model_test")
    set_kind("binary")
    add_files("tests/ui/text_model_test.cpp")
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("rich_text_run_test")
    set_kind("binary")
    add_files("tests/ui/rich_text_run_test.cpp")
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("scroll_test")
    set_kind("binary")
    add_files("tests/ui/scroll_test.cpp")
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_input_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_input_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_focus_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_focus_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_actions_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_actions_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_observation_diagnostics_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_observation_diagnostics_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_observation_closure_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_observation_closure_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_view_handle_token_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_view_handle_token_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_text_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_text_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_rendering_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_rendering_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_scheduling_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_scheduling_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_frame_scheduling_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/ui/window_runtime_frame_scheduling_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs, "tests/ui")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("window_runtime_multiwindow_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_multiwindow_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("window_runtime_theme_test")
    set_kind("binary")
    add_files("tests/ui/window_runtime_theme_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("app_runner_test")
    set_kind("binary")
    add_files("tests/ui/app_runner_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("hello_window_lifetime_test")
    set_kind("binary")
    add_files("tests/architecture/hello_window_lifetime_test.cpp")
    add_tests("default")

target("wayland_window_source_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/architecture/wayland_window_source_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("win32_window_source_test")
    set_kind("binary")
    add_files("tests/architecture/win32_window_source_test.cpp")
    add_tests("default")

target("desktop_target_readiness_test")
    set_kind("binary")
    add_files("tests/architecture/desktop_target_readiness_test.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
    add_includedirs(public_includedirs)
    add_tests("default")

target("ui_source_structure_test")
    set_kind("binary")
    add_files("tests/architecture/ui_source_structure_test.cpp")
    add_tests("default")

target("zero_cost_abstraction_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/architecture/zero_cost_abstraction_structure_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("widget_source_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/architecture/widget_source_structure_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("platform_source_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/architecture/platform_source_structure_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("platform_font_discovery_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/architecture/platform_font_discovery_structure_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("renderer_source_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files("tests/architecture/renderer_source_structure_test.cpp")
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

if is_plat("windows") then
    target("vulkan_resize_test")
        set_kind("binary")
        add_files("tests/renderer/vulkan_resize_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(public_includedirs)
        add_syslinks("user32")
        add_tests("default")

    target("vulkan_surface_validation_test")
        set_kind("binary")
        add_files("tests/renderer/vulkan_surface_validation_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(public_includedirs)
        add_tests("default")

    target("win32_vulkan_surface_test")
        set_kind("binary")
        add_files("tests/renderer/win32_vulkan_surface_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(public_includedirs)
        add_syslinks("user32", "gdi32")
        add_tests("default")

    target("vulkan_frame_lifetime_test")
        set_kind("binary")
        add_files("tests/renderer/vulkan_frame_lifetime_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(public_includedirs)
        add_syslinks("user32")
        add_tests("default")

    target("vulkan_solid_rect_test")
        set_kind("binary")
        add_files("tests/renderer/vulkan_solid_rect_test.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(public_includedirs)
        add_syslinks("user32", "gdi32")
        add_tests("default")
end

target("hello_window")
    set_kind("binary")
    add_files("examples/hello_window/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
        add_tests("windows_first_frame", {runenvs = {CGPUI_EXIT_AFTER_FIRST_FRAME = "1"}})
        add_tests("windows_resize_after_first_frame", {runenvs = {CGPUI_RESIZE_AFTER_FIRST_FRAME = "1"}})
        add_tests("windows_close_after_first_frame", {runenvs = {CGPUI_CLOSE_AFTER_FIRST_FRAME = "1"}})
        add_tests("windows_demo_smoke_flow", {runenvs = {CGPUI_DEMO_SMOKE_FLOW = "1", CGPUI_CLOSE_AFTER_FIRST_FRAME = "0"}})
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
        add_tests("linux_first_frame", {runenvs = {CGPUI_EXIT_AFTER_FIRST_FRAME = "1"}})
        add_tests("linux_resize_after_first_frame", {runenvs = {CGPUI_RESIZE_AFTER_FIRST_FRAME = "1"}})
        add_tests("linux_close_after_first_frame", {runenvs = {CGPUI_CLOSE_AFTER_FIRST_FRAME = "1"}})
        add_tests("linux_demo_smoke_flow", {runenvs = {CGPUI_DEMO_SMOKE_FLOW = "1", CGPUI_CLOSE_AFTER_FIRST_FRAME = "0"}})
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)
