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
    inline constexpr float Crossing_dist_m = 16.0f; // от центра до зебры, 6 рядов
    inline constexpr float Stop_dist_m = 20.0f;     // от центра до стоп-линии
    inline constexpr float Road_length_m = 44.f;    // от центра до края
    inline constexpr float Waypoint_step_m = 10.f;
    inline constexpr float Waypoint_reach_m = 1.0f; // Расстояние на котором машина считает, что достигла waypoint
    inline constexpr float Light_clearance_m = 5.0f; // от границы перекрёстка до светофора
    inline constexpr float Green_s = 12.0f;
    inline constexpr float Yellow_s = 3.0f;
    inline constexpr float Walk_s = 12.0f;
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
            // свой маршрут, если с этого ряда можно в разные стороны
            std::vector<sf::Vector2f> own_route;
            int own_stop = -1;
            int turn = 1; // 0 направо, 1 прямо, 2 налево
            Lane* my_exit = nullptr;
            // потом: соседний ряд + прогресс 0..1
            bool alive = true;
        public:
            bool IsAlive() const {
                return alive;
            }

            int Turn() const { return turn; }
            int StopAt() const { return own_stop; }

            Car() {
                SetSpeedLimit(120);

                float default_speed = GenFloat(30, 120);
                SetSpeed(default_speed);

                float dist = GetSpeedLimit() - default_speed;
                float default_acceleration = GenFloat(0, dist);
                SetAcceleration(default_acceleration);
            }

            explicit Car(Lane* lane_pointer);

            int WaypointIndex() const { return waypoint_i; }

            // если впереди машина/стоп — тормозим, иначе газуем до Speed_limit
            void Update(float dt);
    };


    // Класс пешехода, набросок
    class Pedestrian : public Entity {
            std::vector<sf::Vector2f> waypoints;
            int waypoint_i = 0;
            bool alive = true;
        public:
            bool IsAlive() const { return alive; }

            Pedestrian() {
                SetSpeedLimit(5);
                SetSpeed(5);
                SetAcceleration(8);
            }

            void SetPath(sf::Vector2f from, sf::Vector2f to) {
                waypoints.clear();
                waypoints.push_back(to);
                waypoints.push_back(from);
                waypoint_i = 1;
                Pos = from;
                Direction = to - from;
                NormalizeDirection();
            }

            void Update(float dt) {
                if (!alive || waypoints.empty() || waypoint_i < 0) {
                    alive = false;
                    return;
                }
                if (waypoint_i >= waypoints.size()) {
                    waypoint_i = waypoints.size() - 1;
                }
                Direction = waypoints[waypoint_i] - Pos;
                NormalizeDirection();
                float dist = get_distance(Pos, waypoints[waypoint_i]);
                float step = Km_to_Px(Speed) * dt;
                if (step >= dist) {
                    Pos = waypoints[waypoint_i];
                    waypoint_i--;
                    if (waypoint_i < 0) {
                        alive = false;
                    }
                } else {
                    Pos += Direction * step;
                }
            }
    };


    // Класс для статических объектов
    class Static_entity {
        sf::Vector2f Pos; //позиция на экране

    };


    enum class Signal { Red, Yellow, Green, GreenLeft, YellowLeft };

    class TrafficLight;

    // Класс линий движения
    class Lane {
        private:
            sf::Vector2f corners[4]; // 4 угла полосы для отрисовки
            
            config::direction Direction;
            /*
            После Connect маршрут лежит наоборот ходу машины:
            [0] край выезда, дальше дуга и стоп-линия подъезда, [last] край подъезда.
            Машина спавнится у last и едет к [0], поэтому waypoint_i уменьшается.
            */
            std::vector<sf::Vector2f> waypoints;
            std::vector<Car*> cars;
            sf::Vector2f outward{};
            sf::Vector2f offset{};
            int stop_i = 1;
            TrafficLight* light = nullptr;
            Lane* exit_lane = nullptr;
            std::vector<Lane*> yield_lanes;
            struct Choice {
                std::vector<sf::Vector2f> pts;
                int stop_i;
                int turn;
                Lane* exit;
            };
            std::vector<Choice> choices;
        public:
            enum class Maneuver { Left, Straight, Right };
        private:
            Maneuver maneuver = Maneuver::Straight;
        public:
            const std::vector<sf::Vector2f>& GetWaypoints() const { return waypoints; }
            const sf::Vector2f* GetCorners() const { return corners; }
            sf::Vector2f Crossing() { return waypoints[0]; }
            sf::Vector2f StopLine() { return waypoints[1]; }
            const std::vector<Car*>& GetCars() const { return cars; }
            // на краю ещё влезает машина, очередь не вылезла за дорогу
            bool HasRoom() const {
                if (waypoints.size() < 2) {
                    return false;
                }
                int spawn_i = waypoints.size() - 1;
                sf::Vector2f forward = waypoints[spawn_i - 1] - waypoints[spawn_i];
                float len = std::sqrt(forward.x * forward.x + forward.y * forward.y);
                if (len <= 0.0f) {
                    return false;
                }
                forward.x /= len;
                forward.y /= len;
                float gap = Meters_to_Px(config::Car_length_m + 2.0f);
                float edge_s = waypoints[spawn_i].x * forward.x + waypoints[spawn_i].y * forward.y;
                bool any = false;
                float tail_s = edge_s;
                for (int i = 0; i < cars.size(); ++i) {
                    Car* other = cars[i];
                    if (!other || !other->IsAlive()) {
                        continue;
                    }
                    float s = other->GetPos().x * forward.x + other->GetPos().y * forward.y;
                    if (!any || s < tail_s) {
                        tail_s = s;
                        any = true;
                    }
                }
                if (!any) {
                    return true;
                }
                return edge_s <= tail_s - gap + 0.5f;
            }
            Maneuver GetManeuver() const { return maneuver; }
            void SetManeuver(Maneuver m) { maneuver = m; }
            int StopIndex() const { return stop_i; }
            TrafficLight* GetLight() const { return light; }
            void SetLight(TrafficLight* l) { light = l; }
            Lane* GetExit() const { return exit_lane; }
            void SetExit(Lane* l) { exit_lane = l; }
            void AddYield(Lane* l) { yield_lanes.push_back(l); }
            void AddChoice(const std::vector<sf::Vector2f>& pts, int stop, int turn_kind, Lane* exit) {
                Choice c;
                c.pts = pts;
                c.stop_i = stop;
                c.turn = turn_kind;
                c.exit = exit;
                choices.push_back(c);
            }
            int ChoiceCount() const { return choices.size(); }
            const std::vector<sf::Vector2f>& ChoicePts(int i) const { return choices[i].pts; }
            int ChoiceStop(int i) const { return choices[i].stop_i; }
            int ChoiceTurn(int i) const { return choices[i].turn; }
            Lane* ChoiceExit(int i) const { return choices[i].exit; }
            sf::Vector2f GetOffset() const { return offset; }
            sf::Vector2f GetOutward() const { return outward; }
            void SetRoute(const std::vector<sf::Vector2f>& pts, int stop_index) {
                waypoints = pts;
                stop_i = stop_index;
            }
            void ReverseWaypoints() {
                std::reverse(waypoints.begin(), waypoints.end());
            }
            bool YieldNow() const;
            bool ExitBusy() const;
            bool ExitBlocked(Lane* exit) const;
            Signal CurrentSignal() const;

            void AddWaypoint(sf::Vector2f p) { waypoints.push_back(p); }

            // outward — от перекрёстка к краю, offset — сдвиг ряда вбок (метры)
            void Build(sf::Vector2f outward_dir, sf::Vector2f offset_px) {
                outward = outward_dir;
                offset = offset_px;
                waypoints.clear();
                AddWaypoint(outward * Meters_to_Px(config::Crossing_dist_m) + offset);
                AddWaypoint(outward * Meters_to_Px(config::Stop_dist_m) + offset);
                for (float d = config::Stop_dist_m + config::Waypoint_step_m; d < config::Road_length_m - 0.5f; d += config::Waypoint_step_m) {
                    AddWaypoint(outward * Meters_to_Px(d) + offset);
                }
                AddWaypoint(outward * Meters_to_Px(config::Road_length_m) + offset);
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
                for (int i = cars.size() - 1; i >= 0; --i) {
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
        own_stop = lane_pointer->StopIndex();
        if (lane_pointer->GetManeuver() == Lane::Maneuver::Right) {
            turn = 0;
        } else if (lane_pointer->GetManeuver() == Lane::Maneuver::Left) {
            turn = 2;
        }

        const std::vector<sf::Vector2f>* use = &waypoints;
        if (lane_pointer->ChoiceCount() > 0) {
            int pick = Gen(0, lane_pointer->ChoiceCount() - 1);
            own_route = lane_pointer->ChoicePts(pick);
            own_stop = lane_pointer->ChoiceStop(pick);
            turn = lane_pointer->ChoiceTurn(pick);
            my_exit = lane_pointer->ChoiceExit(pick);
            use = &own_route;
        }
        if (use->empty()) {
            return;
        }

        waypoint_i = use->size() - 1;
        Pos = (*use)[waypoint_i];

        if (waypoint_i > 0) {
            Direction = (*use)[waypoint_i - 1] - (*use)[waypoint_i];
            NormalizeDirection();
        }

        // хвост очереди — кто дальше всех от перекрёстка, новая встаёт за ним
        float gap = Meters_to_Px(config::Car_length_m + 2.0f);
        Car* tail = nullptr;
        float tail_s = 0.0f;
        const std::vector<Car*>& queue = lane_pointer->GetCars();
        for (int i = 0; i < queue.size(); ++i) {
            Car* other = queue[i];
            if (!other || !other->IsAlive()) {
                continue;
            }
            float s = other->GetPos().x * Direction.x + other->GetPos().y * Direction.y;
            if (!tail || s < tail_s) {
                tail = other;
                tail_s = s;
            }
        }
        if (tail) {
            float my_s = Pos.x * Direction.x + Pos.y * Direction.y;
            if (my_s > tail_s - gap) {
                Pos = tail->GetPos() - Direction * gap;
                SetSpeed(0.0f);
            }
        }
    }

    inline void Car::Update(float dt) {
        if (!lane) {
            return;
        }

        const std::vector<sf::Vector2f>& waypoints = own_route.empty() ? lane->GetWaypoints() : own_route;

        if (waypoints.empty() || waypoint_i < 0) {
            alive = false;
            return;
        }
        if (waypoint_i >= waypoints.size()) {
            waypoint_i = waypoints.size() - 1;
        }

        float target = Speed_limit;
        float gap = Meters_to_Px(config::Car_length_m + 2.0f);
        const std::vector<Car*>& queue = lane->GetCars();
        int my_place = -1;
        for (int i = 0; i < queue.size(); ++i) {
            if (queue[i] == this) {
                my_place = i;
                break;
            }
        }
        int stop_i = own_stop >= 0 ? own_stop : lane->StopIndex();
        int my_prog = waypoints.size() - 1 - waypoint_i;
        bool me_before = waypoint_i >= stop_i;
        Car* ahead = nullptr;
        float ahead_d = 0.0f;
        for (int i = 0; i < queue.size(); ++i) {
            Car* other = queue[i];
            if (!other || other == this || !other->IsAlive()) {
                continue;
            }
            int his_n = other->own_route.empty()
                ? lane->GetWaypoints().size()
                : other->own_route.size();
            int his_stop = other->own_stop >= 0 ? other->own_stop : lane->StopIndex();
            int his_prog = his_n - 1 - other->waypoint_i;
            bool him_before = other->waypoint_i >= his_stop;
            bool in_front = false;
            if (turn == other->turn || (me_before && him_before)) {
                if (his_prog > my_prog) {
                    in_front = true;
                } else if (his_prog == my_prog) {
                    in_front = i < my_place;
                }
            }
            if (!in_front) {
                continue;
            }
            float d = get_distance(Pos, other->Pos);
            if (!ahead || d < ahead_d) {
                ahead = other;
                ahead_d = d;
            }
        }
        if (ahead) {
            float room = ahead_d - gap;
            if (room <= 0.0f) {
                target = 0.0f;
            } else {
                float room_m = room / config::Pixels_to_meter;
                float a = 40.0f * (1000.0f / 3600.0f);
                float cap = std::sqrt(2.0f * a * room_m) * 3.6f;
                if (cap < target) {
                    target = cap;
                }
            }
        }

        bool wait = false;
        if (waypoint_i >= stop_i && lane->GetLight() != nullptr) {
            float dist_stop = get_distance(Pos, waypoints[stop_i]);
            Signal sig = lane->CurrentSignal();
            // зелёный круг: прямо и направо. зелёная стрелка: только налево
            bool go = false;
            if (turn == 2) {
                if (sig == Signal::GreenLeft) {
                    go = true;
                } else if (sig == Signal::YellowLeft && dist_stop <= Meters_to_Px(6.0f)) {
                    go = true;
                }
            } else if (sig == Signal::Green) {
                go = true;
            } else if (sig == Signal::Yellow && dist_stop <= Meters_to_Px(6.0f)) {
                go = true;
            }
            if (!go) {
                wait = true;
            }
            if (turn == 2 && lane->YieldNow()) {
                wait = true;
            }
            Lane* exit = my_exit ? my_exit : lane->GetExit();
            if (lane->ExitBlocked(exit)) {
                wait = true;
            }
        }
        if (wait) {
            float dist_stop = get_distance(Pos, waypoints[stop_i]);
            float horizon = Meters_to_Px(22.0f);
            if (dist_stop <= Meters_to_Px(1.5f)) {
                target = 0.0f;
            } else if (dist_stop < horizon) {
                float cap = Speed_limit * (dist_stop / horizon);
                if (cap < target) {
                    target = cap;
                }
            }
        }

        if (Speed < target) {
            float accel = Acceleration < 20.0f ? 20.0f : Acceleration;
            Speed += accel * dt;
            if (Speed > target) {
                Speed = target;
            }
        } else if (Speed > target) {
            float brake = Acceleration < 40.0f ? 40.0f : Acceleration;
            Speed -= brake * dt;
            if (Speed < target) {
                Speed = target;
            }
        }
        if (Speed < 0.0f) {
            Speed = 0.0f;
        }

        // каждый кадр смотрим в текущую точку, иначе после отката индекса
        // машина уезжает по старому направлению наискосок и не возвращается
        Direction = waypoints[waypoint_i] - Pos;
        NormalizeDirection();

        float step = Km_to_Px(Speed) * dt;
        if (ahead) {
            float room = ahead_d - gap;
            if (room < step) {
                step = room > 0.0f ? room : 0.0f;
                if (room <= 0.0f) {
                    Speed = 0.0f;
                }
            }
        }
        Velocity = Direction * Km_to_Px(Speed);
        Pos += Direction * step;

        float waypoint_reach_px = Meters_to_Px(config::Waypoint_reach_m);

        while (waypoint_i >= 0 &&
            get_distance(waypoints[waypoint_i], Pos) <= waypoint_reach_px) {
            if (wait && waypoint_i <= stop_i) {
                waypoint_i = stop_i;
                Speed = 0.0f;
                if (!ahead) {
                    Pos = waypoints[stop_i];
                }
                break;
            }
            // не перескакиваем точку, до которой передняя машина ещё не доехала
            if (ahead && turn == ahead->turn) {
                int ahead_n = lane->GetWaypoints().size();
                if (ahead->own_route.size() > 0) {
                    ahead_n = ahead->own_route.size();
                }
                if (ahead_n == waypoints.size() && waypoint_i - 1 < ahead->waypoint_i) {
                    break;
                }
            }
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

        if (ahead) {
            float d = get_distance(Pos, ahead->Pos);
            if (d < gap) {
                // отодвигаем назад по линии между машинами, а не по носу:
                // на повороте нос уже смотрит в сторону и выкидывает машину с дороги
                if (d > 0.0f) {
                    sf::Vector2f back = Pos - ahead->Pos;
                    Pos = ahead->Pos + back * (gap / d);
                } else {
                    Pos = ahead->Pos - Direction * gap;
                }
                Speed = 0.0f;
            }
        }
    }

    inline bool Lane::YieldNow() const {
        float limit = Meters_to_Px(config::Stop_dist_m);
        for (int i = 0; i < yield_lanes.size(); ++i) {
            Lane* other = yield_lanes[i];
            if (!other) {
                continue;
            }
            const std::vector<Car*>& cars_ahead = other->GetCars();
            for (int c = 0; c < cars_ahead.size(); ++c) {
                Car* car = cars_ahead[c];
                if (!car || !car->IsAlive()) {
                    continue;
                }
                if (car->Turn() != 1) {
                    continue;
                }
                if (car->WaypointIndex() >= car->StopAt()) {
                    continue;
                }
                sf::Vector2f p = car->GetPos();
                float dist = std::sqrt(p.x * p.x + p.y * p.y);
                if (dist < limit) {
                    return true;
                }
            }
        }
        return false;
    }

    inline bool Lane::ExitBusy() const {
        return ExitBlocked(exit_lane);
    }

    inline bool Lane::ExitBlocked(Lane* exit) const {
        if (!exit) {
            return false;
        }
        sf::Vector2f spot = exit->GetOutward() * Meters_to_Px(config::Stop_dist_m + 8.0f) + exit->GetOffset();
        float radius = Meters_to_Px(5.0f);
        const std::vector<Car*>& exit_cars = exit->GetCars();
        for (int i = 0; i < exit_cars.size(); ++i) {
            Car* car = exit_cars[i];
            if (!car || !car->IsAlive()) {
                continue;
            }
            if (get_distance(car->GetPos(), spot) < radius) {
                return true;
            }
        }
        return false;
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
            void Build(sf::Vector2f outward, int n = 6) {
                sf::Vector2f travel = -outward;
                // правая стена справа по ходу: на левой дороге это нижние ряды
                sf::Vector2f right = {-travel.y, travel.x};
                float total_width_m = (float)n * config::Lane_width_m;
                float first_lane_center_m =
                    -total_width_m * 0.5f + config::Lane_width_m * 0.5f;

                for (int i = 0; i < n; ++i) {
                    Lane* l = new Lane();
                    float lane_center_m =
                        first_lane_center_m +
                        (float)i * config::Lane_width_m;

                    l->Build(outward, right * Meters_to_Px(lane_center_m));
                    // верхняя половина левой дороги — встречка, едет влево
                    if (i >= n / 2) {
                        l->ReverseWaypoints();
                    }

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

    // Светофор стоит за углом перекрёстка, справа по ходу своего подъезда.
    class TrafficLight {
        sf::Vector2f pos;
        sf::Vector2f right; // вправо по ходу подъезда, допсекция со стрелкой
        Signal signal = Signal::Red;
    public:
        TrafficLight(sf::Vector2f p, sf::Vector2f right_dir) : pos(p), right(right_dir) {}
        sf::Vector2f GetPos() const { return pos; }
        sf::Vector2f GetRight() const { return right; }
        Signal GetSignal() const { return signal; }
        void SetSignal(Signal s) { signal = s; }
    };

    inline Signal Lane::CurrentSignal() const {
        if (!light) {
            return Signal::Green;
        }
        return light->GetSignal();
    }

    // Класс перекрёсток
    class Crossroad {
        float UL_corner, UR_corner, DL_corner, DR_corner;
    };

    // Переход через дорогу: от одного тротуара до другого
    class Crosswalk {
            sf::Vector2f from{};
            sf::Vector2f to{};
            std::vector<Pedestrian*> people;
        public:
            void Build(sf::Vector2f a, sf::Vector2f b) {
                from = a;
                to = b;
            }
            sf::Vector2f From() const { return from; }
            sf::Vector2f To() const { return to; }
            const std::vector<Pedestrian*>& People() const { return people; }

            bool Busy() const {
                for (int i = 0; i < people.size(); ++i) {
                    if (people[i] && people[i]->IsAlive()) {
                        return true;
                    }
                }
                return false;
            }

            void Spawn() {
                bool flip = Gen(0, 1) == 1;
                sf::Vector2f start = flip ? to : from;
                sf::Vector2f end = flip ? from : to;
                sf::Vector2f dir = end - start;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len < 1.0f) {
                    return;
                }
                dir.x /= len;
                dir.y /= len;
                // идут по правой стороне зебры, встречные разъезжаются
                sf::Vector2f right(dir.y, -dir.x);
                sf::Vector2f shift = right * Meters_to_Px(0.8f);
                start += shift;
                end += shift;
                float gap = Meters_to_Px(1.5f);
                for (int i = 0; i < people.size(); ++i) {
                    Pedestrian* other = people[i];
                    if (!other || !other->IsAlive()) {
                        continue;
                    }
                    if (get_distance(other->GetPos(), start) < gap) {
                        return;
                    }
                }
                Pedestrian* ped = new Pedestrian();
                ped->SetPath(start, end);
                people.push_back(ped);
            }

            void Update(float dt) {
                for (int i = people.size() - 1; i >= 0; --i) {
                    Pedestrian* ped = people[i];
                    if (!ped) {
                        people.erase(people.begin() + i);
                        continue;
                    }
                    ped->Update(dt);
                    if (!ped->IsAlive()) {
                        delete ped;
                        people.erase(people.begin() + i);
                    }
                }
            }

            void Clear() {
                for (int i = 0; i < people.size(); ++i) {
                    delete people[i];
                }
                people.clear();
            }
    };

    // Класс мира/карты
    class World {
        private:
            Crossroad* crossroad = nullptr;
            // Дороги указывать с Северной по часовой стрелке.
            std::vector<Road*> roads; // 4 подъезда: З, В, С, Ю
            std::vector<TrafficLight> lights;
            std::vector<Crosswalk> walks;
            float phase_time = 0.0f;
            float walk_spawn = 0.0f;
            int phase = 0; // на каждую ось: прямо+направо, жёлтый, налево, жёлтый, потом пешеходы

            bool CarOnWalk(const Crosswalk& walk) const {
                sf::Vector2f a = walk.From();
                sf::Vector2f b = walk.To();
                sf::Vector2f ab = b - a;
                float ab2 = ab.x * ab.x + ab.y * ab.y;
                if (ab2 < 1.0f) {
                    return false;
                }
                float band = Meters_to_Px(4.0f);
                for (int r = 0; r < roads.size(); ++r) {
                    if (!roads[r]) {
                        continue;
                    }
                    const std::vector<Lane*>& lanes = roads[r]->GetLanes();
                    for (int i = 0; i < lanes.size(); ++i) {
                        if (!lanes[i]) {
                            continue;
                        }
                        const std::vector<Car*>& cars = lanes[i]->GetCars();
                        for (int c = 0; c < cars.size(); ++c) {
                            Car* car = cars[c];
                            if (!car || !car->IsAlive()) {
                                continue;
                            }
                            sf::Vector2f p = car->GetPos() - a;
                            float t = (p.x * ab.x + p.y * ab.y) / ab2;
                            if (t < 0.0f || t > 1.0f) {
                                continue;
                            }
                            sf::Vector2f proj(a.x + ab.x * t, a.y + ab.y * t);
                            if (get_distance(car->GetPos(), proj) < band) {
                                return true;
                            }
                        }
                    }
                }
                return false;
            }

            void BuildWalks() {
                walks.clear();
                float side = Meters_to_Px(2.0f);
                for (int r = 0; r < roads.size(); ++r) {
                    const std::vector<Lane*>& lanes = roads[r]->GetLanes();
                    if (lanes.size() == 0 || lanes[0] == nullptr || lanes[lanes.size() - 1] == nullptr) {
                        continue;
                    }
                    const sf::Vector2f* first = lanes[0]->GetCorners();
                    const sf::Vector2f* last = lanes[lanes.size() - 1]->GetCorners();
                    sf::Vector2f near[4] = {first[0], first[1], last[0], last[1]};
                    sf::Vector2f edge_a = near[0];
                    sf::Vector2f edge_b = near[1];
                    float best = -1.0f;
                    for (int i = 0; i < 4; ++i) {
                        for (int j = i + 1; j < 4; ++j) {
                            float d = get_distance(near[i], near[j]);
                            if (d > best) {
                                best = d;
                                edge_a = near[i];
                                edge_b = near[j];
                            }
                        }
                    }
                    sf::Vector2f mid{(edge_a.x + edge_b.x) * 0.5f, (edge_a.y + edge_b.y) * 0.5f};
                    float near_dist = std::sqrt(mid.x * mid.x + mid.y * mid.y);
                    float half = best * 0.5f;
                    float depth = near_dist - half;
                    if (near_dist < 1.0f || depth < 1.0f) {
                        continue;
                    }
                    sf::Vector2f inward{-mid.x / near_dist, -mid.y / near_dist};
                    sf::Vector2f line(mid.x + inward.x * depth * 0.5f, mid.y + inward.y * depth * 0.5f);
                    sf::Vector2f across(edge_b.x - edge_a.x, edge_b.y - edge_a.y);
                    float alen = std::sqrt(across.x * across.x + across.y * across.y);
                    across.x /= alen;
                    across.y /= alen;
                    sf::Vector2f A(line.x - across.x * (half + side), line.y - across.y * (half + side));
                    sf::Vector2f B(line.x + across.x * (half + side), line.y + across.y * (half + side));
                    Crosswalk walk;
                    walk.Build(A, B);
                    walks.push_back(walk);
                }
            }

            void ApplySignals() {
                Signal ns = Signal::Red;
                Signal ew = Signal::Red;
                if (phase >= 8) {
                    ns = Signal::Red;
                    ew = Signal::Red;
                } else {
                    int step = phase % 4;
                    Signal on = Signal::Red;
                    if (step == 0) {
                        on = Signal::Green;
                    } else if (step == 1) {
                        on = Signal::Yellow;
                    } else if (step == 2) {
                        on = Signal::GreenLeft;
                    } else {
                        on = Signal::YellowLeft;
                    }
                    if (phase < 4) {
                        ns = on;
                    } else {
                        ew = on;
                    }
                }
                if (lights.size() != 4) {
                    return;
                }
                lights[0].SetSignal(ew);
                lights[1].SetSignal(ew);
                lights[2].SetSignal(ns);
                lights[3].SetSignal(ns);
            }

            void FillRoute(const std::vector<std::vector<std::vector<sf::Vector2f>>>& saved,
                           Lane* lane, int r, int i, int dest_r, int dest_i, Lane::Maneuver maneuver,
                           std::vector<sf::Vector2f>& route, int& stop_index, Lane*& exit_lane) {
                exit_lane = roads[dest_r]->GetLanes()[dest_i];
                const std::vector<sf::Vector2f>& approach = saved[r][i];
                const std::vector<sf::Vector2f>& exit_pts = saved[dest_r][dest_i];
                int approach_n = approach.size();
                std::vector<sf::Vector2f> travel;
                for (int p = approach_n - 1; p >= 0; p--) {
                    travel.push_back(approach[p]);
                }
                int stop_travel = approach_n - 2;
                sf::Vector2f A = approach[0];
                if (maneuver == Lane::Maneuver::Straight) {
                    // встречка развёрнута: едем к дальнему краю и там пропадаем
                    sf::Vector2f B = exit_pts[exit_pts.size() - 1];
                    travel.push_back((A + B) * 0.5f);
                    for (int p = exit_pts.size() - 1; p >= 0; p--) {
                        travel.push_back(exit_pts[p]);
                    }
                } else {
                    // нос по ходу выезда, от центра наружу
                    sf::Vector2f exit_dir = exit_lane->GetOutward();
                    float inner = Meters_to_Px(7.0f);
                    sf::Vector2f enter = lane->GetOffset() + lane->GetOutward() * inner;
                    sf::Vector2f leave = exit_lane->GetOffset() + exit_dir * inner;
                    sf::Vector2f corner = lane->GetOffset() + exit_lane->GetOffset();
                    travel.push_back(enter);
                    for (int s = 1; s <= 3; s++) {
                        float t = (float)s / 4.0f;
                        float u = 1.0f - t;
                        travel.push_back(enter * (u * u) + corner * (2.0f * u * t) + leave * (t * t));
                    }
                    travel.push_back(leave);
                    float edge = Meters_to_Px(config::Road_length_m);
                    float step = Meters_to_Px(config::Waypoint_step_m);
                    for (float dist = inner + step; dist < edge; dist += step) {
                        travel.push_back(exit_lane->GetOffset() + exit_dir * dist);
                    }
                    travel.push_back(exit_lane->GetOffset() + exit_dir * edge);
                }
                route.resize(travel.size());
                for (int p = 0; p < travel.size(); p++) {
                    route[travel.size() - 1 - p] = travel[p];
                }
                stop_index = travel.size() - 1 - stop_travel;
            }

            void Connect() {
                if (roads.size() != 4) {
                    return;
                }
                int right_of[4] = {3, 2, 0, 1};
                int left_of[4] = {2, 3, 1, 0};
                int opposite[4] = {1, 0, 3, 2};
                int n = roads[0]->GetLanes().size();

                std::vector<std::vector<std::vector<sf::Vector2f>>> saved(4);
                for (int r = 0; r < 4; ++r) {
                    const std::vector<Lane*>& lanes = roads[r]->GetLanes();
                    for (int i = 0; i < n; ++i) {
                        saved[r].push_back(lanes[i]->GetWaypoints());
                    }
                }

                for (int r = 0; r < 4; ++r) {
                    const std::vector<Lane*>& lanes = roads[r]->GetLanes();
                    for (int i = 0; i < n; ++i) {
                        Lane* lane = lanes[i];
                        // верхние ряды — встречка, свой маршрут уже развёрнут
                        if (i >= n / 2) {
                            continue;
                        }
                        // ближний светофор справа, вдоль правой стены
                        lane->SetLight(&lights[r]);

                        std::vector<sf::Vector2f> route;
                        int stop_index = 0;
                        Lane* exit_lane = nullptr;

                        // у центра: только налево. прямо — соседний ряд, направо — крайний
                        if (i == n / 2 - 1) {
                            FillRoute(saved, lane, r, i, left_of[r], n / 2, Lane::Maneuver::Left, route, stop_index, exit_lane);
                            lane->SetManeuver(Lane::Maneuver::Left);
                            lane->SetExit(exit_lane);
                            lane->SetRoute(route, stop_index);

                            const std::vector<Lane*>& oncoming = roads[opposite[r]]->GetLanes();
                            for (int k = 1; k <= n - 2; ++k) {
                                lane->AddYield(oncoming[k]);
                            }
                        } else if (i == 0) {
                            // правая стена: только направо, на левой дороге это вниз
                            FillRoute(saved, lane, r, i, right_of[r], n - 1, Lane::Maneuver::Right, route, stop_index, exit_lane);
                            lane->SetManeuver(Lane::Maneuver::Right);
                            lane->SetExit(exit_lane);
                            lane->SetRoute(route, stop_index);
                        } else {
                            FillRoute(saved, lane, r, i, opposite[r], n - 1 - i, Lane::Maneuver::Straight, route, stop_index, exit_lane);
                            lane->SetManeuver(Lane::Maneuver::Straight);
                            lane->SetExit(exit_lane);
                            lane->SetRoute(route, stop_index);
                        }
                    }
                }
            }
        public:
            void Build() {
                Road* west = new Road();
                west->Build(sf::Vector2f(-1.f, 0.f));
                roads.push_back(west);
                Road* east = new Road();
                east->Build(sf::Vector2f(1.f, 0.f));
                roads.push_back(east);
                Road* north = new Road();
                north->Build(sf::Vector2f(0.f, 1.f));
                roads.push_back(north);
                Road* south = new Road();
                south->Build(sf::Vector2f(0.f, -1.f));
                roads.push_back(south);

                float half = (float)roads[0]->GetLanes().size() * config::Lane_width_m * 0.5f;
                float d = Meters_to_Px(half + config::Light_clearance_m);
                lights.push_back(TrafficLight(sf::Vector2f(-d, -d), sf::Vector2f(0.f, -1.f)));
                lights.push_back(TrafficLight(sf::Vector2f(d, d), sf::Vector2f(0.f, 1.f)));
                lights.push_back(TrafficLight(sf::Vector2f(-d, d), sf::Vector2f(-1.f, 0.f)));
                lights.push_back(TrafficLight(sf::Vector2f(d, -d), sf::Vector2f(1.f, 0.f)));
                ApplySignals();
                Connect();
                BuildWalks();
            }
            void Update(float dt) {
                for(int i = 0;i < roads.size();i++) {
                    roads[i]->Update(dt);
                }
                for (int i = 0; i < walks.size(); ++i) {
                    walks[i].Update(dt);
                }
                if (phase == 8) {
                    phase_time += dt;
                    if (phase_time < config::Walk_s) {
                        walk_spawn += dt;
                        if (walk_spawn >= 1.2f && walks.size() > 0) {
                            walk_spawn = 0.0f;
                            int r = Gen(0, walks.size() - 1);
                            if (!CarOnWalk(walks[r])) {
                                walks[r].Spawn();
                            }
                        }
                    }
                    bool busy = false;
                    for (int i = 0; i < walks.size(); ++i) {
                        if (walks[i].Busy()) {
                            busy = true;
                        }
                    }
                    if (phase_time >= config::Walk_s && !busy) {
                        phase_time = 0.0f;
                        walk_spawn = 0.0f;
                        phase = 0;
                        ApplySignals();
                    }
                    return;
                }
                phase_time += dt;
                float limit = (phase % 2 == 0) ? config::Green_s : config::Yellow_s;
                if (phase_time >= limit) {
                    phase_time = 0.0f;
                    phase = (phase + 1) % 9;
                    walk_spawn = 0.0f;
                    if (phase == 8) {
                        for (int i = 0; i < walks.size(); ++i) {
                            if (!CarOnWalk(walks[i])) {
                                walks[i].Spawn();
                            }
                        }
                    }
                    ApplySignals();
                }
            }
            bool WalkNow() const { return phase == 8; }
            const std::vector<Crosswalk>& GetWalks() const { return walks; }
            const std::vector<Road*>& Get_roads() {
                return roads;
            };
            const std::vector<TrafficLight>& Get_lights() const {
                return lights;
            };
            const obj::Crossroad* Get_crossroad() {
                return crossroad;
            };
            ~World() {
                for (int i = 0; i < walks.size(); ++i) {
                    walks[i].Clear();
                }
                for(int i = 0;i < roads.size();i++) {
                    if(roads[i] != nullptr)
                        delete roads[i];
                }
            }
    };

}

