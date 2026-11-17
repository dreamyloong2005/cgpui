#version 450

// One triangle covering the viewport; fragments use gl_FragCoord.
void main() {
  vec2 corner = vec2(float((gl_VertexIndex << 1) & 2),
                     float(gl_VertexIndex & 2));
  gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);
}
