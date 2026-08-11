#pragma once

#include <variant>
#include <string>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Color.hpp>
#include <UI/UIAPI.hpp>
#include <System/Vec2.hpp>

using Text = sf::Text;
using Color = sf::Color;
using string = std::string;


namespace UI {
  class UIWindow;

  class UI_API UIObject {
  public:
    int index;
    Vec2 position;
    string name;

    UIObject(const string& str);
    virtual ~UIObject() {}

    virtual void draw(UIWindow* _uiWindow) const {}
  };

  class UI_API UIBool : public UIObject {
  public:
    bool& value;

    UIBool(const string& str, bool& val);

    void draw(UIWindow* _uiWindow) const override;
  };

  template <typename T>
  class UI_API UITitle : public UIObject {
  public:
    T* value;
    mutable Text text;
    bool editable;

    UITitle(const string& str, T* val = nullptr, bool isEditable = false);

    void draw(UIWindow* _uiWindow) const override;

    void setString(const string& str = "\0");
  };

  class UI_API UIColor : public UIObject {
  public:
    Color& value;

    UIColor(const string& str, Color& val);

    void draw(UIWindow* _uiWindow) const override;
  };


  //Add Custom UITitle Templates Here
  //Make sure custom UITitle templates have sstream operator<< functions
    //Make sure to do the same thing at the bottom of the UIObject.cpp file

  using UITitleVariant = std::variant<
    UITitle<string>*,
    UITitle<int>*,
    UITitle<unsigned int>*,
    UITitle<unsigned char>*,
    UITitle<float>*
  >;

  extern template class UITitle<string>;
  extern template class UITitle<int>;
  extern template class UITitle<unsigned int>;
  extern template class UITitle<unsigned char>;
  extern template class UITitle<float>;
  //Add Custom UITitle Templates Here
};

#include <UI/UIWindow.hpp>