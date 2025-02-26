cmake_minimum_required(VERSION 3.10.0)
project(RotatingCube VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Nastavení vcpkg (nová cesta!)
set(CMAKE_TOOLCHAIN_FILE "/Users/dante/Desktop/Projekty/PG2_project/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "arm64-osx")

# Nastavení pro macOS
if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
endif()

# Najdi knihovny
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED) 
find_package(OpenCV REQUIRED)

add_executable(RotatingCube main.cpp)

target_link_libraries(RotatingCube PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm
    nlohmann_json::nlohmann_json
    ${OpenCV_LIBS}  # Přidejte OpenCV knihovny
)