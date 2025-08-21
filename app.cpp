#include "app.h"

#include <iostream> 
#include <fstream> 
#include <vector>
#include <string> 
#include <algorithm> 
#include <map>

#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp> 
#include <nlohmann/json.hpp>

#include "callbacks.h"

// --- Načítání nastavení z JSON souboru ---
// Nastavení okna a antialiasingu ze souboru "app_settings.json"
void App::load_settings() {
    // Otevření souboru
    std::ifstream f("app_settings.json");
    if (!f.is_open()) {
        // Chybová hláška
        std::cerr << "Nelze otevrit app_settings.json" << std::endl;
        return;
    }
    // Parsování dat JSON ze souboru
    nlohmann::json data = nlohmann::json::parse(f);

    // Kontrola existence objektu "window"
    if (data.contains("window")) {
        // Načtení nastavení okna (pokud existují)
        if (data["window"].contains("width")) {
            window_settings_.width = data["window"]["width"];
        }
        if (data["window"].contains("height")) {
            window_settings_.height = data["window"]["height"];
        }
        if (data["window"].contains("title")) {
            window_settings_.title = data["window"]["title"];
        }
        if (data["window"].contains("fullscreen")) {
            window_settings_.fullscreen = data["window"]["fullscreen"];
        }
        if (data["window"].contains("vsync")) {
            window_settings_.vsync = data["window"]["vsync"];
        }
        if (data["window"].contains("pos_x")) {
            window_settings_.pos_x = data["window"]["pos_x"];
        }
        if (data["window"].contains("pos_y")) {
            window_settings_.pos_y = data["window"]["pos_y"];
        }
    }

    // Kontrola existence objektu "antialiasing"
    if (data.contains("antialiasing")) {
        // Načtení nastavení antialiasingu (pokud existuje)
        if (data["antialiasing"].contains("enabled")) {
            antialiasing_settings_.enabled = data["antialiasing"]["enabled"];
        }
        if (data["antialiasing"].contains("level")) {
            antialiasing_settings_.level = data["antialiasing"]["level"];
        }
    }

    // Výpis statusu AA
    if (antialiasing_settings_.enabled) {
        std::cout << "Antialiasing je povolen s urovni " << antialiasing_settings_.level << std::endl;
    } else {
        std::cout << "Antialiasing je zakazan." << std::endl;
    }
}

