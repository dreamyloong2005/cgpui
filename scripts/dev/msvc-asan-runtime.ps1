# Get-MsvcAsanRuntimeDir: the bin directory (Hostx64\x64) of the MSVC
# toolset msvc@system resolves to, which holds the ASan runtime DLL
# (clang_rt.asan_dynamic-x86_64.dll) that /fsanitize=address programs import.
# Dot-sourced by test.ps1 -Asan. vswhere finds the latest installation;
# Microsoft.VCToolsVersion.default.txt names its default toolset.
function Get-MsvcAsanRuntimeDir {
  $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
  if (-not (Test-Path -LiteralPath $vswhere)) { throw "vswhere.exe not found: $vswhere" }
  $vs = (& $vswhere -latest -products '*' -property installationPath | Select-Object -First 1)
  if (-not $vs) { throw 'no Visual Studio installation with the MSVC toolset' }
  $version = (Get-Content -LiteralPath (Join-Path $vs 'VC\Auxiliary\Build\Microsoft.VCToolsVersion.default.txt') |
    Select-Object -First 1).Trim()
  $dir = Join-Path $vs "VC\Tools\MSVC\$version\bin\Hostx64\x64"
  if (-not (Test-Path -LiteralPath (Join-Path $dir 'clang_rt.asan_dynamic-x86_64.dll'))) {
    throw "clang_rt.asan_dynamic-x86_64.dll not found in $dir (install the C++ AddressSanitizer component)"
  }
  return $dir
}
