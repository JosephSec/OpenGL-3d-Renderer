#include <UI.hpp>
#include <UI/UIRenderer.hpp>
#include <UI/UIController.hpp>
#include <iostream>

using Manager = UI::Manager;
using ColorSelect = UI::ColorSelect;
using UIWindow = UI::UIWindow;


#include <Renderer.hpp>


sf::RenderWindow* Manager::window;
string Manager::PATH;

UIWindow* Manager::grabbed;
Vec2 Manager::grabOffset = Vec2one*-1;


void Manager::init(const string& path, sf::RenderWindow* target, bool controller) {
  PATH = path;
  UIRenderer::init(PATH, window = target);

  //Add (Tabs/Objects) Here v
  UIController::AddTab("Renderer");
  UIController::AddObject(new UIBool("Draw Light Range", Renderer::DrawLightRange));
  UIController::AddObject(new UITitle("Cone Side Count", &Renderer::coneFaceCount, true));
  //Add (Tabs/Objects) Here ^
  
  if(controller) UIController::init();
  ColorSelect::init();
}
void Manager::update() {
  const Vec2 mousePos = Mouse::getPosition(*window);
  if(UIController::uiWindow.IsActive) {
    if(grabbed && (grabbed->id == UIController::uiWindow.id) && (grabOffset.x != -1)) {
      const Vec2 grabToMouseDir = mousePos-UIController::uiWindow.getPosition()-grabOffset;
      UIController::uiWindow.setPosition(grabToMouseDir + UIController::uiWindow.getPosition());
      UIController::listText.setPosition(grabToMouseDir + UIController::listText.getPosition());
      UIController::titleText.setPosition(grabToMouseDir + UIController::titleText.getPosition());
    }
    UIController::update();
  }
  if(ColorSelect::uiWindow.IsActive) {
    if(grabbed && (grabbed->id == ColorSelect::uiWindow.id) && (grabOffset.x != -1)) {
      const Vec2 mouseDif = mousePos-ColorSelect::uiWindow.getPosition()-grabOffset;
      ColorSelect::uiWindow.setPosition(mouseDif + ColorSelect::uiWindow.getPosition());
      ColorSelect::colorValsText.setPosition(mouseDif + ColorSelect::colorValsText.getPosition());
    }
  }
}
void Manager::draw() {
  UIRenderer::draw();
  TextEditor::draw();
}
void Manager::clear() {
  UIController::clear();
  ColorSelect::clear();
}

const bool Manager::handle(const Event& event) {
  if(TextEditor::HandleTextEntered(event)) {
    std::cout << "Text Editor Handled\n";
    return true;
  }

  else if(const auto* key = event.getIf<Event::KeyPressed>()) {
    if(TextEditor::selected || !Keyboard::isKeyPressed(Keyboard::Key::Grave)) return false;

    else if(key->code == Keyboard::Key::C) UIController::ToggleActive();
    else if(key->code == Keyboard::Key::R) UIController::ResetPosition();
    else if(key->code == Keyboard::Key::Num1) UIController::PreviousTab();
    else if(key->code == Keyboard::Key::Num2) UIController::NextTab();
    else return false;
    return true;
  }

  else if(const auto* mouseButton = event.getIf<Event::MouseButtonPressed>()) {
    const Vec2 mousePos = mouseButton->position;
    if(mouseButton->button == Mouse::Button::Left) {
      if(UIWindow* windowObject = UIRenderer::tryGetWindow(mousePos)) {
        UIRenderer::SetWindowToTop(windowObject);

        if(TextEditor::HandleLeftClick(mousePos, windowObject)) {
          if(windowObject->id != ColorSelect::uiWindow.id) ColorSelect::uiWindow.IsActive = false;
          return true;
        }

        if(windowObject->id == ColorSelect::uiWindow.id) {
          if(!ColorSelect::HandleLeftClick(mousePos)) //sets grabOffset and grabbed*
            grabOffset = mousePos-(grabbed = windowObject)->getPosition();
        } else if(ColorSelect::uiWindow.IsActive) ColorSelect::uiWindow.IsActive = false;

        if(windowObject->id == UIController::uiWindow.id) {
          if(!UIController::HandleLeftClick(mousePos)) //sets grabOffset and grabbed*
            grabOffset = mousePos-(grabbed = windowObject)->getPosition();
        } else grabOffset = mousePos-(grabbed = windowObject)->getPosition();
        
        return true;
      }

      ColorSelect::uiWindow.IsActive = false;
    }

    else if(mouseButton->button == Mouse::Button::Right) {
      TextEditor::selected = std::nullopt;
    }
  }

  else if(const auto* mouseButton = event.getIf<Event::MouseButtonReleased>()) {
    if(mouseButton->button == Mouse::Button::Left) {
      grabbed = nullptr;
      grabOffset = Vec2one*-1;
    }
  }
  return false;
}