// --- Konstruktor App ---
// Inicializace GLFW, vytváření okna a nastavení OpenGL
App::App() : camera(Camera(glm::vec3(0.0f, 0.0f, 3.0f))) {
    // Načtení nastavení ze souboru JSON
    load_settings();

    // Inicializace GLFW
    if (!glfwInit()) {
        std::cerr << "Selhala inicializace GLFW. Neco se pokazilo. Privedte sem Gatese." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Nastavení error callbacku pro GLFW
    glfwSetErrorCallback(error_callback);

    // Nastavení profilu OpenGL kontextu (4.6)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Povolení MSAA, pokud je povolen antialiasing
    if (antialiasing_settings_.enabled) {
        glfwWindowHint(GLFW_SAMPLES, antialiasing_settings_.level);
    }

    // Získání primárního monitoru
    primary_monitor = glfwGetPrimaryMonitor();

    // Vytvoření okna
    if (window_settings_.fullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
        window = glfwCreateWindow(mode->width, mode->height, window_settings_.title.c_str(), primary_monitor, nullptr);
    } else {
        window = glfwCreateWindow(window_settings_.width, window_settings_.height, window_settings_.title.c_str(), nullptr, nullptr);
        glfwSetWindowPos(window, window_settings_.pos_x, window_settings_.pos_y);
    }
    if (!window) {
        std::cerr << "Selhalo vytvoreni okna GLFW. Privedte sem Gatese." << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Nastavení aktuálního kontextu a ukazatele
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    // Nastavení callbacků pro eventy okna
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Skrytí a zachycení kurzoru
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Nastavení vsync
    glfwSwapInterval(window_settings_.vsync ? 1 : 0);

    // Inicializace GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Selhala inicializace GLEW :(" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // --- Nastavení shaderů a světel ---
    // Načtení a kompilace shaderů
    lighting_shader = ShaderProgram("resources/shaders/phong.vert", "resources/shaders/phong.frag");
    lamp_shader = ShaderProgram("resources/shaders/basic.vert", "resources/shaders/basic.frag");
    transparent_shader = ShaderProgram("resources/shaders/phong.vert", "resources/shaders/transparent.frag");

    // Nastavení směrového světla
    dir_light.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    dir_light.ambient = glm::vec3(0.05f, 0.05f, 0.05f); // Ambient složka
    dir_light.diffuse = glm::vec3(0.4f, 0.4f, 0.4f); // Diffusion složka
    dir_light.specular = glm::vec3(0.5f, 0.5f, 0.5f); // Specular složka

    // Nastavení vlastností materiálu
    material.shininess = 32.0f; // Lesklost

    // --- Bodové světla ---
    PointLight p1, p2, p3, p4, p5, p6, p7;

    // Červené
    p1.position = glm::vec3(30.0f, 102.0f, 45.0f);
    p1.diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
    p1.ambient = p1.diffuse * 0.1f;
    p1.specular = p1.diffuse;
    p1.constant = 1.0f; p1.linear = 0.09f; p1.quadratic = 0.032f; 

    // Oranžové
    p2.position = glm::vec3(35.0f, 102.0f, 45.0f);
    p2.diffuse = glm::vec3(1.0f, 0.5f, 0.0f);
    p2.ambient = p2.diffuse * 0.1f;
    p2.specular = p2.diffuse;
    p2.constant = 1.0f; p2.linear = 0.09f; p2.quadratic = 0.032f;

    // Žluté
    p3.position = glm::vec3(40.0f, 102.0f, 45.0f);
    p3.diffuse = glm::vec3(1.0f, 1.0f, 0.0f);
    p3.ambient = p3.diffuse * 0.1f;
    p3.specular = p3.diffuse;
    p3.constant = 1.0f; p3.linear = 0.09f; p3.quadratic = 0.032f;

    // Zelené
    p4.position = glm::vec3(45.0f, 102.0f, 45.0f);
    p4.diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
    p4.ambient = p4.diffuse * 0.1f;
    p4.specular = p4.diffuse;
    p4.constant = 1.0f; p4.linear = 0.09f; p4.quadratic = 0.032f;

    // Modré
    p5.position = glm::vec3(50.0f, 102.0f, 45.0f);
    p5.diffuse = glm::vec3(0.0f, 0.0f, 1.0f);
    p5.ambient = p5.diffuse * 0.1f;
    p5.specular = p5.diffuse;
    p5.constant = 1.0f; p5.linear = 0.09f; p5.quadratic = 0.032f;

    // Indigové
    p6.position = glm::vec3(55.0f, 102.0f, 45.0f);
    p6.diffuse = glm::vec3(0.29f, 0.0f, 0.51f);
    p6.ambient = p6.diffuse * 0.1f;
    p6.specular = p6.diffuse;
    p6.constant = 1.0f; p6.linear = 0.09f; p6.quadratic = 0.032f;

    // Fialové
    p7.position = glm::vec3(60.0f, 102.0f, 45.0f);
    p7.diffuse = glm::vec3(0.5f, 0.0f, 1.0f);
    p7.ambient = p7.diffuse * 0.1f;
    p7.specular = p7.diffuse;
    p7.constant = 1.0f; p7.linear = 0.09f; p7.quadratic = 0.032f;

    point_lights.push_back(p1);
    point_lights.push_back(p2);
    point_lights.push_back(p3);
    point_lights.push_back(p4);
    point_lights.push_back(p5);
    point_lights.push_back(p6);
    point_lights.push_back(p7);

    // Nastavení kuželového světla (čelovky)
    spot_light.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    spot_light.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    spot_light.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    spot_light.constant = 1.0f;
    spot_light.linear = 0.09f;
    spot_light.quadratic = 0.032f;
    spot_light.cutOff = glm::cos(glm::radians(12.5f)); // Vnitřní úhel kužele
    spot_light.outerCutOff = glm::cos(glm::radians(15.0f)); // Vnější úhel kužele

    // --- Načítání modelů a textur ---
    // Načtení modelu kostky a textury
    Model cube_model("resources/objects/cube_triangles_vnt.obj");
    light_cube_model = cube_model;
    GLuint box_texture = textureInit("resources/textures/box_rgb888.png");
    cube_model.setTexture(box_texture);



    // Vytvoření průhledných kostek
    Model transparent_cube1 = cube_model;
    transparent_cube1.transparent = true;
    transparent_cube1.diffuse_color = glm::vec4(1.0f, 0.0f, 0.0f, 0.2f); // Červená s 20% průhledností
    glm::mat4 model_m = glm::mat4(1.0f);
    model_m = glm::translate(model_m, glm::vec3(40.0f, 101.0f, 40.0f));
    model_m = glm::scale(model_m, glm::vec3(2.0f));
    transparent_cube1.setMatrix(model_m);
    scene["transparent_cube1"] = transparent_cube1;

    Model transparent_cube2 = cube_model;
    transparent_cube2.transparent = true;
    transparent_cube2.diffuse_color = glm::vec4(0.0f, 1.0f, 0.0f, 0.4f); // Zelená s 40% průhledností
    model_m = glm::mat4(1.0f);
    model_m = glm::translate(model_m, glm::vec3(43.0f, 101.0f, 40.0f));
    model_m = glm::scale(model_m, glm::vec3(2.0f));
    transparent_cube2.setMatrix(model_m);
    scene["transparent_cube2"] = transparent_cube2;

    Model transparent_cube3 = cube_model;
    transparent_cube3.transparent = true;
    transparent_cube3.diffuse_color = glm::vec4(0.0f, 0.0f, 1.0f, 0.6f); // Modrá s 60% průhledností
    model_m = glm::mat4(1.0f);
    model_m = glm::translate(model_m, glm::vec3(46.0f, 101.0f, 40.0f));
    model_m = glm::scale(model_m, glm::vec3(2.0f));
    transparent_cube3.setMatrix(model_m);
    scene["transparent_cube3"] = transparent_cube3;

    // Načtení modelu králíčka
    Model bunny_model("resources/objects/bunny_tri_vnt.obj");
    bunny_model.setTexture(box_texture);
    glm::mat4 bunny_matrix = glm::mat4(1.0f);
    bunny_matrix = glm::translate(bunny_matrix, glm::vec3(50.0f, 101.0f, 40.0f));
    bunny_matrix = glm::scale(bunny_matrix, glm::vec3(0.5f));
    bunny_model.setMatrix(bunny_matrix);
    scene["bunny"] = bunny_model;


    // Načtení textur stěn
    wall_textures.clear();
    for (const auto& entry : std::filesystem::directory_iterator("resources/textures")) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename != "heights.png" && filename != "tex_256.png") {
                wall_textures.push_back(textureInit(entry.path()));
            }
        }
    }

    // --- Generování labyrintu a heightmapy ---
    // Generace mapy labyrintu
    maze_map = cv::Mat(10, 25, CV_8U);
    genLabyrinth(maze_map);

    // Definování oblasti pro zplacatění na heightmapě
    flatten_area = cv::Rect(25, 25, 50, 50);

    // Načtení obrázku heightmapy
    hmap = cv::imread("resources/textures/heights.png", cv::IMREAD_GRAYSCALE); // nacteni stupni sedi
    if (hmap.empty()) {
        throw std::runtime_error("Selhalo nacteni heightmapy.");
    }

    // Normalizace výškové mapy (na rozsah 0-255)
    double min, max;
    cv::minMaxLoc(hmap, &min, &max);
    std::cout << "Vyskova mapa min hodnota: " << min << ", max hodnota: " << max << std::endl;
    cv::normalize(hmap, hmap, 0, 255, cv::NORM_MINMAX);

    // Generace modelu vyskove mapy
    Model height_map_model = GenHeightMap(hmap, 1, flatten_area, flatten_height);
    height_map_model.setTexture(textureInit("resources/textures/tex_256.png"));
    scene["height_map"] = height_map_model;

    // Načtení modelu "sten" labyrintu
        Model wall_model("resources/objects/cube_triangles_vnt.obj");
        scene["wall"] = wall_model;

    // --- Ostatní nastavení ---
    // Nastavení počáteční pozice myši
    last_x = static_cast<float>(window_settings_.width) / 2.0f;
    last_y = static_cast<float>(window_settings_.height) / 2.0f;

    // Aktualizace projekční matice
    update_projection_matrix();

    // Povolení OpenGL funkcí
    glEnable(GL_DEPTH_TEST); 
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glDepthFunc(GL_LEQUAL); 

    // Povolení multisamplingu, pokud je povolen antialiasing
    if (antialiasing_settings_.enabled) {
        glEnable(GL_MULTISAMPLE);
    }

    // Inicializace sound enginu
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        printf("Selhala inicializace sound enginu.\n");
    }

    // Načtení zvuku skoku
    if (ma_sound_init_from_file(&engine, "resources/audio/jump_male.wav", 0, NULL, NULL, &jump_sound) != MA_SUCCESS) {
        printf("Selhalo nacteni zvuku skakani.\n");
    }
}

