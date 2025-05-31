/*%%HEADER%%*/
precision highp float;
precision highp sampler2DArray;
uniform sampler2DArray srcTex;
in vec2 v_texCoor;
out vec4 outColor;
void main()
{
   float newU = 0.0;
   float newV = 0.0;
   if (v_texCoor.x < 0.5) {
         newU = v_texCoor.x * 2.0;
         newV = v_texCoor.y;
         outColor = texture(srcTex, vec3(newU, newV, 0.0));
   }
   else {
         newU = (v_texCoor.x-0.5) * 2.0;
         newV = v_texCoor.y;
         outColor = texture(srcTex, vec3(newU, newV, 1.0));
  }
}