#include <Editor/UI/PerformanceOverlay.hpp>
using namespace Editor;

#include <Renderer/Core.hpp>
#include <Renderer/Hardware/Monitor.hpp>
#include <Renderer/Hardware/GPU.hpp>


bool PerformanceOverlay::IsEnabled = true;

sf::FloatRect PerformanceOverlay::Rect = sf::FloatRect{{15.0f, 15.0f}, {350.0f, 60.0f}};

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

  const Renderer::LineGraph *lineGraphs[2] = {&fpsGraph, &vramGraph};
  const sf::Color colors[2] = {sf::Color(0,0,255,100), sf::Color(255,0,0,100)};

  const std::string names[2] = {"FPS", "VRAM"};
  const float height = Rect.size.y / 2.0f;
  const float graphWidth = Rect.size.x * .8f;


  sf::RectangleShape shape(Rect.size);

  sf::Text text(_font);
  text.setCharacterSize(15);

  for(int i = 0; i < 2; i++) {
    const sf::Vector2f start = Rect.position + sf::Vector2f{0, height + 5} * static_cast<float>(i);

    shape.setPosition(start);
    shape.setSize(sf::Vector2f{Rect.size.x, height});
    shape.setFillColor(colors[i]);
    Renderer::Core::window->draw(shape);

    const sf::Vector2f textAreaCenter = start + sf::Vector2f{Rect.size.x - graphWidth, height} / 2.0f;

    text.setString(names[i]);

    const sf::FloatRect localBounds = text.getLocalBounds();
    text.setPosition(textAreaCenter - localBounds.size / 2.0f - localBounds.position);
    Renderer::Core::window->draw(text);


    lineGraphs[i]->draw(sf::FloatRect{start + sf::Vector2f{Rect.size.x - graphWidth, 0}, sf::Vector2f{graphWidth, height}}, sf::Color::White);
  }
}