// --- Přepínání fullscreen/window ---
void App::toggle_fullscreen() {
    window_settings_.fullscreen = !window_settings_.fullscreen;

    if (window_settings_.fullscreen) {
        // Uložení pozice a velikosti okna
        glfwGetWindowPos(window, &window_settings_.pos_x, &window_settings_.pos_y);
        glfwGetWindowSize(window, &window_settings_.width, &window_settings_.height);

        // Přepnutí do fulscreen
        const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
        glfwSetWindowMonitor(window, primary_monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        // Přepnutí do window
        glfwSetWindowMonitor(window, nullptr, window_settings_.pos_x, window_settings_.pos_y, window_settings_.width, window_settings_.height, 0);
    }
}

// --- Switchování VSync ---
void App::toggle_vsync() {
    window_settings_.vsync = !window_settings_.vsync;
    glfwSwapInterval(window_settings_.vsync ? 1 : 0);
}

// --- Ukládání nastavení ---
void App::save_settings() {
    // Načtení nastavení v souboru
    std::ifstream f_in("app_settings.json");
    nlohmann::json data;
    if (f_in.is_open()) {
        data = nlohmann::json::parse(f_in);
        f_in.close();
    }

    // Aktualizace nastavení okna
    data["window"]["width"] = window_settings_.width;
    data["window"]["height"] = window_settings_.height;
    data["window"]["fullscreen"] = window_settings_.fullscreen;
    data["window"]["vsync"] = window_settings_.vsync;
    data["window"]["pos_x"] = window_settings_.pos_x;
    data["window"]["pos_y"] = window_settings_.pos_y;

    // Zápis do souboru
    std::ofstream f_out("app_settings.json");
    f_out << data.dump(4);
    f_out.close();
}

// --- Destruktor aplikace ---
App::~App() {
    save_settings();
    ma_engine_uninit(&engine);
    glfwDestroyWindow(window);
    glfwTerminate();
}

// --- Main loop aplikace ---
void App::run() {
    double last_time = glfwGetTime();
    int nb_frames = 0;

    float last_frame = 0.0f;
    glm::vec3 light_pos; 
    glm::vec3 light_pos2;

    while (!glfwWindowShouldClose(window)) {
        // --- FPS counter a aktualizace titlu okna ---
        double current_time = glfwGetTime();
        nb_frames++;
        if (current_time - last_time >= 1.0) {
            std::string title = window_settings_.title + " | FPS: " + std::to_string(nb_frames) + 
                                " | VSync (F12): " + (window_settings_.vsync ? "Zap" : "Vyp") + 
                                " | Fullscreen (F10): " + (window_settings_.fullscreen ? "Zap" : "Vyp") + 
                                " | AA (F11): " + (antialiasing_settings_.enabled ? "Zap (" + std::to_string(antialiasing_settings_.level) + "x)" : "Vyp") + 
                                " | Pozice: (" + std::to_string(camera.Position.x) + ", " + std::to_string(camera.Position.y) + ", " + std::to_string(camera.Position.z) + ")";
            glfwSetWindowTitle(window, title.c_str());
            nb_frames = 0;
            last_time += 1.0;
        }

        // --- Výpočet delta time ---
        float current_frame = static_cast<float>(glfwGetTime());
        float delta_time = current_frame - last_frame;
        last_frame = current_frame;

        // Zpracování inputu od uživatele
        process_input(delta_time);

        // --- Aktualizace fyziky ---
        const float gravity = -20.0f; // -20 se mi líbí nejvíc
        const float player_height = 2.0f; // Výška hráče

        // Gravitace pracuje
        if (!camera_on_ground) {
            camera_velocity.y += gravity * delta_time;
        }

        // Aktualizace pozice podle rychlosti
        camera.Position += camera_velocity * delta_time;

        // Kontrola kolize se zemí
        float terrainHeight = getTerrainHeight(camera.Position.x, camera.Position.z);
        if (camera.Position.y < terrainHeight + player_height) {
            camera.Position.y = terrainHeight + player_height;
            camera_velocity.y = 0;
            camera_on_ground = true;
        } else {
            camera_on_ground = false;
        }

        // --- Vykreslování ---
        // Clearování bufferů.
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Aktualizace projection a view matice.
        update_projection_matrix();
        view_matrix = camera.GetViewMatrix();

        // --- Nastavení uniform ---
        lighting_shader.activate();
        lighting_shader.setUniform("uP_m", projection_matrix);
        lighting_shader.setUniform("uV_m", view_matrix);
        lighting_shader.setUniform("u_view_pos", camera.Position);
        lighting_shader.setUniform("material.diffuse", 0);
        lighting_shader.setUniform("material.shininess", material.shininess);

        // --- Směrového světlo ---
        glm::vec3 scene_center = glm::vec3(flatten_area.x + flatten_area.width / 2.0f, 0.0f, flatten_area.y + flatten_area.height / 2.0f);
        float radius = 150.0f;
        float sun_rotation_speed = 0.5f;
        float light_x = scene_center.x + sin(glfwGetTime() * sun_rotation_speed) * radius;
        float light_z = scene_center.z + cos(glfwGetTime() * sun_rotation_speed) * radius;
        light_pos = glm::vec3(light_x, 301.0f, light_z); 
        dir_light.direction = glm::normalize(scene_center - light_pos);

        float light_x2 = scene_center.x + sin(glfwGetTime() * sun_rotation_speed + 3.14159f) * radius;
        float light_z2 = scene_center.z + cos(glfwGetTime() * sun_rotation_speed + 3.14159f) * radius;
        light_pos2 = glm::vec3(light_x2, 301.0f, light_z2);

        lighting_shader.setUniform("dirLight.direction", dir_light.direction);
        lighting_shader.setUniform("dirLight.ambient", dir_light.ambient);
        lighting_shader.setUniform("dirLight.diffuse", dir_light.diffuse);
        lighting_shader.setUniform("dirLight.specular", dir_light.specular);

        // --- Pohyb bodových světel ---
        float amplitude = 2.0f;
        float speed = 2.0f;
        for (size_t i = 0; i < point_lights.size(); ++i) {
            point_lights[i].position.y = 102.0f + sin(glfwGetTime() * speed + i) * amplitude;
        }

        // --- Nastavení bodových světel ---
        for (int i = 0; i < NR_POINT_LIGHTS; i++)
        {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            lighting_shader.setUniform(base + ".position", point_lights[i].position);
            lighting_shader.setUniform(base + ".ambient", point_lights[i].ambient);
            lighting_shader.setUniform(base + ".diffuse", point_lights[i].diffuse);
            lighting_shader.setUniform(base + ".specular", point_lights[i].specular);
            lighting_shader.setUniform(base + ".constant", point_lights[i].constant);
            lighting_shader.setUniform(base + ".linear", point_lights[i].linear);
            lighting_shader.setUniform(base + ".quadratic", point_lights[i].quadratic);
        }

        // --- Nastavení kuželového světla (čelenky) ---
        spot_light.position = camera.Position; // Pozice kamery
        spot_light.direction = camera.Front; // Kouká stejně jako hráč
        lighting_shader.setUniform("spotLight.position", spot_light.position);
        lighting_shader.setUniform("spotLight.direction", spot_light.direction);
        lighting_shader.setUniform("spotLight.ambient", spot_light.ambient);
        lighting_shader.setUniform("spotLight.diffuse", spot_light.diffuse);
        lighting_shader.setUniform("spotLight.specular", spot_light.specular);
        lighting_shader.setUniform("spotLight.constant", spot_light.constant);
        lighting_shader.setUniform("spotLight.linear", spot_light.linear);
        lighting_shader.setUniform("spotLight.quadratic", spot_light.quadratic);
        lighting_shader.setUniform("spotLight.cutOff", spot_light.cutOff);
        lighting_shader.setUniform("spotLight.outerCutOff", spot_light.outerCutOff);

        // --- Rotace průhledné kostky ---
        glm::mat4& cube1_model_matrix = scene.at("transparent_cube1").model_matrix;
        cube1_model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 101.0f, 40.0f));
        cube1_model_matrix = glm::rotate(cube1_model_matrix, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        cube1_model_matrix = glm::scale(cube1_model_matrix, glm::vec3(2.0f));

        // --- Vykreslování objektů ---
        std::vector<Model*> transparent_models;
        transparent_models.reserve(scene.size());

        // Vykreslení neprůhledných objektů
        glDepthMask(GL_TRUE);
        lighting_shader.activate();
        for (auto& [name, model] : scene) {
            if (!model.transparent) {
                if (name == "height_map") {
                    model.draw(lighting_shader);
                } else if (name == "wall") {
                    if (!wall_textures.empty()) {
                        std::random_device r;
                        std::default_random_engine e1(r());
                        std::uniform_int_distribution<int> uniform_tex(0, static_cast<int>(wall_textures.size()) - 1);

                        for (int y = 0; y < maze_map.rows; ++y) {
                            for (int x = 0; x < maze_map.cols; ++x) {
                                if (getmap(maze_map, x, y) == '#') {
                                    glm::mat4 model_matrix = glm::mat4(1.0f);
                                    model_matrix = glm::translate(model_matrix, glm::vec3(flatten_area.x + x, flatten_height, flatten_area.y + y));
                                    model.setMatrix(model_matrix);
                                    model.setTexture(wall_textures[uniform_tex(e1)]);
                                    model.draw(lighting_shader);
                                }
                            }
                        }
                    }
                }
                else {
                    model.draw(lighting_shader);
                }
            } else {
                transparent_models.push_back(&model);
            }
        }

        // Seřazení průhledných objektů odzadu dopředu
        std::sort(transparent_models.begin(), transparent_models.end(), [&](const Model* a, const Model* b) {
            glm::vec3 a_pos = glm::vec3(a->model_matrix[3]);
            glm::vec3 b_pos = glm::vec3(b->model_matrix[3]);
            return glm::distance(camera.Position, a_pos) > glm::distance(camera.Position, b_pos);
        });

        // Vykreslení průhledných objektů
        glDepthMask(GL_FALSE);
        transparent_shader.activate();
        transparent_shader.setUniform("uP_m", projection_matrix);
        transparent_shader.setUniform("uV_m", view_matrix);
        transparent_shader.setUniform("u_view_pos", camera.Position);
        transparent_shader.setUniform("material.diffuse", 0);
        transparent_shader.setUniform("material.shininess", material.shininess);
        transparent_shader.setUniform("dirLight.direction", dir_light.direction);
        transparent_shader.setUniform("dirLight.ambient", dir_light.ambient);
        transparent_shader.setUniform("dirLight.diffuse", dir_light.diffuse);
        transparent_shader.setUniform("dirLight.specular", dir_light.specular);
        for (int i = 0; i < NR_POINT_LIGHTS; i++)
        {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            transparent_shader.setUniform(base + ".position", point_lights[i].position);
            transparent_shader.setUniform(base + ".ambient", point_lights[i].ambient);
            transparent_shader.setUniform(base + ".diffuse", point_lights[i].diffuse);
            transparent_shader.setUniform(base + ".specular", point_lights[i].specular);
            transparent_shader.setUniform(base + ".constant", point_lights[i].constant);
            transparent_shader.setUniform(base + ".linear", point_lights[i].linear);
            transparent_shader.setUniform(base + ".quadratic", point_lights[i].quadratic);
        }
        transparent_shader.setUniform("spotLight.position", camera.Position);
        transparent_shader.setUniform("spotLight.direction", camera.Front);
        transparent_shader.setUniform("spotLight.ambient", spot_light.ambient);
        transparent_shader.setUniform("spotLight.diffuse", spot_light.diffuse);
        transparent_shader.setUniform("spotLight.specular", spot_light.specular);
        transparent_shader.setUniform("spotLight.constant", spot_light.constant);
        transparent_shader.setUniform("spotLight.linear", spot_light.linear);
        transparent_shader.setUniform("spotLight.quadratic", spot_light.quadratic);
        transparent_shader.setUniform("spotLight.cutOff", spot_light.cutOff);
        transparent_shader.setUniform("spotLight.outerCutOff", spot_light.outerCutOff);

        for (const auto& model : transparent_models) {
            transparent_shader.setUniform("u_diffuse_color", model->diffuse_color);
            model->draw(transparent_shader);
        }
        glDepthMask(GL_TRUE);

        // --- Vykreslení světýlek (lamp?) ---
        lamp_shader.activate();
        lamp_shader.setUniform("uP_m", projection_matrix);
        lamp_shader.setUniform("uV_m", view_matrix);

        for (int i = 0; i < NR_POINT_LIGHTS; i++)
        {
            glm::mat4 model_m = glm::mat4(1.0f);
            model_m = glm::translate(model_m, point_lights[i].position);
            model_m = glm::scale(model_m, glm::vec3(0.2f));
            light_cube_model.setMatrix(model_m);
            lamp_shader.setUniform("ourColor", glm::vec4(point_lights[i].diffuse, 1.0f));
            light_cube_model.draw(lamp_shader);
        }

        // --- Vykreslení slunce ---
        glm::mat4 model_m = glm::mat4(1.0f);
        model_m = glm::translate(model_m, light_pos);
        model_m = glm::scale(model_m, glm::vec3(25.0f));
        light_cube_model.setMatrix(model_m);
        lamp_shader.setUniform("ourColor", glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        light_cube_model.draw(lamp_shader);

        // --- Vykreslení druhého slunce ---
        model_m = glm::mat4(1.0f);
        model_m = glm::translate(model_m, light_pos2);
        model_m = glm::scale(model_m, glm::vec3(25.0f));
        light_cube_model.setMatrix(model_m);
        lamp_shader.setUniform("ourColor", glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); // Oranžová barva
        light_cube_model.draw(lamp_shader);

        // Swapování bufferů a zpracování eventů
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

// --- Aktualizace projekční matice ---
void App::update_projection_matrix() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (height == 0) height = 1;
    float ratio = (float)width / height;
    projection_matrix = glm::perspective(glm::radians(camera.Zoom), ratio, 0.1f, 1000.0f);
}

// --- Zpracování vstupu ---
void App::process_input(float delta_time) {
    // Reset horizontální rychlosti každý snímek
    camera_velocity.x = 0.0f;
    camera_velocity.z = 0.0f;

    // Vektor pohybu, který je nezávislý na sklonu kamery
    glm::vec3 front_flat = glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z));
    glm::vec3 right_flat = glm::normalize(glm::cross(front_flat, camera.WorldUp));

    // Výpočet vektoru podle vstupu
    glm::vec3 intended_movement(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        intended_movement += front_flat;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        intended_movement -= front_flat;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        intended_movement -= right_flat;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        intended_movement += right_flat;

    // Normalizace vektoru pohybu a aplikace rychlosti
    if (glm::length(intended_movement) > 0.0f) {
        intended_movement = glm::normalize(intended_movement) * camera.MovementSpeed;

        // Chození po svahu
        if (camera_on_ground) {
            glm::vec3 new_pos = camera.Position + intended_movement * delta_time;
            glm::vec3 terrain_normal = getTerrainNormal(new_pos.x, new_pos.z);
            const float max_slope_y_component = 0.7f; // Povolení chůze po svazích do ~45 stupňů

            if (terrain_normal.y < max_slope_y_component) { // Příliš strmé
                intended_movement = intended_movement - glm::dot(intended_movement, terrain_normal) * terrain_normal;
            }
        }

        // Aktualizace rychlosti kamery
        camera_velocity.x = intended_movement.x;
        camera_velocity.z = intended_movement.z;
    }

    // Skákání
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && camera_on_ground) {
        camera_velocity.y = 10.0f; // Rychlost skoku
        camera_on_ground = false;
        ma_sound_seek_to_pcm_frame(&jump_sound, 0);
        ma_sound_start(&jump_sound);
    }
}

