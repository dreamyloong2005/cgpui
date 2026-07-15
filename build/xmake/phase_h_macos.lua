if is_plat("macosx") then
    target("cgpui_platform")
        add_files(path.join(os.projectdir(), "src/platform/macos/macos_clipboard.mm"))
        add_frameworks("AppKit")

    target("cgpui_platform_macos")
        set_kind("static")
        add_files(path.join(os.projectdir(), "src/platform/macos/*.mm"))
        remove_files(path.join(os.projectdir(), "src/platform/macos/macos_clipboard.mm"))
        add_deps("cgpui_core", "cgpui_platform")
        add_frameworks(
            "AppKit", "CoreText", "UniformTypeIdentifiers", "QuartzCore")
        add_includedirs(path.join(os.projectdir(), "include"), {public = true})

    target("macos_window_lifecycle_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/macos/macos_window_lifecycle_test.mm"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_macos")
        add_frameworks("AppKit", "QuartzCore")
        add_includedirs(path.join(os.projectdir(), "include"))
        add_tests("default")

    target("macos_input_event_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/macos/macos_input_event_test.mm"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_macos")
        add_frameworks("AppKit", "QuartzCore")
        add_includedirs(
            path.join(os.projectdir(), "include"),
            path.join(os.projectdir(), "src/platform/macos"))
        add_tests("default")

    target("macos_font_discovery_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/macos/macos_font_discovery_test.mm"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_macos")
        add_frameworks("AppKit", "CoreText", "QuartzCore")
        add_includedirs(
            path.join(os.projectdir(), "include"),
            path.join(os.projectdir(), "src/platform/macos"))
        add_tests("default")

    target("macos_text_input_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/macos/macos_text_input_test.mm"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_macos")
        add_frameworks("AppKit", "CoreText", "QuartzCore")
        add_includedirs(
            path.join(os.projectdir(), "include"),
            path.join(os.projectdir(), "src/platform/macos"))
        add_tests("default")

    target("macos_clipboard_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/macos/macos_clipboard_test.mm"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_macos")
        add_frameworks("AppKit", "QuartzCore")
        add_includedirs(
            path.join(os.projectdir(), "include"),
            path.join(os.projectdir(), "src/platform/macos"))
        add_tests("default")

    target("macos_drag_drop_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/macos/macos_drag_drop_test.mm"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_macos")
        add_frameworks("AppKit", "QuartzCore")
        add_includedirs(
            path.join(os.projectdir(), "include"),
            path.join(os.projectdir(), "src/platform/macos"))
        add_tests("default")

    target("macos_platform_services_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/macos/macos_platform_services_test.mm"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_macos")
        add_frameworks("AppKit", "UniformTypeIdentifiers", "QuartzCore")
        add_includedirs(
            path.join(os.projectdir(), "include"),
            path.join(os.projectdir(), "src/platform/macos"))
        add_tests("default")

    target("macos_accessibility_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/macos/macos_accessibility_test.mm"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_macos")
        add_frameworks("AppKit", "QuartzCore")
        add_includedirs(
            path.join(os.projectdir(), "include"),
            path.join(os.projectdir(), "src/platform/macos"))
        add_tests("default")

    target("macos_example_smoke_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/macos/macos_example_smoke_test.mm"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_macos", "cgpui_renderer", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
        add_includedirs(
            path.join(os.projectdir(), "include"),
            path.join(os.projectdir(), "src/platform/macos"))
        add_tests("default")
end
