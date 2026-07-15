if is_plat("macosx") then
    target("cgpui_renderer_metal")
        set_kind("static")
        add_files(path.join(os.projectdir(), "src/renderer/metal/*.mm"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
        add_frameworks("Metal", "QuartzCore")
        add_includedirs(path.join(os.projectdir(), "include"), {public = true})
        on_load(function (target)
            target:add("includedirs", target:autogendir())
        end)
        before_build(function (target)
            local sourcefile = path.join(os.projectdir(), "src/renderer/metal/shaders.metal")
            local source = io.readfile(sourcefile)
            local generated = target:autogendir()
            local header = path.join(generated, "metal_shaders_source.hpp")
            local air = path.join(generated, "shaders.air")
            local library = path.join(generated, "shaders.metallib")
            os.mkdir(generated)
            local header_contents =
                "#pragma once\n\ninline constexpr char kCgpuiMetalShaderSource[] = R\"CGPUI_METAL(\n" ..
                source .. "\n)CGPUI_METAL\";\n"
            if not os.isfile(header) or io.readfile(header) ~= header_contents then
                io.writefile(header, header_contents)
            end
            if not os.isfile(library) or os.mtime(library) < os.mtime(sourcefile) then
                os.vrunv("xcrun", {"-sdk", "macosx", "metal", "-c", sourcefile, "-o", air})
                os.vrunv("xcrun", {"-sdk", "macosx", "metallib", air, "-o", library})
            end
        end)

    target("metal_bootstrap_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/metal/metal_bootstrap_test.mm"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_macos", "cgpui_renderer", "cgpui_renderer_metal")
        add_frameworks("AppKit", "Metal", "QuartzCore")
        add_includedirs(
            path.join(os.projectdir(), "include"),
            path.join(os.projectdir(), "src/renderer/metal"))
        add_tests("default")

    for _, metal_pixel_test in ipairs({
        "metal_primitive_pixel_test",
        "metal_text_image_pixel_test",
        "metal_clip_transform_pixel_test"
    }) do
        target(metal_pixel_test)
            set_kind("binary")
            add_files(path.join(os.projectdir(), "tests/renderer/metal/" .. metal_pixel_test .. ".mm"))
            add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_macos", "cgpui_renderer", "cgpui_renderer_metal")
            add_frameworks("AppKit", "Metal", "QuartzCore")
            add_includedirs(
                path.join(os.projectdir(), "include"),
                path.join(os.projectdir(), "tests/renderer/metal"))
            add_tests("default")
    end
end