// --- Generování labyrintu ---
// Získání znaku na zadané pozici v mapě
uchar App::getmap(cv::Mat& map, int x, int y)
{
    x = std::clamp(x, 0, map.cols);
    y = std::clamp(y, 0, map.rows);

    return map.at<uchar>(y, x);
}

// Vytváření chodeb v labyrintu pomocí recusrive backtracking algoritmu
void App::carve_passages(int cx, int cy, cv::Mat& map, std::default_random_engine& rng) {
    std::vector<int> directions = { 0, 1, 2, 3 };
    std::shuffle(directions.begin(), directions.end(), rng);

    for (int direction : directions) {
        int nx = cx;
        int ny = cy;

        switch (direction) {
        case 0: // North
            ny -= 2;
            break;
        case 1: // South
            ny += 2;
            break;
        case 2: // East
            nx += 2;
            break;
        case 3: // West
            nx -= 2;
            break;
        }

        if (nx > 0 && nx < map.cols - 1 && ny > 0 && ny < map.rows - 1 && map.at<uchar>(ny, nx) == '#') {
            map.at<uchar>(ny, nx) = '.';
            map.at<uchar>(cy + (ny - cy) / 2, cx + (nx - cx) / 2) = '.';
            carve_passages(nx, ny, map, rng);
        }
    }
}

