#include <Editor/UI/PerformanceOverlay.hpp>
using namespace Editor;

#include <Renderer/Core.hpp>
#include <Renderer/Hardware/Monitor.hpp>
#include <Renderer/Hardware/GPU.hpp>


bool PerformanceOverlay::IsEnabled = true;

sf::FloatRect PerformanceOverlay::Rect = sf::FloatRect{{15.0f, 15.0f}, {350.0f, 150.0f}};

Renderer::LineGraph PerformanceOverlay::fpsGraph(60, 0, 240.0f);
Renderer::LineGraph PerformanceOverlay::vramGraph(60, 0, 1);


void PerformanceOverlay::init() {  
  fpsGraph.max = Renderer::Monitor::RefreshRate;
  fpsGraph.setTimerLength(1 / 15.0f);

  vramGraph.max = Renderer::GPU::VRAMCapacity;
  vramGraph.setTimerLength(1 / 15.0f);
}
void PerformanceOverlay::update(float _dt) {
  fpsGraph.update(1.0f / _dt, _dt);
  vramGraph.update(Renderer::GPU::VRAMInUse, _dt);
}
void PerformanceOverlay::draw(const sf::Font &_font) {
  if(IsEnabled == false) return;

  sf::RectangleShape shape(Rect.size);
  shape.setPosition(Rect.position);
  shape.setFillColor(sf::Color(25,25,25,200));
  Renderer::Core::window->draw(shape);

  fpsGraph.draw(Rect, sf::Color::Blue);
  vramGraph.draw(Rect, sf::Color::Red);

  sf::Text text(_font);
  text.setCharacterSize(15);

  text.setString(std::to_string(static_cast<int>(fpsGraph.getValue())));
  text.setFillColor(sf::Color::Blue);
  text.setPosition(fpsGraph.getPlotPosition(Rect, fpsGraph.getSize() - 1) - text.getLocalBounds().size);
  Renderer::Core::window->draw(text);
  
  text.setString(std::to_string(static_cast<int>(vramGraph.getValue())));
  text.setFillColor(sf::Color::Red);
  text.setPosition(vramGraph.getPlotPosition(Rect, vramGraph.getSize() - 1) - text.getLocalBounds().size);
  Renderer::Core::window->draw(text);
}