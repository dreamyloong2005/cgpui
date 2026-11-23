param([switch]$Asan, [switch]$AsanSymbols)
$ErrorActionPreference = 'Stop'
if ($env:MOCK_NATIVE_PREFERENCE) {
  $PSNativeCommandUseErrorActionPreference = [bool]::Parse($env:MOCK_NATIVE_PREFERENCE)
}
function Write-MockCall($Kind, $Arguments) {
  $entry = @{ kind = $Kind; arguments = @($Arguments) }
  [IO.File]::AppendAllText($env:MOCK_LOG, (($entry | ConvertTo-Json -Compress) + "`n"))
}
function global:python {
  Write-MockCall 'python' $args
  $global:LASTEXITCODE = [int]$env:MOCK_PYTHON_EXIT
  if ($global:LASTEXITCODE -eq 0) {
    New-Item -ItemType Directory -Path $args[2] -Force | Out-Null
  }
}
function global:cl.exe {
  Write-MockCall 'cl' $args
  $global:LASTEXITCODE = [int]$env:MOCK_CL_EXIT
}
function Read-TestState {
  $nativePreferenceVariable = Get-Variable PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue
  $nativePreferenceValue = $null
  if ($null -ne $nativePreferenceVariable) { $nativePreferenceValue = $nativePreferenceVariable.Value }
  return @{
    renderer = $env:CGPUI_RENDERER; path = $env:PATH
    compile_commands = $env:MCPP_COMPILE_COMMANDS; home = $env:MCPP_HOME
    cwd = (Get-Location).Path
    native_preference = $nativePreferenceValue
    native_preference_exists = ($null -ne $nativePreferenceVariable)
  }
}
$before = Read-TestState
$hostInfo = @{ executable = (Get-Process -Id $PID).Path
               version = $PSVersionTable.PSVersion.ToString(); edition = $PSVersionTable.PSEdition }
$code = 0
if ($env:MOCK_STRICT_MODE) { Set-StrictMode -Version Latest }
try {
  & $env:MOCK_DRIVER -Asan:$Asan -AsanSymbols:$AsanSymbols @args
  $code = $LASTEXITCODE
} catch {
  Write-Host $_.Exception.Message
  $code = 1
} finally {
  Set-StrictMode -Off
  @{ host = $hostInfo; before = $before; after = (Read-TestState) } | ConvertTo-Json -Depth 4 |
    Set-Content -LiteralPath $env:MOCK_STATE -Encoding utf8
}
exit $code