// Generace labyrintu
void App::genLabyrinth(cv::Mat& map) {
    // Inicializace mapy
    for (int j = 0; j < map.rows; j++) {
        for (int i = 0; i < map.cols; i++) {
            map.at<uchar>(cv::Point(i, j)) = '#';
        }
    }

    // Vytvoření generátoru náhodných čísel
    std::random_device r;
    std::default_random_engine rng(r());

    // Nastavení počátečního bodu pro vytváření chodeb
    int start_x = 1;
    int start_y = 1;

    // Vytvoření chodeb v labyrintu
    map.at<uchar>(start_y, start_x) = '.';
    carve_passages(start_x, start_y, map, rng);

    // Nastavení počátečního a koncového bodu
    cv::Point2i start_position, end_position;
    std::uniform_int_distribution<int> uniform_height(1, map.rows - 2);
    std::uniform_int_distribution<int> uniform_width(1, map.cols - 2);

    do {
        start_position.x = uniform_width(rng);
        start_position.y = uniform_height(rng);
    } while (getmap(map, start_position.x, start_position.y) == '#');

    do {
        end_position.x = uniform_width(rng);
        end_position.y = uniform_height(rng);
    } while (start_position == end_position || getmap(map, end_position.x, end_position.y) == '#');
    map.at<uchar>(cv::Point(end_position.x, end_position.y)) = 'e';

    // Vypsání labyrintu do konzole
    std::cout << "Start: " << start_position << std::endl;
    std::cout << "End: " << end_position << std::endl;

    for (int j = 0; j < map.rows; j++) {
        for (int i = 0; i < map.cols; i++) {
            if ((i == start_position.x) && (j == start_position.y))
                std::cout << 'X';
            else
                std::cout << getmap(map, i, j);
        }
        std::cout << std::endl;
    }

    // Nastavení pozice kamery
    camera.Position = glm::vec3(43.0f, 103.0f, 60.0f);
}

