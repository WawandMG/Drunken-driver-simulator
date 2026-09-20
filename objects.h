#include <SFML/Graphics.hpp>
#include <random>
using namespace std;
 // ВВ нам за это жопу сломает, пока оставлю, но сам буду писать с std::. Если тебе норм, рекомендую убрать

mt19937 rnd(time(nullptr));

inline int Gen(int l, int r) {// генератор рандомного целого числа в [l;r]
    uniform_int_distribution<int> dist(l, r);
    return dist(rnd);
}

inline int GenFloat(float l, float r) {
    uniform_real_distribution<float> dist(l, r);
    return dist(rnd);
}

namespace config {
    const float Pixels_to_meter = 5.0f;

}


// Пространство имён для описания классов используемых в проекте
namespace obj {

    // Перевод километров в пиксели, для вычесления скорости и ускорения
    float Km_to_Px(float km) { return km * 1000 * config::Pixels_to_meter; };

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
            // Передвижение в км/ч, для экрана перевод в отдельной функции
            sf::Vector2f Velocity;

            //текущая скорость
            float Speed;
            // На сколько км/ч 
            float Acceleration;
            // Максимальная скорость достигаемая объектом
            float Speed_limit; //пусть будет пока что 120, как сказано в тз

        public:
            // Получить позицию для отрисовки
            sf::Vector2f GetPos() { return Pos; }
            // Получить направления взгляда объекта для отрисовки
            sf::Vector2f GetDirection() { return Direction; }

            // Получить лимит скорости объекта
            float GetSpeedLimit() { return Speed_limit; }

            //установка лимита скорости
            void SetSpeedLimit(float x) { Speed_limit = x; }

            //установка скорости
            void SetSpeed(float x) {Speed = x;};

            //установка ускорения
            void SetAcceleration(float x) {Acceleration = x;}

            Entity() : Acceleration(0), Speed_limit(0) {
                Pos = {0, 0};
                Direction = {1, 1};
                Velocity = {0, 0};
            };
    };

    // Класс машины, набросок
    class Car : public Entity {

    };
    // Класс пешехода, набросок
    class Pedestrian : public Entity {
        public:
            Pedestrian() {
                SetSpeedLimit(8);

                float default_speed = GenFloat(3, 8);
                SetSpeed(default_speed);

                float dist = GetSpeedLimit() - default_speed;
                float default_acceleration = GenFloat(0, dist);
                SetAcceleration(default_acceleration);
            }
    };


    // Класс для статических объектов 
    class Static_entity {
        sf::Vector2f Pos; //позиция на экране

    };


    // Класс линий движения
    class Lane {
        private:
            // Координаты углов линии для отрисовки.
            float UL_corner, UR_corner, DL_corner, DR_corner; 
            /* 
                Вектор направления дороги
                {1, 0} - Восток
                {-1, 0} - Запад
                {0, 1} - Север (нужно нормировать координаты, сделать центр экрана - центром координат)
                {0, -1} - Юг
            */
            //sf::Vector2f Direction;
            /*Массив точек-ориентиров по которым будут двигаться машины*/
            std::vector<sf::Vector2f> waypoints;
        public:

    };

    // Класс дороги
    class Road {
        private:
            std::vector<Lane*> Lanes;
        public:

    };

    // Класс перекрёсток
    class Crossroad {

    };

    // Класс мира/карты
    class World {
        private:
            Crossroad* crossroad;
            std::vector<Road*> roads;
        public:
        
    };

}

