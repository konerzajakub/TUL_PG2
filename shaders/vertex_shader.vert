#version 460 core
layout(location = 0) in vec3 attribute_Position;

void main() {
    gl_Position = vec4(attribute_Position, 1.0);
} 