// --- Generování výškové mapy a terénu ---
// Vrací souřadnice subtextury z obrázku textur
glm::vec2 App::get_subtex_st(const int x, const int y)
{
    return glm::vec2(x * 1.0f / 16.0f, y * 1.0f / 16.0f);
}

// Vrací subtexturu na základě výšky
glm::vec2 App::get_subtex_by_height(float height)
{
    if (height > 0.9f)
        return get_subtex_st(2, 4); //sníh
    else if (height > 0.8f)
        return get_subtex_st(3, 4); //led
    else if (height > 0.5f)
        return get_subtex_st(6, 0); //skála
    else if (height > 0.3f)
        return get_subtex_st(3, 1); //půda
    else
        return get_subtex_st(0, 0); //tráva
}

// Generuje "model" výškové mapy
Model App::GenHeightMap(cv::Mat& hmap, const unsigned int mesh_step_size, const cv::Rect& flatten_area, uchar flatten_height)
{
    // Zploštění kusu ve výškové mapě
    cv::rectangle(hmap, flatten_area, cv::Scalar(flatten_height), -1);

    std::vector<vertex> vertices;
    std::vector<GLuint> indices;

    // Generování vrcholů a indexů pro model výškové mapy
    for (unsigned int x_coord = 0; x_coord < (hmap.cols - mesh_step_size); x_coord += mesh_step_size)
    {
        for (unsigned int z_coord = 0; z_coord < (hmap.rows - mesh_step_size); z_coord += mesh_step_size)
        {
            // Získání čtyř rohových bodů čtyřúhelníku
            glm::vec3 p0(x_coord, hmap.at<uchar>(cv::Point(x_coord, z_coord)), z_coord);
            glm::vec3 p1(x_coord + mesh_step_size, hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord)), z_coord);
            glm::vec3 p2(x_coord + mesh_step_size, hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord + mesh_step_size)), z_coord + mesh_step_size);
            glm::vec3 p3(x_coord, hmap.at<uchar>(cv::Point(x_coord, z_coord + mesh_step_size)), z_coord + mesh_step_size);

            // Získání maximální výšky čtyřúhelníku
            float max_h = (std::max)({ (float)hmap.at<uchar>(cv::Point(x_coord, z_coord)) / 255.0f,
                                     (float)hmap.at<uchar>(cv::Point(x_coord, z_coord + mesh_step_size)) / 255.0f,
                                     (float)hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord + mesh_step_size)) / 255.0f,
                                     (float)hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord)) / 255.0f });

            // Získání souřadnic textury na základě výšky
            glm::vec2 tc0 = get_subtex_by_height(max_h);
            glm::vec2 tc1 = tc0 + glm::vec2(1.0f / 16.0f, 0.0f);
            glm::vec2 tc2 = tc0 + glm::vec2(1.0f / 16.0f, 1.0f / 16.0f);
            glm::vec2 tc3 = tc0 + glm::vec2(0.0f, 1.0f / 16.0f);

            // Výpočet normals
            glm::vec3 n1 = glm::normalize(glm::cross(p2 - p0, p1 - p0));
            glm::vec3 n2 = glm::normalize(glm::cross(p3 - p0, p2 - p0));
            glm::vec3 navg = glm::normalize(n1 + n2);

            // Přidání vertexů a indices do vektorů
            unsigned int start_index = static_cast<unsigned int>(vertices.size());
            vertices.emplace_back(vertex{ p0, navg, tc0 });
            vertices.emplace_back(vertex{ p1, n1,   tc1 });
            vertices.emplace_back(vertex{ p2, navg, tc2 });
            vertices.emplace_back(vertex{ p3, n2,   tc3 });

            indices.push_back(start_index + 0);
            indices.push_back(start_index + 1);
            indices.push_back(start_index + 2);
            indices.push_back(start_index + 0);
            indices.push_back(start_index + 2);
            indices.push_back(start_index + 3);
        }
    }

    // Vytvoření a vrácení modelu
    Model model;
    model.meshes.emplace_back(GL_TRIANGLES, vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
    return model;
}

