#include "app.h"
#include <cstddef>



void app::run() {
    sf::RenderWindow window(sf::VideoMode({app::SCREEN_WIDTH, app::SCREEN_HEIGHT}), "App");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window)) {
        return; 
    }

    obj::World World;
    World.Build();
    sf::Clock deltaClock; 

    background_manager Back;
    Back.Init_textures();
    sf::Texture tttxxxttt;
    tttxxxttt.loadFromFile("../assets/goose.jpg");
    sf::Sprite Background(tttxxxttt);
    
    Back.Get_background(Background);
    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            ImGui::SFML::ProcessEvent(window, *event);
        }

        sf::Time dt = deltaClock.restart();
        ImGui::SFML::Update(window, dt);


        World.Update(dt.asSeconds());

        ImGui::SetNextWindowSize({390, 590}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Simulation controls", nullptr, ImGuiWindowFlags_NoMove);
            int signal_mode = World.AutomaticSignals() ? 1 : 0;
            if (ImGui::Combo("Signal mode", &signal_mode,
                    "Static preset\0Automatic\0")) {
                World.SetAutomaticSignals(signal_mode == 1);
            }
            ImGui::Separator();
            if (!World.AutomaticSignals()) {
                ImGui::Text("Static signal timing");
                ImGui::SliderFloat(
                    "Green duration (s)", &config::Green_s, 1.0f, 90.0f, "%.1f");
                ImGui::SliderFloat(
                    "Yellow duration (s)", &config::Yellow_s, 1.0f, 15.0f, "%.1f");
                ImGui::SliderFloat(
                    "Red / walk duration (s)", &config::Static_red_s,
                    1.0f, 90.0f, "%.1f");
            } else {
                ImGui::TextWrapped(
                    "Static color durations apply only in Static preset mode.");
            }

            ImGui::Separator();
            ImGui::Text("Traffic parameters");
            ImGui::SliderFloat(
                "Signal visibility (m)", &config::Signal_visibility_m,
                5.0f, 100.0f, "%.1f");

            float min_speed = config::Vehicle_speed_min_kmh;
            if (ImGui::SliderFloat(
                    "Minimum car speed (km/h)", &min_speed,
                    5.0f, 160.0f, "%.0f")) {
                config::Vehicle_speed_min_kmh = min_speed;
                if (config::Vehicle_speed_max_kmh < min_speed) {
                    config::Vehicle_speed_max_kmh = min_speed;
                }
            }
            ImGui::SliderFloat(
                "Maximum car speed (km/h)", &config::Vehicle_speed_max_kmh,
                config::Vehicle_speed_min_kmh, 160.0f, "%.0f");

            float min_arrival = config::Car_arrival_min_s;
            if (ImGui::SliderFloat(
                    "Minimum spawn interval (s)", &min_arrival,
                    1.0f, 30.0f, "%.1f")) {
                config::Car_arrival_min_s = min_arrival;
                if (config::Car_arrival_max_s < min_arrival) {
                    config::Car_arrival_max_s = min_arrival;
                }
            }
            ImGui::SliderFloat(
                "Maximum spawn interval (s)", &config::Car_arrival_max_s,
                config::Car_arrival_min_s, 60.0f, "%.1f");
            ImGui::TextWrapped(
                "Each randomized arrival cycle adds one car to every road.");

            ImGui::Separator();
            ImGui::Text("Vehicles in active queue: %d",
                World.WaitingVehiclesForActivePhase());
            if (World.AutomaticSignals()) {
                ImGui::Text("Pedestrians waiting: %d",
                    World.WaitingPedestrians());
            }
            if (ImGui::Button("Add car")) {
                const std::vector<obj::Road*>& roads = World.Get_roads();
                if (roads.size() > 0) {
                    obj::Road* road = roads[Gen(0, roads.size() - 1)];
                    if (road != nullptr) {
                        const std::vector<obj::Lane*>& lanes = road->GetLanes();
                        int inbound = lanes.size() / 2;
                        if (inbound > 0) {
                            int lane_start = Gen(0, inbound - 1);
                            for (int j = 0; j < inbound; ++j) {
                                obj::Lane* lane = lanes[(lane_start + j) % inbound];
                                if (lane == nullptr || !lane->HasRoom()) {
                                    continue;
                                }
                                lane->Car_push(new obj::Car(lane));
                                break;
                            }
                        }
                    }
                }
            }
            if (ImGui::Button("Next background")) {
                Back.Next_background(Background);

            }
        ImGui::End();


        app::Render_world(window, World, Background);

        ImGui::SFML::Render(window);
        window.resetGLStates();
        window.display();
    }

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
}

