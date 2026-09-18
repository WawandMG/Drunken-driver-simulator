#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <random>

using namespace std;

mt19937 rnd(time(nullptr));

int Gen(int l, int r) {// генератор рандомного целого числа в [l;r]
    uniform_int_distribution<int> dist(l, r);
    return dist(rnd);
}

int GenFloat(float l, float r) {
    uniform_real_distribution<float> dist(l, r);
    return dist(rnd);
}

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

        //текущая скорость
        float Speed;
        // На сколько км/ч проходимых за секунду ускоряется объект
        float Acceleration;
        // Максимальная скорость достигаемая объектом
        float Speed_limit;//пусть будет пока что 120, как сказано в тз
        // Здоровье объекта :р
        int HP;

    public:
        // Получить позицию для отрисовки
        sf::Vector2f GetPos() { return Pos; }
        // Получить направления взгляда объекта для отрисовки
        sf::Vector2f GetDirection() { return Direction; }

        // Получить лимит скорости объекта
        int GetSpeedLimit() { return Speed_limit; }

        //установка лимита скорости
        void SetSpeedLimit(float x) { Speed_limit = x; }

        //установка скорости
        void SetSpeed(float x) {Speed = x;};

        //установка ускорения
        void SetAcceleration(float x) {Acceleration = x;}

        Entity() : Acceleration(0), Speed_limit(0), HP(100) {
            Pos = {0, 0};
            Direction = {1, 1};
            Velocity = {0, 0};
        };
    };

    // Класс машины, набросок
    class Car : public Entity {
        public:
            Car() {
                SetSpeedLimit(120);

                float default_speed = GenFloat(30, 120);
                SetSpeed(default_speed);

                float dist = GetSpeedLimit() - default_speed;
                float default_acceleration = GenFloat(0, dist);
                SetAcceleration(default_acceleration);
            }
    };


    // Класс пешехода, набросок
    class Pedastrian : public Entity {
    public:
        Pedastrian() {
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
        sf::Vector2f Pos;//позиция на экране

    };

    class Road : Static_entity {
    private:
        int type = 0;// будет 4 дороги - все они разные по направлению, то есть у каждой свой тип
        bool active = 0;// если дорога пуста = не активна
        vector<Car>Cars;//текущие машины на дороге
    public:
    };

}

