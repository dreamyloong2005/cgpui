function Set-CgpuiWindowsDependencyEnvironment {
  [CmdletBinding()]
  param(
    [string]$DependencyRoot = "",
    [switch]$ExportGitHubEnvironment
  )

  $ErrorActionPreference = "Stop"
  Set-StrictMode -Version Latest

  if ($env:OS -ne "Windows_NT") {
    throw "windows-dependencies.ps1 requires Windows"
  }

  $repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
  if ([string]::IsNullOrWhiteSpace($DependencyRoot)) {
    $DependencyRoot = Join-Path $repoRoot `
      "build\phase-g-ci\dependencies\windows"
  } elseif (-not [System.IO.Path]::IsPathRooted($DependencyRoot)) {
    $DependencyRoot = Join-Path $repoRoot $DependencyRoot
  }
  $resolvedRoot = [System.IO.Path]::GetFullPath($DependencyRoot)
  $repoPrefix = $repoRoot.TrimEnd('\', '/') +
    [System.IO.Path]::DirectorySeparatorChar
  if ($resolvedRoot -eq $repoRoot -or
      -not $resolvedRoot.StartsWith(
        $repoPrefix,
        [System.StringComparison]::OrdinalIgnoreCase
      )) {
    throw "dependency root must stay inside the repository: $resolvedRoot"
  }

  $values = [ordered]@{
    XMAKE_GLOBALDIR = Join-Path $resolvedRoot "global"
    XMAKE_PKG_CACHEDIR = Join-Path $resolvedRoot "pkg-cache"
    XMAKE_PKG_INSTALLDIR = Join-Path $resolvedRoot "pkg-install"
  }
  New-Item -ItemType Directory -Force -Path @($values.Values) | Out-Null
  foreach ($entry in $values.GetEnumerator()) {
    [System.Environment]::SetEnvironmentVariable(
      $entry.Key,
      $entry.Value,
      [System.EnvironmentVariableTarget]::Process
    )
  }

  if ($ExportGitHubEnvironment) {
    if ([string]::IsNullOrWhiteSpace($env:GITHUB_ENV)) {
      throw "GITHUB_ENV is required when exporting dependency variables"
    }
    $utf8 = [System.Text.UTF8Encoding]::new($false)
    foreach ($entry in $values.GetEnumerator()) {
      [System.IO.File]::AppendAllText(
        $env:GITHUB_ENV,
        "$($entry.Key)=$($entry.Value)`n",
        $utf8
      )
    }
  }
}
