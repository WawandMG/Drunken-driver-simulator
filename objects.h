#include <SFML/Graphics.hpp>
#include <random>
#include <vector>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <memory>

using namespace std;
 // ВВ нам за это жопу сломает, пока оставлю, но сам буду писать с std::. Если тебе норм, рекомендую убрать

inline mt19937 rnd(time(nullptr));

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
    inline constexpr float Pixels_to_meter = 8.0f;
    // Дальнейшие величины - указаны в метрах

    inline constexpr float Lane_width_m = 3.5f;   // метры
    inline constexpr float Car_length_m = 4.5f;
    inline constexpr float Car_width_m = 1.8f;
    inline constexpr float Crossing_dist_m = 12.5f; // от центра до зебры
    inline constexpr float Stop_dist_m = 16.5f;     // от центра до стоп-линии
    inline constexpr float Road_length_m = 44.f;    // от центра до края
    inline constexpr float Waypoint_step_m = 10.f;
    inline constexpr float Waypoint_reach_m = 1.0f; // Расстояние на котором машина считает, что достигла waypoint
    enum class direction {
        North = 0,  // Дорога направлена наверх
        South = 1,  // Дорога направлена вниз
        West = 2,   // Дорога направлена влево
        East = 3    // Дорога направлена вправо
    };
}

inline float get_distance(sf::Vector2f position_a, sf::Vector2f position_b) {
    sf::Vector2f a_b = position_a - position_b;
    return std::sqrt(a_b.x * a_b.x + a_b.y * a_b.y);
};
// inline float get_speed(sf::Vector2f Velocity) {
//     return std::sqrt(Velocity.x * Velocity.x + Velocity.y * Velocity.y) / config::Pixels_to_meter * 3.6f;
// };

using namespace std;

// Пространство имён для описания классов используемых в проекте
namespace obj {

    // Перевод километров в пиксели, для вычесления скорости и ускорения
    inline float Meters_to_Px(float meters) {
        return meters * config::Pixels_to_meter;
    }

    inline float Km_to_Px(float kmh) {
        return kmh * 1000.0f * config::Pixels_to_meter / 3600.0f;
    }
    // Родительский класс динамичных объектов
    class Entity {
        protected:
            // Позиция объекта
            sf::Vector2f Pos;
            /*
                Укзаывает направление в которое смотрит объект
                В обычном случае – нормализованная velocity
            */
            sf::Vector2f Direction;
            // Передвижение в км/ч, для экрана перевод в отдельной функции
            sf::Vector2f Velocity;

            //текущая скорость в км/ч
            float Speed;
            // На сколько км/ч
            float Acceleration;
            // Максимальная скорость достигаемая объектом
            float Speed_limit; //пусть будет пока что 120, как сказано в тз


            void NormalizeDirection() {
                float len = std::sqrt(Direction.x * Direction.x + Direction.y * Direction.y);
                if (len > 0.0f) {
                    Direction.x /= len;
                    Direction.y /= len;
                }
            }
        public:
            // Получить позицию для отрисовки
            sf::Vector2f GetPos() const { return Pos; }
            // Получить направления взгляда объекта для отрисовки
            sf::Vector2f GetDirection() const { return Direction; }

            // Получить лимит скорости объекта
            float GetSpeedLimit() const { return Speed_limit; }

            //установка лимита скорости
            void SetSpeedLimit(float x) { Speed_limit = x; }

            //установка скорости
            void SetSpeed(float x) {Speed = x;};

            //установка ускорения
            void SetAcceleration(float x) {Acceleration = x;}

            Entity() : Speed(0.0f), Acceleration(0.0f), Speed_limit(0.0f) {
                Pos = {0, 0};
                Direction = {1, 1};
                NormalizeDirection();
                Velocity = {0, 0};
            };
    };

    class Lane;

    // Класс машины, набросок
    class Car : public Entity {
        protected:
            Lane* lane = nullptr;
            int waypoint_i = 0; // к какой точке сейчас едем
            // потом: соседний ряд + прогресс 0..1
            bool alive = true;
        public:
            bool IsAlive() const {
                return alive;
            }

