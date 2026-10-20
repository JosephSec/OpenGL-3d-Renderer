#include <Renderer/Core.hpp>
using namespace Renderer;

#include <Renderer/Hardware/GPU.hpp>
#include <Renderer/Hardware/Monitor.hpp>

#include <cstdint>
#include <string>
#include <algorithm>


sf::RenderWindow *Core::window;
glm::ivec2 Core::windowSize;


void Core::init(bool _enableVSync) {
  window = new sf::RenderWindow(sf::VideoMode({800,600}), "3D Rendering Library");
  window->setVerticalSyncEnabled(_enableVSync);
  windowSize = glm::ivec2{window->getSize().x, window->getSize().y};

  glewInit();

  GPU::VRAMCapacity = GPU::GetVRAMCapacity();
  GPU::VRAMInUse = 0;

  Monitor::RefreshRate = Monitor::GetCurrentMonitorRefreshRate();

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  glCullFace(GL_BACK);
}
void Core::clear() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Core::display() {
  window->display();
}
void Core::end() {}

void Core::UpdateWindowSize() {
  windowSize = glm::vec2{window->getSize().x, window->getSize().y};

  window->setView(sf::View(sf::FloatRect{{0,0}, {static_cast<float>(windowSize.x),static_cast<float>(windowSize.y)}}));

  glViewport(0,0, windowSize.x,windowSize.y);
}
