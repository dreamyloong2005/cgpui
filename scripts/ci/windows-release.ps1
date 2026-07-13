[CmdletBinding()]
param(
  [string]$OutputRoot = ""
)

& (Join-Path $PSScriptRoot "windows-package.ps1") `
  -Mode Release -OutputRoot $OutputRoot
