#include <iostream>
#include <deque>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define log_var(x) std::cout << #x << " = " << (x) << std::endl

static inline constexpr double EPSILON = 0.0001;

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

class PhysicsObject
{
public:
    PhysicsObject(glm::vec2 pos, glm::vec2 vel, glm::vec2 acc, float size)
        : m_pos(pos), m_vel(vel), m_acc(acc), m_size(size)
    {
    }

    void physics_update()
    {
        m_vel += m_acc;
        m_pos += m_vel;

        m_acc = { 0, 0 };
    }

    void keep_in_bounds(GameWindow* window)
    {
        glm::vec2 mid = m_pos + glm::vec2(m_size / 2., m_size / 2.);
        if (mid.x > window->width())
        {
            m_pos.x = 0 - m_size / 2.;
        }
        if (mid.y > window->height())
        {
            m_pos.y = 0 - m_size / 2.;
        }

        if (mid.x < 0)
        {
            m_pos.x = window->width() - m_size / 2.;
        }
        if (mid.y < 0)
        {
            m_pos.y = window->height() - m_size / 2.;
        }
    }

protected:
    glm::vec2 m_pos;
    glm::vec2 m_vel;
    glm::vec2 m_acc;
    float     m_size;
};

class Bullet : public PhysicsObject
{
public:
    static constexpr float BULLET_SPEED = 10.0f;

    Bullet(const glm::vec2& pos, float dir, const glm::vec2& ship_vel)
        : PhysicsObject(pos,
                        (0.3f * ship_vel) +
                            glm::rotate(glm::normalize((m_pos + glm::vec2(0, -1)) - m_pos) * BULLET_SPEED, dir),
                        { 0, 0 }, 4.0f)
    {
    }

    sf::VertexArray varray()
    {
        sf::VertexArray varr(sf::PrimitiveType::LineStrip, 4);
        varr[0] = sf::Vertex(sf::Vector2f(m_pos.x + 0.0 * m_size, m_pos.y - 0.5 * m_size), sf::Color::White);
        varr[1] = sf::Vertex(sf::Vector2f(m_pos.x - 0.5 * m_size, m_pos.y + 0.5 * m_size), sf::Color::White);
        varr[2] = sf::Vertex(sf::Vector2f(m_pos.x + 0.5 * m_size, m_pos.y + 0.5 * m_size), sf::Color::White);
        varr[3] = sf::Vertex(sf::Vector2f(m_pos.x + 0.0 * m_size, m_pos.y - 0.5 * m_size), sf::Color::White);
        return varr;
    }
};

class Player : public PhysicsObject
{
public:
    Player(uint x, uint y)
        : PhysicsObject({ x, y }, { 0, 0 }, { 0, 0 }, 20.0f),
          m_changed(true),
          m_acc_speed(0.6f),
          m_rotation(0.0f),
          m_rotation_speed(.1f)
    {
        m_varray = sf::VertexArray(sf::PrimitiveType::LineStrip, 5);
    }

    void update()
    {
        m_changed = true;

        update_varray();
        for (Bullet& bullet : m_bullets)
        {
            bullet.physics_update();
        }

        // inertia dampening
        m_vel *= 0.90f;
    }

    void inform_about_mouse_pos(float x, float y) { m_mouse_pos = { x, y }; }


    void shoot()
    {
        //                     max bullets
        if (m_bullets.size() > 10)
        {
            m_bullets.pop_front();
        }

        m_bullets.push_back(Bullet(m_pos + glm::vec2(m_size / 2.0, m_size / 2.0), m_rotation, m_vel));
    }

    void draw_bullets(GameWindow* window)
    {
        for (Bullet& bullet : m_bullets)
        {
            window->draw(bullet.varray());
        }
    }

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

    const glm::vec2& position() const { return m_pos; }
    void             set_position(const glm::vec2& pos) { m_pos = pos; }

    std::deque<Bullet>& bullets() { return m_bullets; }

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

    sf::VertexArray    m_varray;
    bool               m_changed;
    float              m_acc_speed;
    float              m_rotation;
    glm::vec2          m_mouse_pos;
    float              m_rotation_speed;
    std::deque<Bullet> m_bullets;
};


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

        player.physics_update();
        player.update();

        player.keep_in_bounds(&window);
        for (Bullet& bullet : player.bullets())
        {
            bullet.keep_in_bounds(&window);
        }

        player.draw_bullets(&window);
        window.draw(player.varray());

        window.display();


        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseMoved)
                player.inform_about_mouse_pos(event.mouseMove.x, event.mouseMove.y);
            else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
                player.shoot();
        }
    }

    return 0;
}

