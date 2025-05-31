#version 320 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

void main()
{
   v_color = a_color;
   v_texCoor = a_texCoor;
   v_normal = mat3(transpose(inverse(u_modelMat4))) * a_normal;
   v_worldPos = vec3(u_modelMat4 * a_position);
   gl_Position = u_projectionMat4 * u_viewMat4 * u_modelMat4 * a_position;
}