void Render_sidewalk(sf::RenderWindow& window, const obj::Road* road) {
    const std::vector<obj::Lane*>& lanes = road->GetLanes();
    if (lanes.size() < 2 || lanes[0] == nullptr || lanes[lanes.size() - 1] == nullptr) {
        return;
    }
    const sf::Vector2f* a = lanes[0]->GetCorners();
    const sf::Vector2f* b = lanes[lanes.size() - 1]->GetCorners();
    sf::Vector2f ma = (a[0] + a[1] + a[2] + a[3]) * 0.25f;
    sf::Vector2f mb = (b[0] + b[1] + b[2] + b[3]) * 0.25f;
    sf::Vector2f across = mb - ma;
    float len = std::sqrt(across.x * across.x + across.y * across.y);
    if (len < 1.0f) {
        return;
    }
    across.x /= len;
    across.y /= len;
    sf::Vector2f center((ma.x + mb.x) * 0.5f, (ma.y + mb.y) * 0.5f);
    float w = obj::Meters_to_Px(2.0f);

    sf::Vector2f away(-across.x, -across.y);
    const sf::Vector2f* c = a;
    for (int side = 0; side < 2; side++) {
        if (side == 1) {
            away = across;
            c = b;
        }
        sf::Vector2f e0((c[0].x + c[3].x) * 0.5f, (c[0].y + c[3].y) * 0.5f);
        sf::Vector2f e1((c[1].x + c[2].x) * 0.5f, (c[1].y + c[2].y) * 0.5f);
        float d0 = (e0.x - center.x) * away.x + (e0.y - center.y) * away.y;
        float d1 = (e1.x - center.x) * away.x + (e1.y - center.y) * away.y;
        sf::Vector2f n = c[1];
        sf::Vector2f f = c[2];
        if (d0 > d1) {
            n = c[0];
            f = c[3];
        }
        sf::ConvexShape shape;
        shape.setPointCount(4);
        shape.setPoint(0, app::WorldToScreen(n));
        shape.setPoint(1, app::WorldToScreen(f));
        shape.setPoint(2, app::WorldToScreen(f + away * w));
        shape.setPoint(3, app::WorldToScreen(n + away * w));
        shape.setFillColor(colors::Sidewalk);
        window.draw(shape);
    }
}

void app::Render_road(sf::RenderWindow& window, const obj::Road* road) {
    if(!road) {
        return;
    }
    Render_sidewalk(window, road);
    const std::vector<obj::Lane*>&  lanes = road->GetLanes();
    for(int i = 0;i < lanes.size();i++) {
        Render_lane(window, lanes[i]);
    }
    
}

void app::Render_pedestrian(sf::RenderWindow& window, const obj::Pedestrian* ped) {
    if (!ped || !ped->IsAlive()) {
        return;
    }
    sf::CircleShape body(5.0f);
    body.setOrigin({5.0f, 5.0f});
    body.setPosition(app::WorldToScreen(ped->GetPos()));
    body.setFillColor(sf::Color(245, 245, 245));
    window.draw(body);
}

bool RoadNearEdges(const obj::Road* road, sf::Vector2f& edge_a, sf::Vector2f& edge_b) {
    if (!road) {
        return false;
    }
    const std::vector<obj::Lane*>& lanes = road->GetLanes();
    if (lanes.size() == 0 || lanes[0] == nullptr || lanes[lanes.size() - 1] == nullptr) {
        return false;
    }
    const sf::Vector2f* first = lanes[0]->GetCorners();
    const sf::Vector2f* last = lanes[lanes.size() - 1]->GetCorners();
    sf::Vector2f near[4] = {first[0], first[1], last[0], last[1]};
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
    return best > 1.0f;
}

