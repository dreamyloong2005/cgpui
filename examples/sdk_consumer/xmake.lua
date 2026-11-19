-- CGPUI SDK consumer sample (a consumer may build with xmake; cgpui itself
-- builds with mcpp): build with
--   CGPUI_SDK=<sdk> xmake f -m <debug|release> && xmake
-- The include directory, libraries in link order and system libraries come
-- from the package's manifest.json.
set_project("cgpui_sdk_consumer")
set_languages("c++23")
add_rules("mode.debug", "mode.release")

target("sdk_consumer")
    set_kind("binary")
    add_files("main.cpp")
    on_load(function (target)
        import("core.base.json")
        local sdk = os.getenv("CGPUI_SDK")
        assert(sdk, "set CGPUI_SDK to the extracted package")
        local manifest = json.loadfile(path.join(sdk, "manifest.json"))
        target:add("includedirs", path.join(sdk, "include"))
        target:add("linkdirs", path.join(sdk, "lib"))
        for _, name in ipairs(manifest.link_names) do
            target:add("links", name)
        end
        for _, name in ipairs(manifest.system_libraries) do
            target:add("syslinks", name)
        end
        if manifest.platform == "windows" then
            -- The package's CRT (/MD in both packages: mcpp has no debug CRT).
            target:set("runtimes", manifest.msvc_runtime)
            target:add("linkdirs", path.join(os.getenv("VULKAN_SDK"), "Lib"))
        end
    end)
