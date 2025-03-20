#version 460 core
in vec3 attribute_Position;

uniform mat4 uP_m = mat4(1.0);
uniform mat4 uM_m = mat4(1.0);
uniform mat4 uV_m = mat4(1.0);
void main() {
   gl_Position = uP_m * uV_m * uM_m * vec4(attribute_Position, 1.0f);
} 