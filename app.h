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
    inline sf::Color Light_body({30, 30, 30});
    inline sf::Color Lamp_red({220, 40, 40});
    inline sf::Color Lamp_yellow({230, 190, 40});
    inline sf::Color Lamp_green({40, 170, 60});
    inline sf::Color Lamp_red_off({70, 20, 20});
    inline sf::Color Lamp_yellow_off({70, 60, 20});
    inline sf::Color Lamp_green_off({20, 50, 25});
    inline sf::Color Sidewalk({130, 130, 130});
}

namespace app {
    inline constexpr unsigned int SCREEN_WIDTH = 1200;
    inline constexpr unsigned int SCREEN_HEIGHT = 800;

    inline sf::Vector2f WorldToScreen(sf::Vector2f p) {
        const float center_x = (float)SCREEN_WIDTH * 0.5f;
        const float center_y = (float)SCREEN_HEIGHT * 0.5f;

        return {
            p.x + center_x,
            center_y - p.y
        };
    }
    void run();

    void Render_car(sf::RenderWindow& window, const obj::Car* car);
    void Render_lane(sf::RenderWindow& window, const obj::Lane* lane);
    void Render_road(sf::RenderWindow& window, const obj::Road* road);
    void Render_crossing(sf::RenderWindow& window, const obj::Road* road);
    void Render_light(sf::RenderWindow& window, const obj::TrafficLight& light);
    void Render_pedestrian(sf::RenderWindow& window, const obj::Pedestrian* ped);
    void Render_crossroad(sf::RenderWindow& window, const obj::Crossroad* crossroad);
    void Render_world(sf::RenderWindow& window, obj::World& world, sf::Sprite& Background);
}


class background_manager {
    private:
    int Current_texture = 0;
    std::vector<std::string> names = {
        "None",
        "no name"
    };
    std::vector<std::string> paths = {
        "../assets/Surgut.jpg",
        "../assets/goose.jpg",
        "../assets/hell_peter.jpg",
        "../assets/paris.jpeg"
    };

    std::vector<sf::Texture> textures;

    public:
    void Init_textures() {
        Current_texture = 0;
        textures.resize(paths.size()); 
        sf::Texture texture_buffer;
        for (int i = 0;i < paths.size();i++) {
            if (!textures[i].loadFromFile(paths[i])) {
                std::cerr << "Warning: Failed to load " << paths[i] << std::endl;
            }
        }
    }
    void Get_background(sf::Sprite& Background) {
        Background.setTexture(textures[Current_texture]);
        Background.setPosition({0, 0});
        Background.setScale({float(app::SCREEN_WIDTH) / float(textures[Current_texture].getSize().x), 
                             float(app::SCREEN_HEIGHT) / float(textures[Current_texture].getSize().y)});
    }

    void Next_background(sf::Sprite& Background) {
        Current_texture++;
        Current_texture %= textures.size();
        Get_background(Background);
    }

    background_manager() = default;
};
