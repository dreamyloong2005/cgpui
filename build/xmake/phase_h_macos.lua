if is_plat("macosx") then
    target("cgpui_platform_macos")
        set_kind("static")
        add_files(path.join(os.projectdir(), "src/platform/macos/*.mm"))
        add_deps("cgpui_core", "cgpui_platform")
        add_frameworks("AppKit", "QuartzCore")
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
end
