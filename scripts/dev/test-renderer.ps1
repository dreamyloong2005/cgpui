# Private parser for test.ps1. Never consumes positional mcpp filters.
function Get-CgpuiTestRendererPlan {
  param([object[]]$Arguments)
  $selectors = @(for ($i = 0; $i -lt $Arguments.Count; $i++) {
    if ($Arguments[$i] -ieq '-Renderer') { $i }
    elseif ("$($Arguments[$i])" -imatch '^-Renderer[:=]') {
      throw '-Renderer requires a separate value: auto, vulkan, dx12, or both'
    }
  })
  if ($selectors.Count -gt 1) { throw '-Renderer may only be specified once' }
  $renderer = $null
  if ($selectors.Count -eq 1) {
    $index = $selectors[0]
    if ($index + 1 -ge $Arguments.Count -or
        $Arguments[$index + 1] -cnotin @('auto', 'vulkan', 'dx12', 'both')) {
      throw '-Renderer must be auto, vulkan, dx12, or both'
    }
    $renderer = $Arguments[$index + 1]
    $Arguments = @(for ($i = 0; $i -lt $Arguments.Count; $i++) {
      if ($i -ne $index -and $i -ne $index + 1) { $Arguments[$i] }
    })
  }
  return [pscustomobject]@{ Renderer = $renderer; Arguments = @($Arguments) }
}
