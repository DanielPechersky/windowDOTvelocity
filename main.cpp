#include <cmath>
#include <fstream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

class Movable {
public:
    virtual sf::Vector2f getVelocity() const {
        return velocity;
    }

    virtual void setVelocity(const sf::Vector2f& velocity) {
        this->velocity = velocity;
    }

    virtual void addVelocity(const sf::Vector2f& velocity) {
        this->velocity += velocity;
    }

    virtual void update(sf::Time t) = 0;

protected:
    sf::Vector2f velocity;
};

class PhysicsWindow: public sf::RenderWindow, public Movable {
public:
    PhysicsWindow(float bounciness, sf::VideoMode mode, const sf::String& title, sf::Uint32 style = sf::Style::Default, const sf::ContextSettings& settings = sf::ContextSettings()):
            sf::RenderWindow(mode, title, style, settings), bounciness(bounciness) {
        lastPosition = getPosition();
    }

    bool isFrozen() const {
        return frozen;
    }

    void setFrozen(bool frozen) {
        this->frozen = frozen;
    }

    bool hasMoved() const {
        return lastPosition != getPosition();
    }

    virtual void update(sf::Time t) {
        if (!frozen) {
            setPosition(getPosition() + sf::Vector2i(velocity * t.asSeconds()));

            velocity.y += 800.0f * t.asSeconds();

            sf::IntRect bounds = getBounds();
            if (getPosition().x < bounds.left) {
                setPosition(sf::Vector2i(0, getPosition().y));
                velocity.x = std::abs(velocity.x)*bounciness;
            } else if (getPosition().x > (bounds.width + bounds.left) - getSize().x) {
                setPosition(sf::Vector2i((bounds.width + bounds.left) - getSize().x, getPosition().y));
                velocity.x = -std::abs(velocity.x)*bounciness;
            }
            if (getPosition().y <= bounds.top) {
                velocity.y = std::abs(velocity.y)*bounciness;
            } else if (getPosition().y > (bounds.height + bounds.top) - getSize().y) {
                setPosition(sf::Vector2i(getPosition().x, (bounds.height + bounds.top) - getSize().y));
                velocity.y = -std::abs(velocity.y)*bounciness;
            }
        } else {
            velocity *= 0.0f;
        }

        lastPosition = getPosition();
    }

private:
    sf::IntRect getBounds() const {
        sf::IntRect bounds(0, 0, sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
#ifdef __APPLE__
        bounds.top = 88;
        bounds.height -= 88;
#elif _WIN32
#endif
        return bounds;
    }

    sf::Vector2i lastPosition;

    bool frozen = true;
    float bounciness;
};

class Ball: public sf::CircleShape, public Movable {
public:
    Ball(PhysicsWindow& window, float bounciness, float radius = 0, std::size_t pointCount = 30):
            CircleShape(radius, pointCount), bounciness(bounciness) {
        setOrigin(radius, radius);
        this->window = &window;
    }

    bool isFrozen() const {
        return frozen;
    }

    void setFrozen(bool frozen) {
        this->frozen = frozen;
        if (!frozen)
            lastScreenPosition = getPositionInScreen();
    }

    void resetLastScreenPosition() {
        lastScreenPosition = getPositionInScreen();
    }

    virtual void update(sf::Time t) {
        if (!frozen) {
            setPositionInScreen(lastScreenPosition);
            move(velocity * t.asSeconds());

            velocity.y += 800.0f * t.asSeconds();

            // the quantity by which the window displaced the ball
            sf::Vector2f displacement_by_window;
            if (getPosition().x < getRadius()) {
                displacement_by_window.x += getRadius() - getPosition().x;
                setPosition(getRadius(), getPosition().y);
                velocity.x = std::abs(velocity.x)*bounciness;
            } else if (getPosition().x > window->getSize().x - getRadius()) {
                displacement_by_window.x += (window->getSize().x - getRadius()) - getPosition().x;
                setPosition(window->getSize().x - getRadius(), getPosition().y);
                velocity.x = -std::abs(velocity.x)*bounciness;
            }
            if (getPosition().y < getRadius()) {
                displacement_by_window.y += getRadius() - getPosition().y;
                setPosition(getPosition().x, getRadius());
                velocity.y = std::abs(velocity.y)*bounciness;
            } else if (getPosition().y > window->getSize().y - getRadius()) {
                displacement_by_window.y += (window->getSize().y - getRadius()) - getPosition().y;
                setPosition(getPosition().x, window->getSize().y - getRadius());
                velocity.y = -std::abs(velocity.y)*bounciness;
            }

            velocity += displacement_by_window*3.0f;

            lastScreenPosition = getPositionInScreen();
        }
    }

private:
    sf::Vector2f getPositionInScreen() const {
        return sf::Vector2f(window->getPosition()) + getPosition();
    }

    void setPositionInScreen(const sf::Vector2f& position) {
        setPosition(position - sf::Vector2f(window->getPosition()));
    }

    PhysicsWindow* window;
    sf::Vector2f lastScreenPosition;

    bool frozen = true;
    float bounciness;
};

struct Configuration {
    Configuration(): window_dims(800, 600), ball_bounciness(.85f), window_bounciness(.85f) {};

    sf::VideoMode window_dims;
    float ball_bounciness;
    float window_bounciness;
};

Configuration getConfig(std::string filepath = "") {
    Configuration config;

    if (!filepath.empty()) {
        std::ifstream file;
        file.open(filepath);
        if (file.is_open())
            while (!file.eof()) {
                std::string line;
                std::getline(file, line);
                unsigned long equals_pos = line.find("=");
                std::string key = line.substr(0, equals_pos);
                std::string val = line.substr(equals_pos + 1, line.size());

                if (key == "width")
                    config.window_dims.width = static_cast<unsigned int>(std::stoul(val));
                else if (key == "height")
                    config.window_dims.height = static_cast<unsigned int>(std::stoul(val));
                else if (key == "ball_bounciness")
                    config.ball_bounciness = std::stof(val);
                else if (key == "window_bounciness")
                    config.window_bounciness = std::stof(val);
            }
    }

    return config;
}

int main() {
    Configuration config = getConfig("config.cfg");

    PhysicsWindow window(config.window_bounciness, config.window_dims, "", sf::Style::Titlebar);
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    Ball ball(window, config.ball_bounciness, 50, 200);
    ball.setPosition(sf::Vector2f(window.getSize()/2u));
    ball.setFillColor(sf::Color(200, 200, 0));
    ball.setFrozen(false);

    sf::Clock clock;

    bool isDragging = false;
    sf::Vector2i mousePressedPos;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                else if (event.key.code == sf::Keyboard::LShift || event.key.code == sf::Keyboard::RShift) {
                    window.setFrozen(true);
                    ball.setFrozen(true);
                }
            } else if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::LShift || event.key.code == sf::Keyboard::RShift)
                    if (!isDragging) {
                        window.setFrozen(false);
                        ball.setFrozen(false);
                    }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDragging = true;
                    window.setFrozen(true);
                    mousePressedPos = sf::Mouse::getPosition();
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDragging = false;
                    window.setFrozen(false);
                    ball.setFrozen(false);
                    window.setVelocity(sf::Vector2f(sf::Mouse::getPosition() - mousePressedPos)*3.0f);
                }
            }
        }
        
        sf::Time elapsedTime = clock.restart();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            window.setVelocity(sf::Vector2f());

        if (window.hasMoved())
            ball.resetLastScreenPosition();
        window.update(elapsedTime);
        ball.update(elapsedTime);

        window.clear(sf::Color(30, 30, 30));
        window.draw(ball);
        window.display();
    }

    return 0;
}