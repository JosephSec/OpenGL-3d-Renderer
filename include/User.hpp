#pragma once

#include <vector>
#include <SFML/Window/Event.hpp>
#include <System/Vec2.hpp>

using Event = sf::Event;
namespace Keyboard = sf::Keyboard;
namespace Mouse = sf::Mouse;


class User {
public:
  static Vec2 mousePos;


  static void update();
  
  static void handle(Event event);

  static inline bool GetKey(const Keyboard::Key& key) {
    return Keyboard::isKeyPressed(key);
  }
  static inline bool GetKeyDown(const Keyboard::Key& key) {
    for(const Keyboard::Key& curA : curKeys) {
      if(curA != key) continue;
      for(const Keyboard::Key& curB : prevKeys) {
        if(curB != key) continue;
        return false;
      }
      return true;
    }
    return false;
  }
  static inline bool GetKeyUp(const Keyboard::Key& key) {
    for(const Keyboard::Key& curA : prevKeys) {
      if(curA != key) continue;
      for(const Keyboard::Key& curB : curKeys) {
        if(curB != key) continue;
        return false;
      }
      return true;
    }
    return false;
  }


  static inline bool GetMouseButton(const Mouse::Button& button) {
    return Mouse::isButtonPressed(button);
  }
  static inline bool GetMouseButtonDown(const Mouse::Button& button) {
    for(const Mouse::Button& curA : curMButtons) {
      if(curA != button) continue;
      for(const Mouse::Button& curB : prevMButtons) {
        if(curB != button) continue;
        return false;
      }
      return true;
    }
    return false;
  }
  static inline bool GetMouseButtonUp(const Mouse::Button& button) {
    for(const Mouse::Button& curA : prevMButtons) {
      if(curA != button) continue;
      for(const Mouse::Button& curB : curMButtons) {
        if(curB != button) continue;
        return false;
      }
      return true;
    }
    return false;
  }


private:
  static std::vector<Keyboard::Key> curKeys;
  static std::vector<Keyboard::Key> prevKeys;

  static std::vector<Mouse::Button> curMButtons;
  static std::vector<Mouse::Button> prevMButtons;
};