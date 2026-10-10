#pragma once

#include <SFML/Window/Event.hpp>

#include <glm/vec2.hpp>


class User {
public:
  enum CursorLockMode {
    None,
    Locked,
    Confined
  };
  class Cursor {
  public:
    static glm::ivec2 position;
    static glm::ivec2 delta;

    static CursorLockMode lockState;


    static void init();
    static void update();
  };


  static bool panningControl;
  static bool flyThroughControl;


  static void init();
  static void update();

  static void HandleFocusLost();
  static void HandleKeyPressed(const sf::Event::KeyPressed *_keyPressed);
  static void HandleMouseButtonPressed(const sf::Event::MouseButtonPressed *_mouseButtonPressed);
  static void HandleMouseButtonReleased(const sf::Event::MouseButtonReleased *_mouseButtonReleased);
};