#include <Editor/UI/Button.hpp>
using namespace Editor::UI;

#include <Renderer/Core.hpp>


void UIButton::draw() const {
  sf::RectangleShape shape(m_rect.size);
  shape.setPosition(m_rect.position);
  shape.setFillColor(m_color);
  shape.setTexture(m_texture, true);
  Renderer::Core::window->draw(shape);
}