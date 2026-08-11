#pragma once

#include <SFML/Graphics.hpp>
#include <UI/UIAPI.hpp>
#include <UI/UIWindow.hpp>

using Event = sf::Event;
namespace Keyboard = sf::Keyboard;
namespace Mouse = sf::Mouse;

using Color = sf::Color;


namespace UI {
  class UI_API Manager {
  public:
    static sf::RenderWindow* window;
    static string PATH;

    static UIWindow* grabbed;
    static Vec2 grabOffset;
    

    static void init(const string& path, sf::RenderWindow* target, bool controller = false);
    static void update();
    static void draw();
    static void clear();

    static const bool handle(const Event& event);
  };
};