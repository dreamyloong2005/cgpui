$entry = [ordered]@{
  kind = 'mcpp'; arguments = @($args); renderer = $env:CGPUI_RENDERER
  cwd = (Get-Location).Path; path = $env:PATH
  compile_commands = $env:MCPP_COMPILE_COMMANDS; export = $env:MCPP_EXPORT
}
[IO.File]::AppendAllText($env:MOCK_LOG, (($entry | ConvertTo-Json -Compress) + "`n"))
if ($env:MOCK_THROW -and $env:MOCK_THROW -eq $env:CGPUI_RENDERER) { throw 'mock invocation exception' }
$code = [Environment]::GetEnvironmentVariable("MOCK_EXIT_$($env:CGPUI_RENDERER)")
if (-not $code) { $code = $env:MOCK_EXIT_DEFAULT }
exit ([int]$code)
