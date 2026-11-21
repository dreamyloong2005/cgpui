# Environment for running mcpp on Windows; dot-sourced by build.ps1, test.ps1
# and scripts/ci/windows-dependencies.ps1. Sets $Mcpp and $RepoRoot.
#
# MCPP_HOME   mcpp's home (toolchains, caches). Default: D:\Dev\Tools\mcpp-home
#             when it exists (the reference machine keeps it off C:), else
#             mcpp's own default (%USERPROFILE%\.mcpp). Set it to override.
# MCPP        mcpp.exe to run. Default: mcpp.exe on PATH, else
#             D:\Dev\Tools\mcpp\mcpp-2026.11.20.1-windows-x86_64\bin\mcpp.exe.
# cl.exe speaks the user's language: since 2026.11.19.1 mcpp reads header
# dependencies from /sourceDependencies and decodes the compiler's text in the
# console code page (mcpp#970), so VSLANG is not set here.
$ErrorActionPreference = 'Stop'
if (-not $env:MCPP_HOME -and (Test-Path -LiteralPath 'D:\Dev\Tools\mcpp-home')) {
  $env:MCPP_HOME = 'D:\Dev\Tools\mcpp-home'
}
$Mcpp = $env:MCPP
if (-not $Mcpp) {
  $found = Get-Command mcpp.exe -ErrorAction SilentlyContinue
  $Mcpp = if ($found) { $found.Source } else {
    'D:\Dev\Tools\mcpp\mcpp-2026.11.20.1-windows-x86_64\bin\mcpp.exe' }
}
if (-not (Test-Path -LiteralPath $Mcpp)) { throw "mcpp not found: $Mcpp (set MCPP)" }
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
