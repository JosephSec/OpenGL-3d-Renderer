#pragma once

#include <System/Rect.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <UI/UIAPI.hpp>
#include <UI/UIWindow.hpp>

using Color = sf::Color;
using Text  = sf::Text;


namespace UI {
  class UI_API ColorSelect {
  public:
    static UIWindow uiWindow;

    static Text colorValsText;
    
    static Rect colorSelectorRect;
    static Color* selectedColor;

    static void init();
    static void draw();
    static void clear();

    static void Start(const Vec2& position, Color* color);

    static bool HandleLeftClick(const Vec2& _mouse);
  
  private:
    static UITitle<uint8_t>* redObject;
    static UITitle<uint8_t>* greenObject;
    static UITitle<uint8_t>* blueObject;
    static UITitle<uint8_t>* alphaObject;
  };
};