#include <UI/Core.hpp>
using namespace UI;

#include <System.hpp>

#include <Renderer/Core.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <iostream>


void SmoothRect::draw() const {
  sf::RectangleShape shape(size);
  shape.setPosition(position);

  Renderer::Core::SetShader(Core::smoothRectShader);
  Core::smoothRectShader.SetUniform("rect", glm::vec4(position.x,position.y, size.x,size.y));
  Core::smoothRectShader.SetUniform("radius", radius);
  Core::smoothRectShader.SetUniform("color", color);

  Core::window->draw(shape);
  Renderer::Core::SetShader(0);
}


void SmoothButton::draw() const {
  SmoothRect::draw();

  sf::Text text(Core::font);
  text.setString("Hierarchy");
  text.setCharacterSize(15);
  const sf::FloatRect localBounds = text.getLocalBounds();
  text.setPosition(position + (size - localBounds.size) / 2.0f - localBounds.position);
  Core::window->draw(text);
}



sf::RenderWindow *Core::window;
sf::Font Core::font;

Renderer::Shader Core::smoothRectShader;


void Core::init(sf::RenderWindow *_window, const std::string &_font) {
  window = _window;

  if(font.openFromFile(System::PATH+"/assets/" + _font) == false) {
    std::cout << "[UI Error]: Font file was not found or could not be opened\n";
  }

  smoothRectShader = Renderer::Shader("SmoothRect");
}
void Core::update() {}
void Core::draw() {
  Renderer::Core::SetState(Renderer::State::UI);

  Renderer::Core::SetShader(smoothRectShader);
  smoothRectShader.SetUniform("windowSize", glm::vec2(Renderer::Core::windowSize));


  SmoothRect hierarchyArea = SmoothRect({{0,0}, sf::Vector2f{300,static_cast<float>(Renderer::Core::windowSize.y)}}, 15, {.05,.05,.05, 1});
  hierarchyArea.draw();

  SmoothButton hierarchyButton(sf::FloatRect{{10,10}, {75,25}}, 5, {.3,.3,.3,1}, "Hierarchy", 15);
  hierarchyButton.draw();

  { //List
    const sf::Vector2f start = {10, 10 + 25 + 10};
    SmoothRect listArea = SmoothRect({start, sf::Vector2f{300 - start.x * 2,static_cast<float>(Renderer::Core::windowSize.y) - start.y - 10}}, 15, {.1,.1,.1, 1});
    listArea.draw();
  }
}