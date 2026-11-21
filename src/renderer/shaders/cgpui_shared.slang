// Helpers shared by the Vulkan and Metal shaders of CGPUI. Compiled into
// each entry point by scripts/dev/compile_shaders.py; do not include GPU
// API attributes here.
module cgpui_shared;

// Radii are {top left, top right, bottom right, bottom left}; y points down.
// p is relative to the rect's center.
public float corner_radius(float2 p, float4 radii) {
  if (p.x < 0.0) {
    return p.y < 0.0 ? radii.x : radii.w;
  }
  return p.y < 0.0 ? radii.y : radii.z;
}

// Signed distance from point to a rounded rect given as {min.xy, max.xy}.
public float rounded_rect_distance(float2 point, float4 rect, float4 radii) {
  float2 half_size = (rect.zw - rect.xy) * 0.5;
  float2 p = point - (rect.xy + rect.zw) * 0.5;
  float r = corner_radius(p, radii);
  float2 q = abs(p) - half_size + r;
  return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

// One device pixel measured in local units, so pixel-aligned edges are
// exact and the antialiasing width follows the transform scale.
public float local_pixel_width(float2 local) {
  return max(length(fwidth(local)) * 0.7071068, 0.0001);
}

// Glyph atlas coverage applied to a straight-alpha color. The coverage gamma
// is 1.0: the atlas holds linear coverage (vulkan_text_coverage_policy).
public static const float coverage_gamma = 1.0;

public float4 glyph_color(float4 color, float atlas_coverage) {
  float coverage = pow(clamp(atlas_coverage, 0.0, 1.0), coverage_gamma);
  return float4(color.rgb, color.a * coverage);
}
