
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


#include "app.hpp"

bool vsyncEnabled = true;

// Error callback
void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Key callback
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    else if (key == GLFW_KEY_F12 && action == GLFW_PRESS) {
        // Přepnutí stavu VSync
        vsyncEnabled = !vsyncEnabled;
        glfwSwapInterval(vsyncEnabled ? 1 : 0);
    }
}

// Other callbacks
// Callback při změně velikosti okna
void fbsize_callback(GLFWwindow* window, int width, int height) {
    //glViewport(0, 0, width, height); // Přizpůsobení viewportu
    //std::cout << "Framebuffer Resized: " << width << "x" << height << std::endl;
}

// Callback pro stisk tlačítka myši
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    //std::cout << "Mouse Button: " << button << " Action: " << action << " Mods: " << mods << std::endl;
}

// Callback pro změnu pozice kurzoru
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    //std::cout << "Cursor Position: (" << xpos << ", " << ypos << ")" << std::endl;
}

// Callback pro kolečko myši (scrollování)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    //std::cout << "Scroll Offset: (" << xoffset << ", " << yoffset << ")" << std::endl;
}

struct Config {
    int windowWidth = 800;
    int windowHeight = 600;
};

Config loadConfig(const std::string& filename) {
    Config config;
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Nelze otevřít konfigurační soubor: " << filename << std::endl;
            return config; // Vrátí výchozí hodnoty
        }

        nlohmann::json j;
        file >> j;

        if (j.contains("window")) {
            auto& window = j["window"];
            if (window.contains("width")) config.windowWidth = window["width"];
            if (window.contains("height")) config.windowHeight = window["height"];
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Chyba při načítání konfiguračního souboru: " << e.what() << std::endl;
    }
    return config;
}

void printExtensions() {
    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    std::cout << "\n=== Supported OpenGL Extensions ===\n";
    for (GLint i = 0; i < numExtensions; i++) {
        const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
        std::cout << extension << std::endl;
    }
}


void printContextInfo() {
    // After initializing GLEW
    std::cout << "=== OpenGL Context Information ===\n";
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << '\n';
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << '\n';
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';

    // Verify OpenGL version
    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < 4 || (major == 4 && minor < 1)) {
        throw std::runtime_error("OpenGL 4.6 not supported");
    }

    // Verify profile
    int profile;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
    if (!(profile & GL_CONTEXT_CORE_PROFILE_BIT)) {
        throw std::runtime_error("Core profile not active");
    }

    // Print GLFW version
    int glfwMajor, glfwMinor, glfwRevision;
    glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
    std::cout << "\n=== GLFW Information ===\n";
    std::cout << "GLFW Version: " << glfwMajor << "." << glfwMinor << "." << glfwRevision << '\n';

    // Print GLM version
    std::cout << "\n=== GLM Information ===\n";
    std::cout << "GLM Version: " << GLM_VERSION_MAJOR << "." << GLM_VERSION_MINOR << "." << GLM_VERSION_PATCH << '\n';

    // Print OpenGL extensions
    int numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    std::cout << "\n=== OpenGL Extensions ===\n";
    std::cout << "Number of supported extensions: " << numExtensions << '\n';

    bool debugSupported = false;
    for (int i = 0; i < numExtensions; i++) {
        const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        if (strcmp(ext, "GL_KHR_debug") == 0) {
            debugSupported = true;
            break;
        }
    }
    std::cout << "GL_KHR_debug: " << (debugSupported ? "Supported" : "Not supported") << std::endl;

    if (glewIsSupported("GL_ARB_debug_output")) {
        std::cout << "GL_ARB_debug_output is supported!" << std::endl;
    }
    else {
        std::cout << "GL_ARB_debug_output is NOT supported!" << std::endl;
    }

    printExtensions();
}

void calculateFPS(double lastTime, int frameCount, GLFWwindow* window) {
    // not working ATM
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;
    frameCount++;
    if (deltaTime >= 1) {
        int fps = static_cast<float>(frameCount / deltaTime);
        std::string title = "OpenGL 4.1 Core Profile | FPS: " + std::to_string(fps);
        glfwSetWindowTitle(window, title.c_str());
        frameCount = 0;
        lastTime = currentTime;
    }
}

int main() {
    Config config = loadConfig("config.json");

    // Set error callback
    glfwSetErrorCallback(error_callback);

    //// 1. open OpenGL context, version 4.6, CORE profile, https://www.glfw.org/docs/latest/quick.html#quick_create_window
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }



    // Window creation
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // IMPORTANT: macOS supports only up to OpenGL 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // IMPORTANT: macOS supports only up to OpenGL 4.1
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // CORE profile
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // IMPORTANT: compability tag, required for macOS
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // Debug context
    GLFWwindow* window = glfwCreateWindow(config.windowWidth, config.windowHeight, "OpenGL 4.1 Core Profile", nullptr, nullptr);
    glfwSetKeyCallback(window, key_callback); // Set key callback, set per-window

    // Check if it succeeded
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // Registrace callbacků
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, fbsize_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);


    // Make context current
    glfwMakeContextCurrent(window);

    // GLEW initialization
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    // Vyčistíme případnou spurious chybu, která vznikla při inicializaci
    //glGetError();
    if (err != GLEW_OK) {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // Print context informationope 
    printContextInfo();

    // FPS variables
    double lastTime = glfwGetTime();
    int frameCount = 0;
    float fps = 0.0f;

    // main loop
    
    App app;
    if (app.init()) {
        app.run(window);
    }
    else {
        std::cout << "run failed";
    }
}