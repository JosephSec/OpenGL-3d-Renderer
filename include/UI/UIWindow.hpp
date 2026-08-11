#pragma once

#include <vector>
#include <System/Rect.hpp>
#include <UI/UIAPI.hpp>
#include <UI/UITab.hpp>


namespace UI {
  class UI_API UIWindow {
  public:
    unsigned int id = 0;
    bool IsActive = false;
    Rect rect;
    std::vector<UITab> tabs;
    UITab* currentTab = nullptr;
    int currentTabIndex = -1;


    UIWindow(const Rect& _rect = Rect(), bool _active = false) : rect(_rect), IsActive(_active) {
      id = windowC;
      windowC++;
    }

    void draw();

    inline void AddTab(UITab tab) {
      currentTabIndex = tabs.size();
      tabs.push_back(tab);
      currentTab = &tabs.back();
    }
    inline void AddObject(UIObject* object) {
      object->index = currentTab->objects.size();
      currentTab->objects.push_back(object);
    }

    inline bool inArea(const Vec2& _mouse) const {
      return rect.getInBounds(_mouse);
    }

    inline Vec2 getPosition() const {
      return rect.getPosition();
    }
    inline Vec2 getSize() const {
      return rect.getSize();
    }
    inline void setPosition(const Vec2& _pos) {
      rect.setPosition(_pos);
    }
    inline void setSize(const Vec2& _siz) {
      rect.setSize(_siz);
    }

  private:
    static unsigned int windowC;
  };
};