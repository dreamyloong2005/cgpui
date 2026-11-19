-- Dump every xmake target of the project as JSON for the mcpp migration.
-- Usage (from the repository root, with xmake configured for the host):
--   xmake l scripts/migration/dump_xmake_targets.lua <plat> <out.json>
-- <plat> is "windows" or "linux"; is_plat() conditions in xmake.lua are
-- evaluated for that platform.
import("core.base.json")
import("core.project.config")
import("core.project.project")

local function list(value)
    if value == nil then
        return {}
    end
    if type(value) ~= "table" then
        return {value}
    end
    return value
end

local function relative(files)
    local result = {}
    for _, file in ipairs(list(files)) do
        table.insert(result, (path.relative(file, os.projectdir()):gsub("\\", "/")))
    end
    table.sort(result)
    return result
end

function main(plat, out)
    config.load()
    config.set("plat", plat, {force = true})
    if plat == "linux" then
        config.set("arch", "x86_64", {force = true})
    end
    local targets = {}
    for name, target in pairs(project.targets()) do
        local tests = {}
        for _, test in ipairs(list(target:get("tests"))) do
            local extra = target:extraconf("tests", test) or {}
            table.insert(tests, {
                name = test,
                rundir = extra.rundir and path.relative(extra.rundir, os.projectdir()) or json.null,
                runenvs = extra.runenvs and table.keys(extra.runenvs) or {},
                runargs = list(extra.runargs)})
        end
        table.insert(targets, {
            name = name,
            kind = target:kind(),
            files = relative(target:sourcefiles()),
            headerfiles = relative(target:headerfiles()),
            group = target:get("group") or json.null,
            deps = list(target:get("deps")),
            packages = list(target:get("packages")),
            syslinks = list(target:get("syslinks")),
            links = list(target:get("links")),
            defines = list(target:get("defines")),
            cxflags = list(target:get("cxflags")),
            cxxflags = list(target:get("cxxflags")),
            ldflags = list(target:get("ldflags")),
            includedirs = relative(target:get("includedirs")),
            rules = list(target:get("rules")),
            rundir = target:get("rundir") and path.relative(target:get("rundir"), os.projectdir()) or json.null,
            runenvs = table.keys(target:get("runenvs") or {}),
            default = target:get("default") ~= false,
            tests = tests})
    end
    table.sort(targets, function (a, b) return a.name < b.name end)
    json.savefile(out, {platform = plat, count = #targets, targets = targets})
    print("%s: %d targets -> %s", plat, #targets, out)
end
