
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



bool vsyncEnabled = true;

GLuint shader_prog_ID{ 0 };
GLuint VBO_ID{ 0 };
GLuint VAO_ID{ 0 };
GLfloat r{ 1.0f }, g{ 0.0f }, b{ 0.0f }, a{ 1.0f };
std::vector<vertex> triangle_vertices = {
    {{0.0f,  0.5f,  0.0f}},
    {{0.5f, -0.5f,  0.0f}},
    {{-0.5f, -0.5f,  0.0f}}
};


void init_assets() {
    const char* vertex_shader =
        "#version 460 core\n"
        "in vec3 attribute_Position;"
        "void main() {"
        "  gl_Position = vec4(attribute_Position, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 460 core\n"
        "uniform vec4 uniform_Color;"
        "out vec4 FragColor;"
        "void main() {"
        "  FragColor = uniform_Color;"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    shader_prog_ID = glCreateProgram();
    glAttachShader(shader_prog_ID, fs);
    glAttachShader(shader_prog_ID, vs);
    glLinkProgram(shader_prog_ID);

    //now we can delete shader parts (they can be reused, if you have more shaders)
    //the final shader program already linked and stored separately
    glDetachShader(shader_prog_ID, fs);
    glDetachShader(shader_prog_ID, vs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // 
    // Create and load data into GPU using OpenGL DSA (Direct State Access)
    //

    // Create VAO + data description (just envelope, or container...)
    glCreateVertexArrays(1, &VAO_ID);

    GLint position_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_Position");

    glEnableVertexArrayAttrib(VAO_ID, position_attrib_location);
    glVertexArrayAttribFormat(VAO_ID, position_attrib_location, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, position));
    glVertexArrayAttribBinding(VAO_ID, position_attrib_location, 0); // (GLuint vaobj, GLuint attribindex, GLuint bindingindex)

    // Create and fill data
    glCreateBuffers(1, &VBO_ID);
    glNamedBufferData(VBO_ID, triangle_vertices.size() * sizeof(vertex), triangle_vertices.data(), GL_STATIC_DRAW);

    // Connect together
    glVertexArrayVertexBuffer(VAO_ID, 0, VBO_ID, 0, sizeof(vertex));
}
bool init() {
    if (!GLEW_ARB_direct_state_access)
        throw std::runtime_error("No DSA :-(");

    init_assets();
}

void run(GLFWwindow* window) {
    //GLfloat r, g, b, a;
    //r = g = b = a = 1.0f;
    double lastTime = glfwGetTime();
    int frameCount = 0;
    float fps = 0.0f;

    // Activate shader program. There is only one program, so activation can be out of the loop. 
    // In more realistic scenarios, you will activate different shaders for different 3D objects.
    glUseProgram(shader_prog_ID);

    // Get uniform location in GPU program. This will not change, so it can be moved out of the game loop.
    GLint uniform_color_location = glGetUniformLocation(shader_prog_ID, "uniform_Color");
    if (uniform_color_location == -1) {
        std::cerr << "Uniform location is not found in active shader program. Did you forget to activate it?\n";
    }

    while (!glfwWindowShouldClose(window)) {
        // clear canvas
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        frameCount++;
        if (deltaTime >= 1) {
            fps = static_cast<float>(frameCount / deltaTime);
            std::string title = "OpenGL 4.1 Core Profile | FPS: " + std::to_string(fps);
            glfwSetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastTime = currentTime;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniform4f(uniform_color_location, r, g, b, a);
        glBindVertexArray(VAO_ID);
        glDrawArrays(GL_TRIANGLES, 0, triangle_vertices.size());
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}

// Error callback
void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Key callback
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    else if (key == GLFW_KEY_F10 && action == GLFW_PRESS) {
        // Přepnutí stavu VSync
        vsyncEnabled = !vsyncEnabled;
        glfwSwapInterval(vsyncEnabled ? 1 : 0);
    }

    // Přidání reakce na klávesy R, G, B
    else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        // Nastaví červenou složku na 1.0 a ostatní složky na 0.0
        ::r = 1.0f;
        ::g = 0.0f;
        ::b = 0.0f;
    }
    else if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        // Nastaví zelenou složku na 1.0 a ostatní složky na 0.0
        ::r = 0.0f;
        ::g = 1.0f;
        ::b = 0.0f;
    }
    else if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        // Nastaví modrou složku na 1.0 a ostatní složky na 0.0
        ::r = 0.0f;
        ::g = 0.0f;
        ::b = 1.0f;
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

int main() {
    Config config = loadConfig("app_settings.json");

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

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, fbsize_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);


    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }
    
    if (init()) {
        run(window);
    }
    else {
        std::cout << "run failed";
    }
}