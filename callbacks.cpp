#include "app.h"
#include "callbacks.h"
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <nlohmann/json.hpp>

// zm�na velikosti okna
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->update_projection_matrix();
}

// stisk tla��tek
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if ((action == GLFW_PRESS) || (action == GLFW_REPEAT))
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE); // zav��t okno
            break;
        case GLFW_KEY_F10: {
            app->toggle_fullscreen(); // zapnut�/vypnut� fullscreenu  
            break;
        }
        case GLFW_KEY_F11: { // p�ep�n�n� AA
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
            app->toggle_vsync(); // zapnut�/vypunt� vsyncu
            break;
        }

        // zm�na barev modelu
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

// pohyb my�i
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    // inicializace, prvn� pohyb
    if (app->first_mouse)
    {
        app->last_x = static_cast<float>(xpos);
        app->last_y = static_cast<float>(ypos);
        app->first_mouse = false;
    }

    // v�po�et rozd�lu oproti p�edchoz� pozici
    float xoffset = static_cast<float>(xpos) - app->last_x;
    float yoffset = app->last_y - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

    // ulo�en� aktu�ln�� pozice pro p�i�t� vol�n�
    app->last_x = static_cast<float>(xpos);
    app->last_y = static_cast<float>(ypos);

    // p�ed�n� informac� o pohybu kamery
    app->camera.ProcessMouseMovement(xoffset, yoffset);
}

// kole�ko my�i
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->camera.ProcessMouseScroll(static_cast<float>(yoffset)); // p�ed�n� posunu kole�ka kame�e
}

void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}