#include "app.h"
#include <cstddef>



void app::run() {
    sf::RenderWindow window(sf::VideoMode({app::SCREEN_WIDTH, app::SCREEN_HEIGHT}), "App");
    window.setFramerateLimit(60);


    // 1. Инициализация ImGui для SFML
    if (!ImGui::SFML::Init(window)) {
        // Если инициализация провалилась, выходим или логируем ошибку
        return; 
    }

    obj::World World;
    World.Build();
    sf::Clock deltaClock; // Часы для измерения времени между кадрами

    while (window.isOpen()) {
        // 2. Обработка событий SFML 3.0
        // В SFML 3.0 pollEvent возвращает std::optional<sf::Event>
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            // Передаем событие в ImGui, чтобы он понимал клики и клавиши
            ImGui::SFML::ProcessEvent(window, *event);
        }

        // 3. Обновление ImGui (передаем дельту времени!)
        sf::Time dt = deltaClock.restart();
        ImGui::SFML::Update(window, dt);

        // --- ТВОЙ ИГРОВОЙ КОД ---
        // Логика игры, обновление позиций и т.д.

        // --- РИСОВАНИЕ ---
        World.Update(dt.asSeconds());

        ImGui::Begin("bebe");
            ImGui::Text("Some test text");
            if (ImGui::Button("Add car")) {
                obj::Lane* lane = World.Get_roads()[0]->GetLanes()[0];

                if (lane != nullptr) {
                    obj::Car* new_car = new obj::Car(lane);
                    lane->Car_push(new_car);
                }
            }
        ImGui::End();


        app::Render_world(window, World);
        // Рисуем игровые объекты (спрайты, фигуры)
        // window.draw(mySprite);

        // 4. Рендер ImGui (поверх всего, что нарисовано в window)
        ImGui::SFML::Render(window);

        // 5. Вывод на экран
        window.display();
    }

    // 6. Очистка памяти
    ImGui::SFML::Shutdown();
}   


void app::Render_car(sf::RenderWindow& window, const obj::Car* car) {
    if (!car || !car->IsAlive()) {
        return;
    }

    sf::Vector2f pos = car->GetPos();
    sf::Vector2f dir = car->GetDirection();
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.0f) {
        dir.x /= len;
        dir.y /= len;
    } else {
        dir = {1.0f, 0.0f};
    }

    sf::Vector2f perp{-dir.y, dir.x};
    float half_len = obj::Meters_to_Px(config::Car_length_m) * 0.5f;
    float half_width = obj::Meters_to_Px(config::Car_width_m) * 0.5f;
    sf::Vector2f corners[4];
    corners[0] = pos + dir * half_len + perp * half_width;
    corners[1] = pos - dir * half_len + perp * half_width;
    corners[2] = pos - dir * half_len - perp * half_width;
    corners[3] = pos + dir * half_len - perp * half_width;

    sf::ConvexShape shape;
    shape.setPointCount(4);
    for (int i = 0; i < 4; ++i) {
        shape.setPoint(i, app::WorldToScreen(corners[i]));
    }
    shape.setFillColor(sf::Color::Red);
    window.draw(shape);
}



void app::Render_lane(sf::RenderWindow& window, const obj::Lane* lane) {
    if (!lane) {
        return;
    }

    const sf::Vector2f* c = lane->GetCorners();

    sf::ConvexShape shape;
    shape.setPointCount(4);

    for (int i = 0; i < 4;i++) {
        shape.setPoint(i, app::WorldToScreen(c[i]));
    }

    shape.setFillColor(colors::Asphalt);
    shape.setOutlineColor(colors::Sep_line);
    shape.setOutlineThickness(1.0f);

    window.draw(shape);
    for (const obj::Car* car : lane->GetCars()) {
        app::Render_car(window, car);
    }
}

void app::Render_road(sf::RenderWindow& window, const obj::Road* road) {
    if(!road) {
        return;
    }
    const std::vector<obj::Lane*>&  lanes = road->GetLanes();
    for(int i = 0;i < lanes.size();i++) {
        Render_lane(window, lanes[i]);
    }
    
}


void app::Render_crossroad(sf::RenderWindow& window,const obj::Crossroad* crossroad) {
    if(crossroad == nullptr)
        return;
}


void app::Render_world(sf::RenderWindow& window, obj::World& world) {
    window.clear(colors::Background);

    const obj::Crossroad* crossroad = world.Get_crossroad(); 
    Render_crossroad(window, crossroad);
    const std::vector<obj::Road*>& roads = world.Get_roads();
    for(int i = 0;i < roads.size();i++) {
        Render_road(window, roads[i]);
    }
}