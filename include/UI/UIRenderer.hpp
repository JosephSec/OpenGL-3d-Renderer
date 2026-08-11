#pragma once

#include <SFML/Graphics.hpp>
#include <System/Rect.hpp>
#include <UI/UIAPI.hpp>
#include <UI/UIWindow.hpp>

using Color = sf::Color;


namespace UI {
  class UI_API UIRenderer {
  public:
    static const Color MENUCOLOR;
    static const Color TEXTFILLCOLOR;
    static const Color EDITTEXTFILLCOLOR;
    static const float BUTTONSIZE;
    static const int CHARACTERSIZE;
    static const Rect PAD;
    static const int VALUEPAD;
    static const Vec2 UIBUTTONSIZE;
    static const Vec2 COLORPREFABSIZE;

    static sf::RenderWindow* window;
    static std::vector<UIWindow*> windows;

    static sf::Font font;
    static Text TextPrefab;


    static void init(const string& path, sf::RenderWindow* _window);
    static void draw();
    static inline void draw(const sf::Drawable& drawable) {
      window->draw(drawable);
    }

    static void DrawRect(const Rect& _rect, const Color& _clr);

    static inline UIWindow* tryGetWindow(const Vec2& _mouse) {
      for(int i = windows.size()-1; i >= 0; i--) {
        if(!windows[i]->IsActive || !windows[i]->inArea(_mouse)) continue;
        return windows[i];
      }
      return nullptr;
    }
    static inline void SetWindowToTop(UIWindow* windowObject) {
      const int windowC = windows.size();
      for(int i = 0; i < windowC-1; i++) {
        UIWindow* checkWindow = windows[i];
        if(windowObject->id != checkWindow->id) continue;
        for(int j = i; j < windowC-1; j++) {
          windows[j] = windows[j+1];
        }
        windows[windowC-1] = windowObject;
        return;
      }
    }
  };
};