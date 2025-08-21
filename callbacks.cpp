#include "app.h"
#include "callbacks.h"
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <nlohmann/json.hpp>

// zmìna velikosti okna
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->update_projection_matrix();
}

// stisk tlaèítek
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if ((action == GLFW_PRESS) || (action == GLFW_REPEAT))
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE); // zavøít okno
            break;
        case GLFW_KEY_F10: {
            app->toggle_fullscreen(); // zapnutí/vypnutí fullscreenu  
            break;
        }
        case GLFW_KEY_F11: { // pøepínání AA
            std::ifstream f_in("app_settings.json");
            nlohmann::json data;
            if (f_in.is_open()) {
                data = nlohmann::json::parse(f_in);
                f_in.close();
            }

            bool current_enabled = data.value("antialiasing", nlohmann::json::object()).value("enabled", false);
            int current_level = data.value("antialiasing", nlohmann::json::object()).value("level", 0);

            if (!current_enabled) {
                data["antialiasing"]["enabled"] = true;
                data["antialiasing"]["level"] = 2;
                std::cout << "Antialiasing set to 2x. Restart to apply." << std::endl;
            } else if (current_level == 2) {
                data["antialiasing"]["level"] = 4;
                std::cout << "Antialiasing set to 4x. Restart to apply." << std::endl;
            } else if (current_level == 4) {
                data["antialiasing"]["level"] = 8;
                std::cout << "Antialiasing set to 8x. Restart to apply." << std::endl;
            } else {
                data["antialiasing"]["enabled"] = false;
                data["antialiasing"]["level"] = 0;
                std::cout << "Antialiasing disabled. Restart to apply." << std::endl;
            }

            std::ofstream f_out("app_settings.json");
            f_out << data.dump(2);
            f_out.close();
            break;
        }
        case GLFW_KEY_F12: {
            app->toggle_vsync(); // zapnutí/vypuntí vsyncu
            break;
        }

        // zmìna barev modelu
        case GLFW_KEY_R:
            app->model_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            break;
        case GLFW_KEY_G:
            app->model_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
            break;
        case GLFW_KEY_B:
            app->model_color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            break;
        
        default:
            break;
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // empty
}

// pohyb myši
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    // inicializace, první pohyb
    if (app->first_mouse)
    {
        app->last_x = static_cast<float>(xpos);
        app->last_y = static_cast<float>(ypos);
        app->first_mouse = false;
    }

    // výpoèet rozdílu oproti pøedchozí pozici
    float xoffset = static_cast<float>(xpos) - app->last_x;
    float yoffset = app->last_y - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

    // uložení aktuálníá pozice pro pøiští volání
    app->last_x = static_cast<float>(xpos);
    app->last_y = static_cast<float>(ypos);

    // pøedání informací o pohybu kamery
    app->camera.ProcessMouseMovement(xoffset, yoffset);
}

// koleèko myši
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->camera.ProcessMouseScroll(static_cast<float>(yoffset)); // pøedání posunu koleèka kameøe
}

void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}