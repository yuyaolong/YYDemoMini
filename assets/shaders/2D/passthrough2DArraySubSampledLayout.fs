/*%%HEADER%%*/
#extension GL_QCOM_texture_foveated_subsampled_layout : require
precision mediump float;
layout(subsampled) mediump uniform sampler2DArray srcTex;
mediump uniform sampler2DArray srcDepthTex;
uniform float showDepth;
in vec2 v_texCoor;
out vec4 outColor;
void main()
{
  if (v_texCoor.x < 0.5) {
      outColor = texture(srcTex, vec3(v_texCoor.x * 2.0, v_texCoor.y, 0.0));
      if (showDepth > 0.0) {
          float dValue = texture(srcDepthTex, vec3(v_texCoor.x * 2.0, v_texCoor.y, 0.0)).r;
          outColor = vec4(vec3(dValue), 1.0);
      }
  } else {
      outColor = texture(srcTex, vec3((v_texCoor.x - 0.5) * 2.0, v_texCoor.y, 1.0));
      // float dValue = texture(srcDepthTex, vec3((v_texCoor.x - 0.5) * 2.0, v_texCoor.y, 1.0)).r;
      // outColor = vec4(vec3(dValue), 1.0);
  }
}