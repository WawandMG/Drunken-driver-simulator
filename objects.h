#include <SFML/Graphics.hpp>



// Пространство имён для описания классов используемых в проекте 
namespace obj {

    // Родительский класс динамичных объектов
    class Entity {
        private:
            // Позиция объекта
            sf::Vector2f Pos;
            /* 
                Укзаывает направление в которое смотрит объект 
                В обычном случае – нормализованная velocity
            */
            sf::Vector2f Direction;
            // Передвижение в пикселях, совершаемое объектом за 1 секунду времени
            sf::Vector2f Velocity;

            // На сколько пикселей проходимых за секунду ускоряется объект
            const float Acceleration;
            // Максимальная скорость достигаемая объектом
            const float Speed_limit;
            // Здоровье объекта :р
            const int HP;

        public:
            // Получить позицию для отрисовки
            sf::Vector2f GetPos() { return Pos; }
            // Получить направления взгляда объекта для отрисовки
            sf::Vector2f GetDirection() { return Direction; };

            Entity() : Acceleration(0), Speed_limit(0), HP(100) {
                Pos = {0, 0};
                Direction = {1, 1};
                Velocity = {0, 0};
            };
    };

    // Класс машины, набросок
    class Car : public Entity {

    };
    // Класс пешехода, набросок
    class Pedastrian : public Entity {

    };


    // Класс для статических объектов 
    class Static_entity {

    };
}