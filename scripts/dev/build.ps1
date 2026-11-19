# Build the cgpui workspace on Windows: scripts/dev/build.ps1 [mcpp build args]
#   scripts/dev/build.ps1                      every member (dev profile, /MT)
#   scripts/dev/build.ps1 -p packages/ui       one member and what it needs
#   scripts/dev/build.ps1 --release            release profile
. (Join-Path $PSScriptRoot 'mcpp-env.ps1')
# compile_commands.json at the repository root, for editors.
if (-not $env:MCPP_COMPILE_COMMANDS) { $env:MCPP_COMPILE_COMMANDS = 'compile_commands.json' }
Push-Location $RepoRoot
try {
  & $Mcpp build @args
  exit $LASTEXITCODE
} finally {
  Pop-Location
}
