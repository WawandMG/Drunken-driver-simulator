#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <objects.h>



namespace app {
    inline constexpr unsigned int SCREEN_WIDTH = 1200;
    inline constexpr unsigned int SCREEN_HEIGHT = 800;

    void run();
}