void app::Render_crossing(sf::RenderWindow& window, const obj::Road* road) {
    sf::Vector2f near_left;
    sf::Vector2f near_right;
    if (!RoadNearEdges(road, near_left, near_right)) {
        return;
    }

    sf::Vector2f across = near_right - near_left;
    float road_width = std::sqrt(across.x * across.x + across.y * across.y);
    if (road_width < 1.0f) {
        return;
    }

    sf::Vector2f near_center = (near_left + near_right) * 0.5f;
    float near_dist = std::sqrt(near_center.x * near_center.x + near_center.y * near_center.y);
    float half = road_width * 0.5f;
    float depth = near_dist - half;
    if (near_dist < 1.0f || depth <= 1.0f) {
        return;
    }

    sf::Vector2f outward = near_center / near_dist;
    sf::Vector2f inner_left = near_left - outward * depth;

    sf::Vector2f s_near_left = app::WorldToScreen(near_left);
    sf::Vector2f s_near_right = app::WorldToScreen(near_right);
    sf::Vector2f s_inner_left = app::WorldToScreen(inner_left);

    bool along_x = std::abs(s_near_right.x - s_near_left.x) >= std::abs(s_near_right.y - s_near_left.y);
    float span_a = along_x ? s_near_left.x : s_near_left.y;
    float span_b = along_x ? s_near_right.x : s_near_right.y;
    float depth_a = along_x ? s_near_left.y : s_near_left.x;
    float depth_b = along_x ? s_inner_left.y : s_inner_left.x;

    int span_min = (int)std::round(std::min(span_a, span_b));
    int span_max = (int)std::round(std::max(span_a, span_b));
    int depth_min = (int)std::round(std::min(depth_a, depth_b));
    int depth_max = (int)std::round(std::max(depth_a, depth_b));
    int span_i = span_max - span_min;
    int depth_i = depth_max - depth_min;
    if (span_i < 2 || depth_i < 2) {
        return;
    }

    sf::RectangleShape pad;
    if (along_x) {
        pad.setPosition({(float)(span_min), (float)(depth_min)});
        pad.setSize({(float)(span_i), (float)(depth_i)});
    } else {
        pad.setPosition({(float)(depth_min), (float)(span_min)});
        pad.setSize({(float)(depth_i), (float)(span_i)});
    }
    pad.setFillColor(colors::Asphalt);
    window.draw(pad);

    const int stripe_w = 4;
    const int gap_w = 4;
    const int inset = 2;
    int stripe_n = std::max(1, (span_i + gap_w) / (stripe_w + gap_w));
    int used = stripe_n * stripe_w + (stripe_n - 1) * gap_w;
    while (stripe_n > 1 && used > span_i) {
        --stripe_n;
        used = stripe_n * stripe_w + (stripe_n - 1) * gap_w;
    }
    int margin = (span_i - used) / 2;
    int bar_min = depth_min + inset;
    int bar_len = depth_i - inset * 2;
    if (bar_len < 1) {
        bar_min = depth_min;
        bar_len = depth_i;
    }

    for (int i = 0; i < stripe_n; ++i) {
        int at = span_min + margin + i * (stripe_w + gap_w);
        sf::RectangleShape stripe;
        if (along_x) {
            stripe.setPosition({(float)(at), (float)(bar_min)});
            stripe.setSize({(float)(stripe_w), (float)(bar_len)});
        } else {
            stripe.setPosition({(float)(bar_min), (float)(at)});
            stripe.setSize({(float)(bar_len), (float)(stripe_w)});
        }
        stripe.setFillColor(colors::Sep_line);
        window.draw(stripe);
    }
}


void app::Render_light(sf::RenderWindow& window, const obj::TrafficLight& light) {
    sf::Vector2f p = app::WorldToScreen(light.GetPos());

    sf::RectangleShape body({14.0f, 36.0f});
    body.setOrigin({7.0f, 18.0f});
    body.setPosition(p);
    body.setFillColor(colors::Light_body);
    window.draw(body);

    sf::Color on[3] = {colors::Lamp_red, colors::Lamp_yellow, colors::Lamp_green};
    sf::Color off[3] = {colors::Lamp_red_off, colors::Lamp_yellow_off, colors::Lamp_green_off};
    int active = 0;
    if (light.GetSignal() == obj::Signal::Yellow) {
        active = 1;
    } else if (light.GetSignal() == obj::Signal::Green) {
        active = 2;
    }

    for (int i = 0; i < 3; ++i) {
        sf::CircleShape lamp(4.0f);
        lamp.setOrigin({4.0f, 4.0f});
        lamp.setPosition({p.x, p.y - 10.0f + (float)(i) * 10.0f});
        lamp.setFillColor(i == active ? on[i] : off[i]);
        window.draw(lamp);
    }

    sf::Vector2f right = light.GetRight();
    sf::Vector2f side(right.x, -right.y);
    // корпус 14x36, секция 14x14. отступ считаем по той стороне, куда смотрит стрелка
    float body_half = 7.0f;
    if (std::abs(side.y) > std::abs(side.x)) {
        body_half = 18.0f;
    }
    float section_half = 7.0f;
    sf::Vector2f section = p + side * (body_half + 4.0f + section_half);
    // допсекция у нижнего сигнала, если она сбоку, а не сверху/снизу
    if (std::abs(side.x) > std::abs(side.y)) {
        section.y = p.y + 10.0f;
    }

    sf::RectangleShape arrow_body({14.0f, 14.0f});
    arrow_body.setOrigin({section_half, section_half});
    arrow_body.setPosition(section);
    arrow_body.setFillColor(colors::Light_body);
    window.draw(arrow_body);

    sf::Vector2f perp{-side.y, side.x};
    sf::ConvexShape arrow;
    arrow.setPointCount(3);
    arrow.setPoint(0, section + side * 4.0f);
    arrow.setPoint(1, section - side * 3.0f + perp * 3.0f);
    arrow.setPoint(2, section - side * 3.0f - perp * 3.0f);
    sf::Color arrow_color = colors::Lamp_green_off;
    if (light.GetSignal() == obj::Signal::GreenLeft) {
        arrow_color = colors::Lamp_green;
    } else if (light.GetSignal() == obj::Signal::YellowLeft) {
        arrow_color = colors::Lamp_yellow;
    }
    arrow.setFillColor(arrow_color);
    window.draw(arrow);
}

