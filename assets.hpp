#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

// struktura pro vertex
struct vertex {
    glm::vec3 position;     // 3D vector - pozice
    glm::vec3 normal;       // 3D vector - kolmý k ploše
    glm::vec2 texCoords;    // 2D vector - mapování vertexu na texturu (UV souøadnice?)
};