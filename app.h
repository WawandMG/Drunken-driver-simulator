#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include "objects.h"

namespace colors {
    inline sf::Color Background({0, 255, 0});
    inline sf::Color Asphalt({70, 70, 70});
    inline sf::Color Sep_line({200, 200, 200});
}

namespace app {
    inline constexpr unsigned int SCREEN_WIDTH = 1200;
    inline constexpr unsigned int SCREEN_HEIGHT = 800;

    inline sf::Vector2f WorldToScreen(sf::Vector2f p) {
        const float center_x = static_cast<float>(SCREEN_WIDTH) * 0.5f;
        const float center_y = static_cast<float>(SCREEN_HEIGHT) * 0.5f;

        return {
            p.x + center_x,
            center_y - p.y
        };
    }
    void run();

    void Render_car(sf::RenderWindow& window, const obj::Car* car);
    void Render_lane(sf::RenderWindow& window, const obj::Lane* lane);
    void Render_road(sf::RenderWindow& window, const obj::Road* road);
    void Render_crossroad(sf::RenderWindow& window, const obj::Crossroad* crossroad);
    void Render_world(sf::RenderWindow& window, obj::World& world);
}