#include <System.hpp>
#include <User.hpp>
#include <Renderer.hpp>


int main(int argc, char* argv[]) {
  System::init();
  sf::RenderWindow* window = Renderer::window;


  while(window->isOpen()) {
    while(std::optional<Event> eventOpt = window->pollEvent()) {
      const auto& event = *eventOpt;

      User::handle(event);
           if(event.is<Event::Closed>()) window->close();
      else if(event.is<Event::Resized>()) Renderer::HandleResize();
    }

    System::update();
    Renderer::draw();
  }

  Renderer::clear();
  return 0;
}