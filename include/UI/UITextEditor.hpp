#pragma once

#include <optional>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <UI/UIAPI.hpp>
#include <UI/UIWindow.hpp>
#include <System/Rect.hpp>


using Event = sf::Event;
namespace Keyboard = sf::Keyboard;
using Text = sf::Text;


static constexpr uint32_t EnterUnicode = 13;
static constexpr uint32_t BackspaceUnicode = 8;

namespace UI {
  class UI_API TextEditor {
  public:
    static Rect buttonRect;
    static std::vector<UIWindow*> windows;
    static std::optional<UITitleVariant> selected;

    
    static void update(UIWindow* window);
    static void draw();

    static bool HandleLeftClick(const sf::Vector2f& mouse, UIWindow* _window);
    static bool HandleTextEntered(const Event& event);

    static void EnterText();
    static void BackspaceText();
    
    static const sf::RectangleShape getIndicator();
    static inline const Vec2 getTextArea(float valWidth) {
      return Vec2(valWidth+6, 18);
    }

    static std::optional<UITitleVariant> tryGetTitleVariant(UIObject* object);
    static Text* getText(std::optional<UITitleVariant> object = std::nullopt);
  };
};