// Vrací normal terénu na zadané pozici
glm::vec3 App::getTerrainNormal(float x, float z) {
    if (x < 0 || x >= hmap.cols - 1 || z < 0 || z >= hmap.rows - 1) {
        return glm::vec3(0.0f, 1.0f, 0.0f);
    }

    int grid_x = static_cast<int>(x);
    int grid_z = static_cast<int>(z);

    float h00 = hmap.at<uchar>(cv::Point(grid_x, grid_z));
    float h10 = hmap.at<uchar>(cv::Point(grid_x + 1, grid_z));
    float h01 = hmap.at<uchar>(cv::Point(grid_x, grid_z + 1));
    float h11 = hmap.at<uchar>(cv::Point(grid_x + 1, grid_z + 1));

    glm::vec3 p00(grid_x, h00, grid_z);
    glm::vec3 p10(grid_x + 1, h10, grid_z);
    glm::vec3 p01(grid_x, h01, grid_z + 1);
    glm::vec3 p11(grid_x + 1, h11, grid_z + 1);

    float x_frac = x - grid_x;
    float z_frac = z - grid_z;

    glm::vec3 normal;
    if (x_frac > z_frac) { // Pravý dolní trojúhelník
        normal = glm::normalize(glm::cross(p10 - p00, p11 - p00));
    } else { // Levý horní trojúhelník
        normal = glm::normalize(glm::cross(p11 - p00, p01 - p00));
    }

    return normal;
}

