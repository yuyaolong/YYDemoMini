#version 320 es
precision highp float;
uniform sampler2D srcTex;
in vec2 v_texCoor;
out vec4 outColor;
void main()
{
   // outColor = vec4(float(gl_FragCoord.x)/1080.0, float(gl_FragCoord.y)/2400.0, 0.0, 1.0);
   // outColor = vec4(1.0, 0.0, 0.0, 1.0);
   outColor = texture(srcTex, v_texCoor);
}