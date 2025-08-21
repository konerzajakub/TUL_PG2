#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uP_m = mat4(1.0f);
uniform mat4 uM_m = mat4(1.0f);
uniform mat4 uV_m = mat4(1.0f);

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

void main()
{
    vs_out.FragPos = vec3(uM_m * vec4(aPos, 1.0));
    vs_out.Normal = mat3(transpose(inverse(uM_m))) * aNormal;
    vs_out.TexCoords = aTexCoord;
    
    gl_Position = uP_m * uV_m * uM_m * vec4(aPos, 1.0f);
}
