#include <System.hpp>
#include <User.hpp>
#include <Engine/Renderer.hpp>
#include <Editor.hpp>
#include <SceneManager.hpp>

#include <SFML/Graphics.hpp>

#include <iostream>

#include <random>


static void init() {
  System::init();
  Renderer::init();
  User::init();

  Editor::init();
  SceneManager::init();
}


int main(int argc, char *argv[]) {
  init();
  
  SceneManager::TestPrimitiveMeshLoading();

  
  while(Renderer::window->isOpen()) {
    while(const auto &eventOpt = Renderer::window->pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Renderer::window->close();
      else if(event.is<sf::Event::KeyPressed>()) User::HandleEvent(event);
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) Renderer::HandleResize();
    }


    { //Update
      System::update();
      User::Mouse::update();
      Editor::update();
      SceneManager::update();
    }

    { //Render
      Renderer::clear();

      Editor::draw();
      SceneManager::draw();

      Renderer::display();
    }
  }

  Renderer::end();
  Editor::end();
  SceneManager::end();

  return 0;
}