#include <iostream>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define log_var(x) std::cout << #x << " = " << (x) << std::endl

static inline constexpr double EPSILON = 0.0001;

class Player
{
public:
    Player(uint x, uint y)
        : m_size(20),
          m_pos(x + m_size / 2., y + m_size / 2.),
          m_vel(0, 0),
          m_acc(0, 0),
          m_changed(true),
          m_acc_speed(0.6f),
          m_rotation(0.0f),
          m_rotation_speed(.1f)
    {
        m_varray = sf::VertexArray(sf::PrimitiveType::LineStrip, 5);
    }

    void update()
    {
        m_vel += m_acc;
        m_pos += m_vel;

        // inertia dampening
        m_vel *= 0.90f;
        m_acc = { 0, 0 };

        m_changed = true;

        update_varray();
    }

    void inform_about_mouse_pos(float x, float y) { m_mouse_pos = { x, y }; }

    enum class Direction
    {
        Forward,
        Backward,
        Left,
        Right
    };

    void accelerate_in_direction(Direction dir)
    {
        // m_acc += glm::normalize(dir) * m_acc_speed;

        // player forward directional vector (towards mouse)
        // FIXME: m_mouse_pos is screen-relative, not world-relative
        // glm::vec2 forward = glm::normalize(m_mouse_pos - m_pos);
        glm::vec2 forward = glm::normalize((m_pos + glm::vec2 { 0, -1 }) - m_pos);
        forward           = glm::rotate(forward, m_rotation);
        switch (dir)
        {
        case Direction::Forward:
            m_acc += forward * m_acc_speed;
            break;
        case Direction::Backward:
            m_acc += (-forward) * m_acc_speed;
            break;
        case Direction::Left:
            m_rotation -= m_rotation_speed;
            break;
        case Direction::Right:
            m_rotation += m_rotation_speed;
            break;
        }
    }

    sf::VertexArray varray() { return m_varray; }

protected:
    void update_varray()
    {
        if (m_changed)
        {
            glm::vec2 mid          = m_pos + glm::vec2(m_size / 2., m_size / 2.);
            glm::vec2 top          = mid + glm::rotate(glm::vec2(0.0 * m_size, -0.5 * m_size), m_rotation);
            glm::vec2 bottom_right = mid + glm::rotate(glm::vec2(0.4 * m_size, 0.5 * m_size), m_rotation);
            glm::vec2 bottom_mid   = mid + glm::rotate(glm::vec2(0.0 * m_size, 0.2 * m_size), m_rotation);
            glm::vec2 bottom_left  = mid + glm::rotate(glm::vec2(-0.4 * m_size, 0.5 * m_size), m_rotation);

            m_varray[0] = sf::Vertex(sf::Vector2f(top.x, top.y), sf::Color::White);
            m_varray[1] = sf::Vertex(sf::Vector2f(bottom_right.x, bottom_right.y), sf::Color::White);
            m_varray[2] = sf::Vertex(sf::Vector2f(bottom_mid.x, bottom_mid.y), sf::Color::White);
            m_varray[3] = sf::Vertex(sf::Vector2f(bottom_left.x, bottom_left.y), sf::Color::White);
            m_varray[4] = sf::Vertex(sf::Vector2f(top.x, top.y), sf::Color::White);
        }
    }

    float           m_size;
    glm::vec2       m_pos;
    glm::vec2       m_vel;
    glm::vec2       m_acc;
    sf::VertexArray m_varray;
    bool            m_changed;
    float           m_acc_speed;
    float           m_rotation;
    glm::vec2       m_mouse_pos;
    float           m_rotation_speed;
};

class GameWindow : public sf::RenderWindow
{
public:
    GameWindow(uint w, uint h, const char* title) : sf::RenderWindow(sf::VideoMode(w, h), title)
    {
        this->setFramerateLimit(30);
    }
    ~GameWindow() {}

    auto width() const { return getSize().x; }
    auto height() const { return getSize().y; }
};

template<typename _Callback_t>
void handle_key_press(sf::Keyboard::Key key, _Callback_t fnc)
{
    if (sf::Keyboard::isKeyPressed(key))
    {
        fnc();
    }
}

int main(int, char**)
{
    GameWindow window(640, 480, "asteroids");
    sf::Event  event;

    Player player(window.width() / 2, window.height() / 2);

    while (window.isOpen())
    {
        window.clear();

        handle_key_press(sf::Keyboard::W, [&]() { player.accelerate_in_direction(Player::Direction::Forward); });
        handle_key_press(sf::Keyboard::A, [&]() { player.accelerate_in_direction(Player::Direction::Left); });
        handle_key_press(sf::Keyboard::S, [&]() { player.accelerate_in_direction(Player::Direction::Backward); });
        handle_key_press(sf::Keyboard::D, [&]() { player.accelerate_in_direction(Player::Direction::Right); });
        handle_key_press(sf::Keyboard::Escape, [&]() { window.close(); });

        player.update();

        window.draw(player.varray());

        window.display();

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseMoved)
                player.inform_about_mouse_pos(event.mouseMove.x, event.mouseMove.y);
        }
    }

    return 0;
}

