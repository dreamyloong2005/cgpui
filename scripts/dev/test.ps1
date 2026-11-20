# Build and run the cgpui tests on Windows: scripts/dev/test.ps1 [-Asan] [mcpp test args]
#   scripts/dev/test.ps1                       every test program of every member
#   scripts/dev/test.ps1 -p tests/ui           one suite (members by path)
#   scripts/dev/test.ps1 -p tests/ui clipboard tests whose name matches
#   scripts/dev/test.ps1 -Asan [...]           the same under AddressSanitizer
#   scripts/dev/test.ps1 -AsanSymbols [...]    ASan with PDBs (symbolized reports)
# -Asan builds in build/asan (scripts/dev/asan_workspace.py: every member,
# /fsanitize=address, /MD) and puts the MSVC toolset's bin directory, which
# holds clang_rt.asan_dynamic-x86_64.dll, first on PATH for the test
# programs. -AsanSymbols links with /DEBUG:FULL in build/asan-symbols (a
# PDB is ~45 MB per program: use it with -p for a failing suite). See
# docs/getting-started.md, section 7.
param([switch]$Asan, [switch]$AsanSymbols)
. (Join-Path $PSScriptRoot 'mcpp-env.ps1')
$workDir = $RepoRoot
$mcppArgs = @($args)
if ($Asan -or $AsanSymbols) {
  $workDir = Join-Path $RepoRoot $(if ($AsanSymbols) { 'build\asan-symbols' } else { 'build\asan' })
  $symbols = @(if ($AsanSymbols) { '--symbols' })
  & python (Join-Path $PSScriptRoot 'asan_workspace.py') windows $workDir @symbols
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  . (Join-Path $PSScriptRoot 'msvc-asan-runtime.ps1')
  $env:PATH = (Get-MsvcAsanRuntimeDir) + ';' + $env:PATH
  # The entry every ASan program begins at (asan_entry_windows.cpp: it ends
  # the process without the DLL detach that hangs under ASan).
  & cl.exe /nologo /c /MD /O1 /GS- /EHs-c- (Join-Path $PSScriptRoot 'asan_entry_windows.cpp') `
    "/Fo$(Join-Path $workDir 'asan_entry.obj')" | Out-Null
  if ($LASTEXITCODE -ne 0) { throw 'cl.exe failed on scripts/dev/asan_entry_windows.cpp' }
  # Members are listed relative to build/asan: -p tests/ui => ../../tests/ui.
  for ($i = 0; $i -lt $mcppArgs.Count - 1; $i++) {
    if ($mcppArgs[$i] -eq '-p' -and -not "$($mcppArgs[$i + 1])".StartsWith('..')) {
      $mcppArgs[$i + 1] = '../../' + $mcppArgs[$i + 1]
    }
  }
} elseif (-not $env:MCPP_COMPILE_COMMANDS) {
  # compile_commands.json at the repository root, for editors.
  $env:MCPP_COMPILE_COMMANDS = 'compile_commands.json'
}
Push-Location $workDir
try {
  & $Mcpp test @mcppArgs
  exit $LASTEXITCODE
} finally {
  Pop-Location
}
