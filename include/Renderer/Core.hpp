#pragma once

#include <glm/vec2.hpp>

#include <SFML/Graphics.hpp>

#include <Renderer/API.hpp>

#include <GL/glew.h>


namespace Renderer {
  class RENDER3D_API Core {
  public:
    static void init(bool _enableVSync = true);
    static void clear();
    static void display();
    static void end();

    static void UpdateWindowSize();


    static sf::RenderWindow *window;
    static glm::ivec2 windowSize;
  };
}