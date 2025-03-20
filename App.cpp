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
#include "ShaderProgram.hpp"

struct Config {
    int windowWidth = 800;
    int windowHeight = 600;
};
class App {
public:
    App() {
        loadConfig("app_settings.json");
        initGLFW();
        initGLEW();
        printContextInfo();
        setupCallbacks();
        initAssets();
    }

    ~App() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void run() {
        double lastTime = glfwGetTime();
        int frameCount = 0;

        shaderProgram.activate();
        GLint uniformColorLoc = glGetUniformLocation(shaderProgram.getID(), "uniform_Color");

        while (!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();
            double deltaTime = currentTime - lastTime;
            frameCount++;

            if (deltaTime >= 1.0) {
                float fps = static_cast<float>(frameCount / deltaTime);
                std::string title = "OpenGL 4.1 Core Profile | FPS: " + std::to_string(fps);
                glfwSetWindowTitle(window, title.c_str());
                frameCount = 0;
                lastTime = currentTime;
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUniform4fv(uniformColorLoc, 1, glm::value_ptr(clearColor));
            mesh.draw(glm::mat4(1.0f));

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

private:
    GLFWwindow* window = nullptr;
    ShaderProgram shaderProgram;
    Mesh mesh;
    Config config;
    bool vsyncEnabled = true;
    glm::vec4 clearColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    void loadConfig(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file) return;

            nlohmann::json j;
            file >> j;

            if (j.contains("window")) {
                auto& w = j["window"];
                if (w.contains("width")) config.windowWidth = w["width"];
                if (w.contains("height")) config.windowHeight = w["height"];
            }
        }
        catch (...) {}
    }

    void initGLFW() {
        glfwSetErrorCallback([](int error, const char* desc) {
            std::cerr << "GLFW Error " << error << ": " << desc << std::endl;
            });

        if (!glfwInit()) throw std::runtime_error("GLFW init failed");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        window = glfwCreateWindow(config.windowWidth, config.windowHeight,
            "OpenGL App", nullptr, nullptr);
        if (!window) throw std::runtime_error("Window creation failed");

        glfwSetWindowUserPointer(window, this);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(vsyncEnabled ? 1 : 0);
    }

    void initGLEW() {
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
            throw std::runtime_error("GLEW init failed");
        if (!GLEW_ARB_direct_state_access)
            throw std::runtime_error("DSA not supported");
    }

    void initAssets() {
        shaderProgram = ShaderProgram("shaders/vertex_shader.vert",
            "shaders/fragment_shader.frag");

        std::vector<Vertex> vertices;
        std::vector<glm::vec3> positions, normals;
        std::vector<glm::vec2> uvs;

        if (!loadOBJ("resources/triangle.obj", positions, uvs, normals))
            throw std::runtime_error("OBJ load failed");

        for (size_t i = 0; i < positions.size(); ++i) {
            vertices.push_back({
                positions[i],
                normals[i],
                uvs[i]
                });
        }

        std::vector<GLuint> indices(vertices.size());
        for (GLuint i = 0; i < indices.size(); ++i) indices[i] = i;

        mesh = Mesh(GL_TRIANGLES, shaderProgram, vertices, indices,
            glm::vec3(0.0f), glm::vec3(0.0f));
    }

    void setupCallbacks() {
        glfwSetKeyCallback(window, [](GLFWwindow* w, int k, int s, int a, int m) {
            static_cast<App*>(glfwGetWindowUserPointer(w))->handleKeyInput(k, a);
            });

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
            glViewport(0, 0, width, height);
            });
    }

    void handleKeyInput(int key, int action) {
        if (action != GLFW_PRESS) return;

        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_F10:
            vsyncEnabled = !vsyncEnabled;
            glfwSwapInterval(vsyncEnabled ? 1 : 0);
            break;
        case GLFW_KEY_R:
            clearColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            break;
        case GLFW_KEY_G:
            clearColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
            break;
        case GLFW_KEY_B:
            clearColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            break;
        }
    }

    void printContextInfo() const {
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n"
            << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n"
            << "Vendor: " << glGetString(GL_VENDOR) << "\n"
            << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    }
};