            Car() {
                SetSpeedLimit(120);

                float default_speed = GenFloat(30, 120);
                SetSpeed(default_speed);

                float dist = GetSpeedLimit() - default_speed;
                float default_acceleration = GenFloat(0, dist);
                SetAcceleration(default_acceleration);
            }

            explicit Car(Lane* lane_pointer);
            

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
            sf::Vector2f corners[4]; // 4 угла полосы для отрисовки
            
            config::direction Direction;
            /*
            waypoints хранятся от перекрёстка наружу:

            [0] зебра
            [1] стоп-линия
            ...
            [last] край дороги

            Машина спавнится у last и едет к [0], поэтому waypoint_i уменьшается.
            */
            std::vector<sf::Vector2f> waypoints;
            std::vector<Car*> cars;
        public:
            const std::vector<sf::Vector2f>& GetWaypoints() const { return waypoints; }
            const sf::Vector2f* GetCorners() const { return corners; }
            sf::Vector2f Crossing() { return waypoints[0]; }
            sf::Vector2f StopLine() { return waypoints[1]; }
            const std::vector<Car*>& GetCars() const { return cars; }

            void AddWaypoint(sf::Vector2f p) { waypoints.push_back(p); }

            // outward — от перекрёстка к краю, offset — сдвиг ряда вбок (метры)
            void Build(sf::Vector2f outward, sf::Vector2f offset) {
                waypoints.clear();
                auto at = [&](float d) { return outward * Meters_to_Px(d) + offset; };
                AddWaypoint(at(config::Crossing_dist_m));
                AddWaypoint(at(config::Stop_dist_m));
                for (float d = config::Stop_dist_m + config::Waypoint_step_m; d < config::Road_length_m - 0.5f; d += config::Waypoint_step_m) {
                    AddWaypoint(at(d));
                }
                AddWaypoint(at(config::Road_length_m));
                // Расстояние от центра перекрёстка, с которого начинается видимая полоса.
                // Нужно, чтобы дороги рисовались ВОКРУГ невидимого перекрёстка.
                const float near_d = config::Crossing_dist_m;

                // Если захочешь, чтобы дороги сходились прямо в центр экрана,
                // замени строку выше на:
                // const float near_d = 0.0f;

                // Перпендикуляр к направлению дороги.
                // Если outward = {x, y}, то perp = {-y, x}.
                // Это поворот вектора на 90 градусов.
                sf::Vector2f perp{-outward.y, outward.x};

                // Половина ширины полосы в пикселях.
                float half_w = Meters_to_Px(config::Lane_width_m) * 0.5f;

                // Ближний центр полосы, со стороны перекрёстка.
                sf::Vector2f near_center =
                    outward * Meters_to_Px(near_d) + offset;

                // Дальний центр полосы, у края дороги.
                sf::Vector2f far_center =
                    outward * Meters_to_Px(config::Road_length_m) + offset;

                // Четыре угла полосы.
                // Порядок важен для sf::ConvexShape.
                corners[0] = near_center + perp * half_w;
                corners[1] = near_center - perp * half_w;
                corners[2] = far_center  - perp * half_w;
                corners[3] = far_center  + perp * half_w;
            }
            void Update(float dt) {
                for (int i = static_cast<int>(cars.size()) - 1; i >= 0; --i) {
                    Car* car = cars[i];

                    if (!car) {
                        cars.erase(cars.begin() + i);
                        continue;
                    }

                    car->Update(dt);

                    if (!car->IsAlive()) {
                        delete car;
                        cars.erase(cars.begin() + i);
                    }
                }
            }

            void Car_erase(Car* self) {
                auto it = std::find(cars.begin(), cars.end(), self);

                if (it != cars.end()) {
                    if(*it != nullptr) {
                        delete *it;
                    }
                    cars.erase(it);
                }
            }
            void Car_push(Car* self) {
                cars.push_back(self);
            }

            Car* CarAhead(Car* self); // ближайшая машина впереди, потом для торможения
    };


