cmake_minimum_required(VERSION 3.24) 
project(app VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)

find_package(ImGui-SFML REQUIRED)

add_executable(${PROJECT_NAME} main.cpp app.cpp object.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        ImGui-SFML::ImGui-SFML
)