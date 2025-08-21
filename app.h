#pragma once

#include <vector>
#include <filesystem>
#include <random>
#include <algorithm>

#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>

#include "src/ShaderProgram.hpp"
#include "src/Model.hpp"
#include "src/camera.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GLFW/glfw3.h>

#define NOMINMAX
#include "src/miniaudio/miniaudio.h"

struct GLFWwindow;

class App {
public:
    App();
    ~App();
    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

    void toggle_fullscreen();
    void toggle_vsync();
    void run();
    void update_projection_matrix();
    GLuint textureInit(const std::filesystem::path& file_name);
    GLuint gen_tex(cv::Mat& image);

    GLFWwindow* window;
    Camera camera;
    float last_x = 0.0f, last_y = 0.0f;
    bool first_mouse = true;
    glm::vec4 model_color = glm::vec4(1.0f);

    GLFWmonitor* primary_monitor = nullptr;

    cv::Mat maze_map;
    cv::Mat hmap;
    std::vector<GLuint> wall_textures;
    Model height_map_model;
    cv::Rect flatten_area;
    uchar flatten_height = 100;

private:
    void load_settings();
    void save_settings();
    float getTerrainHeight(float x, float z);
    glm::vec3 getTerrainNormal(float x, float z);

    struct WindowSettings {
        int width = 1280;
        int height = 720;
        int pos_x = 100;
        int pos_y = 100;
        bool fullscreen = false;
        bool vsync = true;
        std::string title = "PG2 Test v1";
    } window_settings_;

    struct AntialiasingSettings {
        bool enabled = false;
        int level = 0;
    } antialiasing_settings_;

    void process_input(float delta_time);
    void genLabyrinth(cv::Mat& map);
    void carve_passages(int cx, int cy, cv::Mat& map, std::default_random_engine& rng);
    uchar getmap(cv::Mat& map, int x, int y);
    Model GenHeightMap(cv::Mat& hmap, const unsigned int mesh_step_size, const cv::Rect& flatten_area, uchar flatten_height);
    glm::vec2 get_subtex_by_height(float height);
    glm::vec2 get_subtex_st(const int x, const int y);

    ShaderProgram shader;
    std::map<std::string, Model> scene;
    glm::mat4 projection_matrix;
    glm::mat4 view_matrix;

    // lighting
    ShaderProgram lighting_shader;
    ShaderProgram lamp_shader;
    ShaderProgram transparent_shader;
    Model light_cube_model;

    struct Material {
		float shininess;
	};

    struct DirLight {
        glm::vec3 direction;

        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };

    struct PointLight {
        glm::vec3 position;

        float constant;
        float linear;
        float quadratic;

        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };

    const static int NR_POINT_LIGHTS = 7;

    DirLight dir_light;
    Material material;
    std::vector<PointLight> point_lights;

    struct SpotLight {
        glm::vec3 position;
        glm::vec3 direction;
        float cutOff;
        float outerCutOff;

        float constant;
        float linear;
        float quadratic;

        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };

    SpotLight spot_light;

public:
    glm::vec3 camera_velocity = glm::vec3(0.0f);
    bool camera_on_ground = false;

    ma_engine engine;
    ma_sound jump_sound;
};