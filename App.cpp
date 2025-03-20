#include <vector>
#include <GL/glew.h>
#include <stdexcept>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <nlohmann/json.hpp>
#include "assets.hpp"
#include <fstream>
#include "Mesh.h"
#include "OBJloader.hpp"
#include "camera.hpp"
#include "ShaderProgram.hpp"

class App {
public:
    App() {
        loadConfig("app_settings.json");
        initGLFW();
        initGLEW();
        printContextInfo();
        setupCallbacks();
        initAssets();
        updateProjectionMatrix(); // Initialize projection matrix
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
        GLint projMatrixLoc = glGetUniformLocation(shaderProgram.getID(), "projectionMatrix");
        GLint viewMatrixLoc = glGetUniformLocation(shaderProgram.getID(), "viewMatrix");



        while (!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();
            double deltaTime = currentTime - lastTime;
            frameCount++;

            if (deltaTime >= 1.0) {
                float fps = static_cast<float>(frameCount / deltaTime);
                std::string title = "FPS: " + std::to_string(fps);
                glfwSetWindowTitle(window, title.c_str());
                frameCount = 0;
                lastTime = currentTime;
            }
            shaderProgram.activate();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::vec3 movement = camera.ProcessInput(window, deltaTime);
            camera.Position += movement;
            //std::cout << movement;

            glUniform4fv(uniformColorLoc, 1, glm::value_ptr(clearColor));
            glm::mat4 viewMatrix = camera.GetViewMatrix();

            // Update matrices
            glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
            glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
            shaderProgram.setUniform("uP_m", projectionMatrix);
            shaderProgram.setUniform("uV_m", viewMatrix);
            mesh.draw(glm::mat4(1.0f));

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

protected:
    glm::mat4 projectionMatrix;
    float fovY = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    void updateProjectionMatrix() {
        float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
        projectionMatrix = glm::perspective(glm::radians(fovY), aspectRatio, nearPlane, farPlane);
    }

private:
    int windowWidth = 800;
    int windowHeight = 600;
    GLFWwindow* window = nullptr;
    ShaderProgram shaderProgram;
    Mesh mesh;
    bool vsyncEnabled = true;
    Camera camera{ glm::vec3(0.0f, 0.0f, 1.0f) };
    double cursorLastX = 0.0;
    double cursorLastY = 0.0;
    glm::vec4 clearColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    void loadConfig(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file) return;

            nlohmann::json j;
            file >> j;

            if (j.contains("window")) {
                auto& w = j["window"];
                if (w.contains("width")) windowWidth = w["width"];
                if (w.contains("height")) windowHeight = w["height"];
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

        window = glfwCreateWindow(windowWidth, windowHeight,
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
            static_cast<App*>(glfwGetWindowUserPointer(w))->framebufferSizeCallback(width, height);
            });
        glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
            static_cast<App*>(glfwGetWindowUserPointer(w))->mouseButtonCallback(button, action, mods);
            });
        glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
            static_cast<App*>(glfwGetWindowUserPointer(w))->cursor_position_callback(w, xpos, ypos);
            });
        glfwSetScrollCallback(window, [](GLFWwindow* w, double xoffset, double yoffset) {
            static_cast<App*>(glfwGetWindowUserPointer(w))->scrollCallback(xoffset, yoffset);
            });
    }

    void framebufferSizeCallback(int width, int height) {
        windowWidth = width;
        windowHeight = height;
        glViewport(0, 0, width, height);
        updateProjectionMatrix();
    }

    void mouseButtonCallback(int button, int action, int mods) {
        // Instance-specific mouse button handling.
    }

    void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
    {
        double xoffset = xpos - cursorLastX;
        double yoffset = ypos - cursorLastY;
        cursorLastX = xpos;
        cursorLastY = ypos;
        camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
    }

    void scrollCallback(double xoffset, double yoffset) {
        fovY -= static_cast<float>(yoffset) * 2.0f;
        fovY = glm::clamp(fovY, 1.0f, 120.0f);
        updateProjectionMatrix();
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