void app::Render_crossroad(sf::RenderWindow& window,const obj::Crossroad* crossroad) {
    if(crossroad == nullptr)
        return;
}


void app::Render_world(sf::RenderWindow& window, obj::World& world, sf::Sprite& Backgorund) {
    window.clear(colors::Background);
    window.draw(Backgorund);

    sf::RectangleShape rect;
    rect.setPosition({470, 272});
    rect.setSize({735 - 475, 527 - 272});
    rect.setFillColor(colors::Background);
    window.draw(rect);

    const std::vector<obj::Road*>& roads = world.Get_roads();
    for (int i = 0; i < roads.size(); ++i) {
        Render_crossing(window, roads[i]);
    }
    //window.draw(Backgorund);

    sf::Vector2f box_a;
    sf::Vector2f box_b;
    if (roads.size() > 0 && RoadNearEdges(roads[0], box_a, box_b)) {
        sf::Vector2f across = box_b - box_a;
        float road_width = std::sqrt(across.x * across.x + across.y * across.y);
        float half = road_width * 0.5f;

        sf::RectangleShape box({road_width, road_width});
        box.setOrigin({half, half});
        box.setPosition(app::WorldToScreen({0.0f, 0.0f}));
        box.setFillColor(colors::Asphalt);
        box.setOutlineColor(colors::Sep_line);
        box.setOutlineThickness(1.0f);
        window.draw(box);
    }

    const obj::Crossroad* crossroad = world.Get_crossroad();
    Render_crossroad(window, crossroad);
    for(int i = 0;i < roads.size();i++) {
        Render_road(window, roads[i]);
    }
    for (int i = 0; i < roads.size(); ++i) {
        if (!roads[i]) {
            continue;
        }
        const std::vector<obj::Lane*>& lanes = roads[i]->GetLanes();
        for (int j = 0; j < lanes.size(); ++j) {
            if (!lanes[j]) {
                continue;
            }
            const std::vector<obj::Car*>& cars = lanes[j]->GetCars();
            for (int c = 0; c < cars.size(); ++c) {
                Render_car(window, cars[c]);
            }
        }
    }
    const std::vector<obj::TrafficLight>& lights = world.Get_lights();
    for (int i = 0; i < lights.size(); ++i) {
        Render_light(window, lights[i]);
    }

    const std::vector<obj::Crosswalk>& walks = world.GetWalks();
    sf::Color walk_lamp = world.WalkNow() ? colors::Lamp_green : colors::Lamp_red;
    for (int i = 0; i < walks.size(); ++i) {
        sf::CircleShape lamp(4.0f);
        lamp.setOrigin({4.0f, 4.0f});
        lamp.setFillColor(walk_lamp);
        lamp.setPosition(app::WorldToScreen(walks[i].From()));
        window.draw(lamp);
        lamp.setPosition(app::WorldToScreen(walks[i].To()));
        window.draw(lamp);

        const std::vector<obj::Pedestrian*>& people = walks[i].People();
        for (int p = 0; p < people.size(); ++p) {
            Render_pedestrian(window, people[p]);
        }
    }
}