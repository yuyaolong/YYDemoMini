/*%%HEADER%%*/
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 uv_position;
out vec2 v_texCoor;
void main()
{
   gl_Position = vec4(a_position, 0.0, 1.0);
   v_texCoor = vec2(uv_position.x, uv_position.y);
}