#pragma once

#include <string>
#include <vector>
#include <UI/UIAPI.hpp>
#include <UI/UIObject.hpp>

using string = std::string;


class UI_API UITab {
public:
  string name;
  std::vector<UI::UIObject*> objects;

  UITab(const string& _name = "#Empty") : name(_name) {}
};