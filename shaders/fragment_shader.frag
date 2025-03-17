#version 460 core
uniform vec4 uniform_Color;
out vec4 FragColor;

void main() {
    FragColor = uniform_Color;
}