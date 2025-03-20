
#include <vector>
#include <GL/glew.h>
#include <stdexcept>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nlohmann/json.hpp>
#include "assets.hpp"
#include <fstream>
#include "Mesh.h"
#include "OBJloader.hpp"
#include "App.cpp"
#include "ShaderProgram.hpp"


int main() {
    try {
        App app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}