#include <Editor/UI/PerformanceOverlay.hpp>
using namespace Editor::UI;

#include <Renderer/Core.hpp>
#include <Renderer/Hardware/Monitor.hpp>
#include <Renderer/Hardware/GPU.hpp>


bool PerformanceOverlay::IsEnabled = true;

sf::FloatRect PerformanceOverlay::Rect = sf::FloatRect{{15.0f, 15.0f}, {350.0f, 150.0f}};

Renderer::LineGraph PerformanceOverlay::fpsGraph(60, 0, 240.0f);
Renderer::LineGraph PerformanceOverlay::vramGraph(60, 0, 1);
Renderer::LineGraph PerformanceOverlay::testGraph(60, 0, 1);


void PerformanceOverlay::init() {  
  fpsGraph.max = Renderer::Monitor::RefreshRate;
  fpsGraph.setTimerLength(1 / 15.0f);

  vramGraph.max = Renderer::GPU::VRAMCapacity;
  vramGraph.setTimerLength(1 / 15.0f);

  testGraph.max = 1;
  testGraph.setTimerLength(1 / 15.0f);
}
void PerformanceOverlay::update(float _dt) {
  const float fps = 1.0f / _dt;
  const int vram = Renderer::GPU::VRAMInUse;
  const float performanceT = (fps / fpsGraph.max + (1 - (vram / vramGraph.max))) / 2.0f;

  fpsGraph.update(fps, _dt);
  vramGraph.update(vram, _dt);
  testGraph.update(performanceT, _dt);
}
void PerformanceOverlay::draw(const sf::Font &_font) {
  if(IsEnabled == false) return;

  const int elementCount = 3;
  const float f_elementCount = static_cast<float>(elementCount);

  const int outlineThickness = 4;

  const sf::Vector2f startPad = sf::Vector2f{Rect.size.x * .05f, Rect.size.y * .05f} + sf::Vector2f{1,1} * static_cast<float>(outlineThickness);
  const sf::Vector2f elementSize = sf::Vector2f{Rect.size.x - startPad.x * 2, Rect.size.y * .8f / f_elementCount - 0 / f_elementCount} - sf::Vector2f{1,2} * static_cast<float>(outlineThickness);
  const float graphWidth = elementSize.x * .6f;

  const Renderer::LineGraph *lineGraphs[3] = {&fpsGraph, &vramGraph, &testGraph};
  const sf::Color colors[3] = {sf::Color(0,0,255,100), sf::Color(255,0,0,100), sf::Color(0,255,0,100)};
  const std::string names[3] = {"FPS", "VRAM", "Performance"};
  const std::string values[3] = {
    std::to_string(static_cast<int>(fpsGraph.getValue())),
    std::to_string(static_cast<int>(vramGraph.getValue() / 1000)) + "KB",
    std::to_string(static_cast<int>(testGraph.getValue() * 100.0f)) + "%",
  };


  sf::RectangleShape shape(Rect.size);
  shape.setFillColor(sf::Color(125,125,125,225));
  shape.setPosition(Rect.position);
  Renderer::Core::window->draw(shape);

  sf::Text text(_font);
  text.setCharacterSize(15);
  text.setStyle(sf::Text::Style::Bold);


  for(int i = 0; i < elementCount; i++) {
    const sf::Vector2f start = Rect.position + startPad + sf::Vector2f{0, (elementSize.y + outlineThickness * 2) * i};

    shape.setOutlineThickness(outlineThickness);
    shape.setOutlineColor(sf::Color::Black);
    shape.setPosition(start);
    shape.setSize(elementSize);
    shape.setFillColor(colors[i]);
    Renderer::Core::window->draw(shape);

    { //Label Text
      const sf::Vector2f labelTextAnchor = start + sf::Vector2f{elementSize.x - graphWidth, elementSize.y} / 2.0f;

      text.setString(names[i]);

      const sf::FloatRect localBounds = text.getLocalBounds();
      text.setPosition(labelTextAnchor - localBounds.size / 2.0f - localBounds.position);
      Renderer::Core::window->draw(text);
    } //Label Text
    { //Value Text
      const sf::Vector2f valueTextAnchor = start + sf::Vector2f{elementSize.x - graphWidth + 5, elementSize.y / 2.0f};

      text.setString(values[i]);

      const sf::FloatRect localBounds = text.getLocalBounds();
      // text.setPosition(valueTextAnchor - sf::Vector2f{0, localBounds.size.y / 2.0f - localBounds.position.y});
      text.setPosition(valueTextAnchor - localBounds.position - sf::Vector2f{0, localBounds.size.y / 2.0f});
      Renderer::Core::window->draw(text);
    } //Value Text


    lineGraphs[i]->draw(sf::FloatRect{start + sf::Vector2f{elementSize.x - graphWidth, 0}, sf::Vector2f{graphWidth, elementSize.y}}, sf::Color::White);
  }
}