#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Font.hpp>

#include <Renderer/Debug/Graph.hpp>


namespace Editor {
  namespace UI {
    class PerformanceOverlay {
    public:
      static bool IsEnabled;

      static sf::FloatRect Rect;

      static Renderer::LineGraph fpsGraph;
      static Renderer::LineGraph vramGraph;
      static Renderer::LineGraph testGraph;


      static void init();
      static void update(float _dt);
      static void draw(const sf::Font &_font);

      static inline void ToggleEnabled(bool _enabled = !IsEnabled) {
        IsEnabled = _enabled;
      }
    };
  }
}