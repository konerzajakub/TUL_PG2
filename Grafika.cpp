
#include <vector>
#include <GL/glew.h>
#include <stdexcept>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "assets.hpp"


#include "app.hpp"

int main() {
    App app;
    if (app.init()) {
        app.run();
    }
    else {
        std::cout << "run failed";
    }
}