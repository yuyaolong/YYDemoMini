#version 320 es
precision highp float;
#extension GL_EXT_fragment_shading_rate : require
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (binding = 0) uniform sampler2D srcTexArray;
layout (binding = 1) uniform UniformBufferObject {
    vec4 offsetUV;
}ubo;
layout (location = 0) in vec2 v_texCoor;
layout (location = 0) out vec4 uFragColor;

float uvCal(in float x, in float ofY) {
   float res = 0.0f;
   float ofX = ofY / 2.8;
   float a = x - ofX;
   float R0 = 0.0536;
   float R1 = 0.1429;
   float R2 = 0.8571;
   float R3 = 0.9464;
   if(a < R0) { res =  2.8 * a;; }
   else if (a < R1) { res = 0.15 + (-0.3571 + sqrt(0.3571*0.3571 - 4.0 * 5.3571 * (0.0536-a))) / (2.0 * 5.35714); }
   else if (a < R2) { res = 0.7 * (a - 0.1429) + 0.25; }
   else if (a < R3) { res = 0.85 - (-0.3571 + sqrt(0.3571*0.3571 - 4.0 * 5.3571 * (a-0.9464))) / (2.0 * 5.35714); }
   else { res = 1.0 - 2.8 * (1.0-a); }
   return clamp(res + ofY, 0.0, 1.0);
}

float getNewPos(in float r_pose, in float offset){
  float factor = 32.0/2200.0;
  float ind_lo= floor(r_pose / factor);
  float ind_hi =ind_lo +1.0;
  float lo_pos =ind_lo*factor;
  float hi_pos =ind_hi*factor;
  float w_mix =(r_pose-lo_pos)/factor;
  return uvCal(lo_pos, offset)*(1.0-w_mix)+uvCal(hi_pos, offset)*w_mix;
 }

void main() {
    float newU = 0.0;
       float newV = 0.0;
       //vec4 color = texture( srcTex, v_texCoor);
       if (v_texCoor.x < 0.5) { // left
          newU = getNewPos(v_texCoor.x * 2.0, ubo.offsetUV.x);
          newV = getNewPos(v_texCoor.y, ubo.offsetUV.y);
          uFragColor = texture( srcTexArray, vec2(newU, newV) ); }
       else { // right
          newU = getNewPos(abs(v_texCoor.x-0.5) * 2.0, ubo.offsetUV.z);
          newV = getNewPos(v_texCoor.y, ubo.offsetUV.w);
          uFragColor = texture( srcTexArray, vec2(newU, newV) ); }

    if (gl_ShadingRateEXT == 0) {
        uFragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }else if (gl_ShadingRateEXT == 1) {
        uFragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else if (gl_ShadingRateEXT == 4) {
        uFragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else if (gl_ShadingRateEXT == 5) {
        uFragColor = vec4(0.0, 0.0, 1.0, 1.0);
    } else if (gl_ShadingRateEXT == 9) {
        uFragColor = vec4(1.0, 1.0, 0.0, 1.0);
    } else if (gl_ShadingRateEXT == 10) {
        uFragColor = vec4(1.0, 0.0, 1.0, 1.0);
    } else {
        uFragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
