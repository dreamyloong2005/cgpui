function Set-CgpuiWindowsDependencyEnvironment {
  # The mcpp environment of the CI scripts (scripts/dev/mcpp-env.ps1):
  # MCPP_HOME, VSLANG=1033 and the mcpp executable, which it returns.
  # Dependencies are the MSVC toolset (msvc@system), the LunarG Vulkan SDK
  # (VULKAN_SDK) and the vendored third_party/ libraries; mcpp downloads
  # nothing for them.
  [CmdletBinding()]
  param(
    [switch]$ExportGitHubEnvironment
  )

  $ErrorActionPreference = "Stop"
  Set-StrictMode -Version Latest

  if ($env:OS -ne "Windows_NT") {
    throw "windows-dependencies.ps1 requires Windows"
  }
  . (Join-Path $PSScriptRoot "..\dev\mcpp-env.ps1")
  if ([string]::IsNullOrWhiteSpace($env:VULKAN_SDK)) {
    throw "VULKAN_SDK is not set; install the LunarG Vulkan SDK"
  }

  if ($ExportGitHubEnvironment) {
    if ([string]::IsNullOrWhiteSpace($env:GITHUB_ENV)) {
      throw "GITHUB_ENV is required when exporting dependency variables"
    }
    $utf8 = [System.Text.UTF8Encoding]::new($false)
    foreach ($name in @("MCPP_HOME", "VSLANG")) {
      [System.IO.File]::AppendAllText(
        $env:GITHUB_ENV,
        "$name=$([System.Environment]::GetEnvironmentVariable($name))`n",
        $utf8
      )
    }
    [System.IO.File]::AppendAllText($env:GITHUB_ENV, "MCPP=$Mcpp`n", $utf8)
  }
  return $Mcpp
}
