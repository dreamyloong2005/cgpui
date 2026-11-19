# Build and run the cgpui tests on Windows: scripts/dev/test.ps1 [mcpp test args]
#   scripts/dev/test.ps1                       every test program of every member
#   scripts/dev/test.ps1 -p tests/ui           one suite (members by path)
#   scripts/dev/test.ps1 -p tests/ui clipboard tests whose name matches
. (Join-Path $PSScriptRoot 'mcpp-env.ps1')
Push-Location $RepoRoot
try {
  & $Mcpp test @args
  exit $LASTEXITCODE
} finally {
  Pop-Location
}
