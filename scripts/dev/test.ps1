# Build and run the cgpui tests on Windows: scripts/dev/test.ps1 [-Asan] [mcpp test args]
#   scripts/dev/test.ps1                       every test program of every member
#   scripts/dev/test.ps1 -p tests/ui           one suite (members by path)
#   scripts/dev/test.ps1 -p tests/ui clipboard tests whose name matches
#   scripts/dev/test.ps1 -Asan [...]           the same under AddressSanitizer
#   scripts/dev/test.ps1 -AsanSymbols [...]    ASan with PDBs (symbolized reports)
#   scripts/dev/test.ps1 -Renderer both [...]  Vulkan then DX12; default stays unchanged
# -Asan builds in build/asan (scripts/dev/asan_workspace.py: every member,
# /fsanitize=address, /MD) and puts the MSVC toolset's bin directory, which
# holds clang_rt.asan_dynamic-x86_64.dll, first on PATH for the test
# programs. -AsanSymbols links with /DEBUG:FULL in build/asan-symbols (a
# PDB is ~45 MB per program: use it with -p for a failing suite). See
# docs/getting-started.md, section 7.
param([switch]$Asan, [switch]$AsanSymbols)
. (Join-Path $PSScriptRoot 'test-renderer.ps1')
$plan = Get-CgpuiTestRendererPlan -Arguments @($args)
$mcppArgs = @($plan.Arguments)
# Restore process state even when called from another PowerShell script.
$savedEnvironment = @{}
foreach ($name in @('CGPUI_RENDERER', 'PATH', 'MCPP_COMPILE_COMMANDS', 'MCPP_HOME')) {
  $savedEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
}
try {
  . (Join-Path $PSScriptRoot 'mcpp-env.ps1')
  $workDir = $RepoRoot
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
  $renderers = if ($plan.Renderer -eq 'both') { @('vulkan', 'dx12') }
               elseif ($null -ne $plan.Renderer) { @($plan.Renderer) }
               else { @($null) }
  $result = 0
  Push-Location $workDir
  try {
    foreach ($renderer in @($renderers)) {
      if ($null -ne $renderer) {
        $env:CGPUI_RENDERER = $renderer
        Write-Host "CGPUI test run: renderer=$renderer"
      }
      # Native nonzero exits are test results, not preference-driven exceptions.
      $nativeExitPreference = Get-Variable PSNativeCommandUseErrorActionPreference -ValueOnly -ErrorAction SilentlyContinue
      try {
        $PSNativeCommandUseErrorActionPreference = $false
        & $Mcpp test @mcppArgs
        $runExitCode = $LASTEXITCODE
      } finally {
        $PSNativeCommandUseErrorActionPreference = $nativeExitPreference
      }
      if ($runExitCode -ne 0 -and $result -eq 0) { $result = $runExitCode }
    }
  } finally {
    Pop-Location
  }
  exit $result
} finally {
  foreach ($name in $savedEnvironment.Keys) {
    if ($null -eq $savedEnvironment[$name]) {
      Remove-Item -LiteralPath "Env:$name" -ErrorAction SilentlyContinue
    } else {
      [Environment]::SetEnvironmentVariable($name, $savedEnvironment[$name], 'Process')
    }
  }
}