    inline Car::Car(Lane* lane_pointer) {
        SetSpeedLimit(120);

        float default_speed = GenFloat(30, 120);
        SetSpeed(default_speed);

        float dist = GetSpeedLimit() - default_speed;
        float default_acceleration = GenFloat(0, dist);
        SetAcceleration(default_acceleration);

        if (!lane_pointer) {
            return;
        }

        const std::vector<sf::Vector2f>& waypoints = lane_pointer->GetWaypoints();

        if (waypoints.empty()) {
            return;
        }

        lane = lane_pointer;
        waypoint_i = static_cast<int>(waypoints.size()) - 1;
        Pos = waypoints[waypoint_i];

        if (waypoint_i > 0) {
            Direction = waypoints[waypoint_i - 1] - waypoints[waypoint_i];
            NormalizeDirection();
        }
    }

    inline void Car::Update(float dt) {
        if (!lane) {
            return;
        }

        const std::vector<sf::Vector2f>& waypoints = lane->GetWaypoints();

        if (waypoints.empty()) {
            lane->Car_erase(this);
            lane = nullptr;
            return;
        }
        if (waypoint_i >= static_cast<int>(waypoints.size())) {
            waypoint_i = static_cast<int>(waypoints.size()) - 1;
        }
        if (waypoint_i < 0) {
            lane->Car_erase(this);
            lane = nullptr;
            return;
        }

        if (Speed < Speed_limit) {
            Speed += Acceleration * dt;

            if (Speed > Speed_limit) {
                Speed = Speed_limit;
            }
        } else if (Speed > Speed_limit) {
            Speed -= Acceleration * dt;

            if (Speed < 0.0f) {
                Speed = 0.0f;
            }
        }

        Velocity = Direction * Km_to_Px(Speed);
        Pos += Velocity * dt;

        float waypoint_reach_px = Meters_to_Px(config::Waypoint_reach_m);

        while (waypoint_i >= 0 &&
            get_distance(waypoints[waypoint_i], Pos) <= waypoint_reach_px) {
            waypoint_i--;

            if (waypoint_i >= 0) {
                Direction = waypoints[waypoint_i] - Pos;
                NormalizeDirection();
            }
        }

        if (waypoint_i < 0) {
            alive = false;
            lane = nullptr;
            return;
        }
    }


    // Класс дороги
    class Road {
        private:
            std::vector<Lane*> Lanes;
        public:
            void AddLane(Lane* l) { Lanes.push_back(l); }
            const std::vector<Lane*>& GetLanes() const { return Lanes; }

            // одна дорога = один подъезд к перекрёстку
            // outward: запад {-1,0}, восток {1,0}, север {0,1}, юг {0,-1}
            void Build(sf::Vector2f outward, int n = 3) {
                sf::Vector2f travel = -outward;
                sf::Vector2f right = {travel.y, -travel.x};
                // Полная ширина дороги в метрах.
                float total_width_m = static_cast<float>(n) * config::Lane_width_m;
                float first_lane_center_m =
                    -total_width_m * 0.5f + config::Lane_width_m * 0.5f;

                for (int i = 0; i < n; ++i) {
                    Lane* l = new Lane();
                    float lane_center_m =
                        first_lane_center_m +
                        static_cast<float>(i) * config::Lane_width_m;

                    // Смещаем полосу вбок от оси дороги.
                    l->Build(outward, right * Meters_to_Px(lane_center_m));

                    AddLane(l);

                }
            }
            void Update(float dt) {
                for(int i = 0;i < Lanes.size();i++) {
                    Lanes[i]->Update(dt);
                }
            }
            ~Road() {
                for(int i = 0;i < Lanes.size();i++) {
                    if(Lanes[i] != nullptr) {
                        delete Lanes[i];
                    }
                }
            }
            // соседний ряд для перестроения: Lanes[i ± 1]
            Lane* Neighbor(Lane* lane, int side);
    };

    // Класс перекрёсток
    class Crossroad {
        float UL_corner, UR_corner, DL_corner, DR_corner;
    };

    // Класс мира/карты
    class World {
        private:
            Crossroad* crossroad = nullptr;
            // Дороги указывать с Северной по часовой стрелке.
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
            void Update(float dt) {
                for(int i = 0;i < roads.size();i++) {
                    roads[i]->Update(dt);
                }
            }
            const std::vector<Road*>& Get_roads() {
                return roads;
            };
            const obj::Crossroad* Get_crossroad() {
                return crossroad;
            };
            ~World() {
                for(int i = 0;i < roads.size();i++) {
                    if(roads[i] != nullptr)
                        delete roads[i];
                }
            }
    };

}
