#include <SFML/Graphics.hpp>
#include <random>
#include <vector>
using namespace std;
 // ВВ нам за это жопу сломает, пока оставлю, но сам буду писать с std::. Если тебе норм, рекомендую убрать

mt19937 rnd(time(nullptr));

inline int Gen(int l, int r) {// генератор рандомного целого числа в [l;r]
    uniform_int_distribution<int> dist(l, r);
    return dist(rnd);
}

inline float GenFloat(float l, float r) {
    uniform_real_distribution<float> dist(l, r);
    return dist(rnd);
}

namespace config {
    // 8 пикселей = 1 метр, окно ~1200x800, центр экрана = перекрёсток
    const float Pixels_to_meter = 8.0f;
    const float Lane_width = 3.5f;   // метры
    const float Car_length = 4.5f;
    const float Crossing_dist = 12.5f; // от центра до зебры
    const float Stop_dist = 16.5f;     // от центра до стоп-линии
    const float Road_length = 44.f;    // от центра до края
    const float Waypoint_step = 10.f;
}

using namespace std;

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

    class Lane;

    // Класс машины, набросок
    class Car : public Entity {
        public:
            Lane* lane;
            int waypoint_i; // к какой точке сейчас едем
            // потом: соседний ряд + прогресс 0..1

            Car() {
                SetSpeedLimit(120);

                float default_speed = GenFloat(30, 120);
                SetSpeed(default_speed);

                float dist = GetSpeedLimit() - default_speed;
                float default_acceleration = GenFloat(0, dist);
                SetAcceleration(default_acceleration);
            }

            // если впереди машина/стоп — тормозим, иначе газуем до Speed_limit
            void Update(float dt);
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
            int Direction;
            /* точки от перекрёстка наружу, машина едет по прямой между соседними
               [0] зебра
               [1] стоп-линия — тут останавливаемся перед перекрёстком
               дальше вдоль ряда */
            std::vector<sf::Vector2f> waypoints;
            std::vector<Car*> cars;
        public:
            const std::vector<sf::Vector2f>& GetWaypoints() { return waypoints; }
            sf::Vector2f Crossing() { return waypoints[0]; }
            sf::Vector2f StopLine() { return waypoints[1]; }

            void AddWaypoint(sf::Vector2f p) { waypoints.push_back(p); }

            // outward — от перекрёстка к краю, offset — сдвиг ряда вбок (метры)
            void Build(sf::Vector2f outward, sf::Vector2f offset) {
                waypoints.clear();
                auto at = [&](float d) { return outward * d + offset; };
                AddWaypoint(at(config::Crossing_dist));
                AddWaypoint(at(config::Stop_dist));
                for (float d = config::Stop_dist + config::Waypoint_step; d < config::Road_length - 0.5f; d += config::Waypoint_step) {
                    AddWaypoint(at(d));
                }
                AddWaypoint(at(config::Road_length));
            }

            Car* CarAhead(Car* self); // ближайшая машина впереди, потом для торможения
    };

    // Класс дороги
    class Road {
        private:
            std::vector<Lane*> Lanes;
        public:
            void AddLane(Lane* l) { Lanes.push_back(l); }
            std::vector<Lane*>& GetLanes() { return Lanes; }

            // одна дорога = один подъезд к перекрёстку
            // outward: запад {-1,0}, восток {1,0}, север {0,1}, юг {0,-1}
            void Build(sf::Vector2f outward, int n = 3) {
                sf::Vector2f travel = -outward;
                sf::Vector2f right = {travel.y, -travel.x};
                for (int i = 0; i < n; ++i) {
                    Lane* l = new Lane();
                    l->Build(outward, right * ((i + 0.5f) * config::Lane_width));
                    AddLane(l);
                }
            }

            // соседний ряд для перестроения: Lanes[i ± 1]
            Lane* Neighbor(Lane* lane, int side);
    };

    // Класс перекрёсток
    class Crossroad {

    };

    // Класс мира/карты
    class World {
        private:
            Crossroad* crossroad;
            std::vector<Road*> roads; // 4 подъезда: З, В, С, Ю
        public:
            void Build() {
                auto make = [&](sf::Vector2f outward) {
                    Road* r = new Road();
                    r->Build(outward);
                    roads.push_back(r);
                };
                make({-1.f, 0.f});
                make({1.f, 0.f});
                make({0.f, 1.f});
                make({0.f, -1.f});
            }
    };

}
