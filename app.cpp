#include "app.h"

void app::run() {
    sf::RenderWindow window(sf::VideoMode({app::SCREEN_WIDTH, app::SCREEN_HEIGHT}), "App");
    window.setFramerateLimit(60);

    // 1. Инициализация ImGui для SFML
    ImGui::SFML::Init(window);

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
        window.clear(sf::Color::Black);
        ImGui::Begin("bebe");
            ImGui::Text("Some test text");
        ImGui::End();
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