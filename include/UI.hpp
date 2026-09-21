#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <Renderer/Shader.hpp>


namespace UI {
  struct SmoothRect : public sf::FloatRect {
  public:
    SmoothRect(const sf::FloatRect _rect, float _radius, const glm::vec4 _color)
    : sf::FloatRect(_rect), radius(_radius), color(_color) {}

    void draw() const;

    float radius = 15;
    glm::vec4 color = {.05,.05,.05, 1};
  };

  class Core {
  public:
    static void init(sf::RenderWindow *_window, const std::string &_font);
    static void update();
    static void draw();

    static Renderer::Shader smoothRectShader;

  private:
    static sf::RenderWindow *m_window;

    static sf::Font m_font;
  };
}