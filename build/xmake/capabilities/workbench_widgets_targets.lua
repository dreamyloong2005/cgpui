-- Workbench widgets (C9): splitter, tab strip, tree view, tooltip and
-- popover / context menu. Widgets own geometry, input and paint; a
-- WidgetTreeBuilder publishes their routing / focus / accessibility nodes.
local public_includedirs = "include"

target("cgpui_ui")
    add_files(path.join(os.projectdir(), "src/ui/workbench/*.cpp"))
target_end()

local workbench_behavior_tests = {
    "workbench_splitter_test",
    "workbench_tab_strip_test",
    "workbench_tree_view_test",
    "workbench_tooltip_test",
    "workbench_popover_menu_test",
}
for _, name in ipairs(workbench_behavior_tests) do
    target(name)
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/ui/" .. name .. ".cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
        add_includedirs(path.join(os.projectdir(), public_includedirs))
        add_tests("default")
end

target("workbench_widgets_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/workbench_widgets_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("ui_anchored_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/ui_anchored_header_cleanliness.cpp"))
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("ui_popover_menu_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/ui_popover_menu_header_cleanliness.cpp"))
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("ui_splitter_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/ui_splitter_header_cleanliness.cpp"))
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("ui_tab_strip_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/ui_tab_strip_header_cleanliness.cpp"))
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("ui_tooltip_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/ui_tooltip_header_cleanliness.cpp"))
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("ui_tree_view_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/ui_tree_view_header_cleanliness.cpp"))
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("ui_widget_tree_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/ui_widget_tree_header_cleanliness.cpp"))
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("ui_workbench_theme_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/ui_workbench_theme_header_cleanliness.cpp"))
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("ui_workbench_widgets_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/ui_workbench_widgets_header_cleanliness.cpp"))
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

-- Built, not run: it needs a real window and a human to look at it.
if is_plat("windows", "linux") then
    target("example_workbench")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "examples/workbench/main.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app",
                 "cgpui_renderer_vulkan")
        if is_plat("windows") then
            add_deps("cgpui_platform_win32")
        else
            add_deps("cgpui_platform_linux_wayland")
        end
        add_includedirs(path.join(os.projectdir(), public_includedirs))
end