// Vrací výšku terénu na zadané pozici pomocí bilinear interpolation
float App::getTerrainHeight(float x, float z) {
    if (x < 0 || x >= hmap.cols - 1 || z < 0 || z >= hmap.rows - 1) {
        return 0.0f;
    }

    int grid_x = static_cast<int>(x);
    int grid_z = static_cast<int>(z);

    float x_frac = x - grid_x;
    float z_frac = z - grid_z;

    float h00 = hmap.at<uchar>(cv::Point(grid_x, grid_z));
    float h10 = hmap.at<uchar>(cv::Point(grid_x + 1, grid_z));
    float h01 = hmap.at<uchar>(cv::Point(grid_x, grid_z + 1));
    float h11 = hmap.at<uchar>(cv::Point(grid_x + 1, grid_z + 1));

    // Bilineární interpolace
    float h_top = (1 - x_frac) * h00 + x_frac * h10;
    float h_bottom = (1 - x_frac) * h01 + x_frac * h11;
    return (1 - z_frac) * h_top + z_frac * h_bottom;
}

// --- Načítání textur ---
// Inicializace textur ze souboru
GLuint App::textureInit(const std::filesystem::path& file_name)
{
    cv::Mat image = cv::imread(file_name.string(), cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        throw std::runtime_error("V souboru neni zadna textura: " + file_name.string());
    }

    GLuint texture = gen_tex(image);

    return texture;
}

// Generuje texturu OpenGL z obrázku
GLuint App::gen_tex(cv::Mat& image)
{
    GLuint ID = 0;

    if (image.empty()) {
        throw std::runtime_error("Obrazek neni?");
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &ID);

    switch (image.channels()) {
    case 3:
        glTextureStorage2D(ID, 1, GL_RGB8, image.cols, image.rows);
        glTextureSubImage2D(ID, 0, 0, 0, image.cols, image.rows, GL_BGR, GL_UNSIGNED_BYTE, image.data);
        break;
    case 4:
        glTextureStorage2D(ID, 1, GL_RGBA8, image.cols, image.rows);
        glTextureSubImage2D(ID, 0, 0, 0, image.cols, image.rows, GL_BGRA, GL_UNSIGNED_BYTE, image.data);
        break;
    default:
        throw std::runtime_error("Nepodorovany pocet channelu v texture:" + std::to_string(image.channels()));
    }

    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateTextureMipmap(ID);

    glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return ID;
}