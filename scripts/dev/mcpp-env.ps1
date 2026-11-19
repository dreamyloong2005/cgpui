# Environment for running mcpp on Windows; dot-sourced by build.ps1/test.ps1.
#
# MCPP_HOME   mcpp's home (toolchains, caches). Default: D:\Dev\Tools\mcpp-home,
#             kept off C: on the reference machine. Set it to override.
# MCPP        mcpp.exe to run. Default: the first mcpp.exe on PATH, else
#             D:\Dev\Tools\mcpp\mcpp-2026.11.18.1-windows-x86_64\bin\mcpp.exe.
# VSLANG=1033 makes MSVC print English /showIncludes notes: mcpp records header
#             dependencies from them, and records none when they are localized
#             (https://github.com/mcpp-community/mcpp/issues/970); mcpp also
#             fails on diagnostics that are not UTF-8.
$ErrorActionPreference = 'Stop'
if (-not $env:MCPP_HOME) { $env:MCPP_HOME = 'D:\Dev\Tools\mcpp-home' }
$env:VSLANG = '1033'
if (-not $env:MCPP_COMPILE_COMMANDS) { $env:MCPP_COMPILE_COMMANDS = 'compile_commands.json' }
$script:Mcpp = $env:MCPP
if (-not $script:Mcpp) {
  $found = Get-Command mcpp.exe -ErrorAction SilentlyContinue
  $script:Mcpp = if ($found) { $found.Source } else {
    'D:\Dev\Tools\mcpp\mcpp-2026.11.18.1-windows-x86_64\bin\mcpp.exe' }
}
if (-not (Test-Path $script:Mcpp)) { throw "mcpp not found: $script:Mcpp (set MCPP)" }
$script:RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
