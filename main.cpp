#include <iostream>
#include <random>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

class Movable {
public:
    virtual sf::Vector2f getVelocity() const {
        return velocity;
    }

    virtual void setVelocity(sf::Vector2f velocity) {
        this->velocity = velocity;
    }

    virtual void addVelocity(sf::Vector2f velocity) {
        this->velocity += velocity;
    }

    virtual void update(sf::Time t) = 0;

protected:
    sf::Vector2f velocity;
};

class PhysicsWindow: public sf::RenderWindow, public Movable {
public:
    PhysicsWindow(sf::VideoMode mode, const sf::String& title, sf::Uint32 style = sf::Style::Default, const sf::ContextSettings& settings = sf::ContextSettings()):
            sf::RenderWindow(mode, title, style, settings) {
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
                velocity.x *= -1;
            } else if (getPosition().x > (bounds.width + bounds.left) - getSize().x) {
                setPosition(sf::Vector2i((bounds.width + bounds.left) - getSize().x, getPosition().y));
                velocity.x *= -1;
            }
            if (getPosition().y <= bounds.top) {
                velocity.y = std::abs(velocity.y);
            } else if (getPosition().y > (bounds.height + bounds.top) - getSize().y) {
                setPosition(sf::Vector2i(getPosition().x, (bounds.height + bounds.top) - getSize().y));
                velocity.y *= -.85f;
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

    bool frozen = true;

    sf::Vector2i lastPosition;
};

class Ball: public sf::CircleShape, public Movable {
public:
    Ball(PhysicsWindow& window, float radius = 0, std::size_t pointCount = 30):
            CircleShape(radius, pointCount) {
        setOrigin(radius, radius);
        this->window = &window;
    }

    const bool isFrozen() {
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
            if (lastScreenPosition != sf::Vector2i())
                setPositionInScreen(lastScreenPosition);
            velocity.y += 800.0f * t.asSeconds();

            move(velocity * t.asSeconds());

            // the quantity by which the window displaced the ball
            sf::Vector2f displacement_by_window;
            if (getPosition().x < getRadius()) {
                displacement_by_window.x += getRadius() - getPosition().x;
                setPosition(getRadius(), getPosition().y);
                velocity.x = std::abs(velocity.x)*.9f;
            } else if (getPosition().x > window->getSize().x - getRadius()) {
                displacement_by_window.x += (window->getSize().x - getRadius()) - getPosition().x;
                setPosition(window->getSize().x - getRadius(), getPosition().y);
                velocity.x = -std::abs(velocity.x)*.9f;
            }
            if (getPosition().y < getRadius()) {
                displacement_by_window.y += getRadius() - getPosition().y;
                setPosition(getPosition().x, getRadius());
                velocity.y = std::abs(velocity.y) * .8f;
            } else if (getPosition().y > window->getSize().y - getRadius()) {
                displacement_by_window.y += (window->getSize().y - getRadius()) - getPosition().y;
                setPosition(getPosition().x, window->getSize().y - getRadius());
                velocity.y *= -.85f;
            }

            velocity += displacement_by_window*2.0f;

            lastScreenPosition = getPositionInScreen();
        }
    }

private:
    sf::Vector2i mapCoordsToScreen(const sf::Vector2f& point) const {
        return mapPixelToScreen(window->mapCoordsToPixel(point));
    }

    sf::Vector2f mapScreenToCoords(const sf::Vector2i& point) const {
        return window->mapPixelToCoords(mapScreenToPixel(point));
    }

    sf::Vector2i mapPixelToScreen(const sf::Vector2i& point) const {
        return point + window->getPosition();
    }

    sf::Vector2i mapScreenToPixel(const sf::Vector2i& point) const {
        return point - window->getPosition();
    }

    sf::Vector2i getPositionInScreen() const {
        return mapCoordsToScreen(getPosition());
    }

    void setPositionInScreen(const sf::Vector2i& position) {
        setPosition(mapScreenToCoords(position));
    }

    void movePositionInScreen(const sf::Vector2i& offset) {
        move(mapScreenToCoords(offset));
    }

    PhysicsWindow* window;
    sf::Vector2i lastScreenPosition;
    bool frozen = true;
};

int main() {
    PhysicsWindow window(sf::VideoMode(800, 600), "", sf::Style::Titlebar);
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    Ball ball(window, 50, 200);
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
                if (event.key.code == sf::Keyboard::LShift)
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