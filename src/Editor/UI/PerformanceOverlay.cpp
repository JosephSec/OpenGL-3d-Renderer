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

  static constexpr int outlineThickness = 4;
  static constexpr float outlineSaturation = .1f;
  static constexpr float elementSaturation = .1f;
  static constexpr uint8_t alpha = 255;

  struct Element {
  public:
    Renderer::LineGraph *graph = nullptr;
    sf::Color color = sf::Color::White;
    std::string name = "None";
    std::string value = "0x0";
  };

  static constexpr int elementCount = 2;
  static constexpr float f_elementCount = static_cast<float>(elementCount);

  static const sf::Vector2f elementSize = sf::Vector2f{Rect.size.x, Rect.size.y * .8f / f_elementCount - 0 / f_elementCount} - sf::Vector2f{1,2} * static_cast<float>(outlineThickness);
  static const float graphWidth = elementSize.x * .6f;

  const Element elements[2] = {
    {&fpsGraph,  sf::Color(0,0,255 * elementSaturation, alpha), "FPS",         std::to_string(static_cast<int>(fpsGraph.getValue()))},
    {&vramGraph, sf::Color(255 * elementSaturation,0,0, alpha), "VRAM",        std::to_string(static_cast<int>(vramGraph.getValue() / 1000)) + "KB"},
    // {&vramGraph, sf::Color(0,255 * elementSaturation,0, alpha), "Performance", std::to_string(static_cast<int>(testGraph.getValue() * 100.0f)) + "%"},
  };


  sf::RectangleShape shape;
  shape.setOutlineThickness(outlineThickness);
  shape.setOutlineColor(sf::Color(255 * outlineSaturation,255 * outlineSaturation,255 * outlineSaturation, alpha));
  
  sf::Text text(_font);
  text.setCharacterSize(15);
  text.setStyle(sf::Text::Style::Bold);


  for(int i = 0; i < elementCount; i++) {
    const Element &element = elements[i];

    const sf::Vector2f start = Rect.position + sf::Vector2f{0, (elementSize.y + outlineThickness * 2) * i};

    shape.setPosition(start);
    shape.setSize(elementSize);
    shape.setFillColor(element.color);
    Renderer::Core::window->draw(shape);

    { //Label Text
      const sf::Vector2f labelTextAnchor = start + sf::Vector2f{elementSize.x - graphWidth, elementSize.y} / 2.0f;

      text.setString(element.name);

      const sf::FloatRect localBounds = text.getLocalBounds();
      text.setPosition(labelTextAnchor - localBounds.size / 2.0f - localBounds.position);
      Renderer::Core::window->draw(text);
    } //Label Text
    { //Value Text
      const sf::Vector2f valueTextAnchor = start + sf::Vector2f{elementSize.x - graphWidth + 5, elementSize.y / 2.0f};

      text.setString(element.value);

      const sf::FloatRect localBounds = text.getLocalBounds();
      // text.setPosition(valueTextAnchor - sf::Vector2f{0, localBounds.size.y / 2.0f - localBounds.position.y});
      text.setPosition(valueTextAnchor - localBounds.position - sf::Vector2f{0, localBounds.size.y / 2.0f});
      Renderer::Core::window->draw(text);
    } //Value Text


    element.graph->draw(sf::FloatRect{start + sf::Vector2f{elementSize.x - graphWidth, 0}, sf::Vector2f{graphWidth, elementSize.y}}, sf::Color::White);
  }
}