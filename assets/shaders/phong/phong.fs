#version 320 es
precision mediump float;
in vec4 v_color;
in vec2 v_texCoor;
in vec3 v_normal;
in vec3 v_worldPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D texture1;
uniform uint useHDR;
uniform uint vrsLevel;
uniform float exposure;
out vec4 outColor;
void main()
{
  const float gamma = 2.2;
  // ambient
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * lightColor;
  // diffuse
  vec3 norm = normalize(v_normal);
  vec3 lightDir = normalize(lightPos - v_worldPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;
  // specular
  float specularStrength = 0.8;
  vec3 viewDir = normalize(viewPos - v_worldPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
  vec3 specular = specularStrength * spec * lightColor;
  int dummy_cnt = 850;
  while(dummy_cnt > 0) {dummy_cnt = dummy_cnt - 1; specular = specular * 0.99;}
  // final result
  vec3 phongShading = (ambient + diffuse + specular) * vec3(texture(texture1, v_texCoor));
  vec3 mapped;
  if (useHDR == 0u) {
    // No HDR
    mapped = phongShading;}
  else {
  // Exposure HDR
    mapped = vec3(1.0) - exp(-phongShading * exposure);}"
  // Gamma correction
  vec3 result = pow(mapped, vec3(1.0 / gamma));
  if (vrsLevel == 2u) {result.x *= 1.2;}
  if (vrsLevel == 4u) {result.y *= 1.2;}
  if (vrsLevel == 8u) {result.z *= 1.2;}
  outColor = vec4(result, 1.0f);
}