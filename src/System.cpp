#include <System.hpp>
#include <User.hpp>
#include <Renderer.hpp>

#include <filesystem>
#include <windows.h>


string System::PATH;

sf::Clock System::timeClock;
float System::deltaTime;

std::vector<string> System::assets;


void System::init() {
  char buffer[MAX_PATH]; GetModuleFileNameA(NULL, buffer, MAX_PATH);
  PATH = std::filesystem::path(buffer).parent_path().parent_path().string();

  Renderer::init();

  GenerateReleaseAssets();
}
void System::update() {
  deltaTime = timeClock.restart().asMilliseconds() * .001f;

  User::update();
  Renderer::update();


  if(User::GetMouseButtonDown(Mouse::Button::Left)) {
    Vec3 rayOrigin = ScreenToWorld(User::mousePos, 0);
    Vec3 rayDirection = (ScreenToWorld(User::mousePos, 1) - rayOrigin).normalize();
    float radius;

    Renderer::selected = nullptr;
    for(int i = 0; i < Renderer::objects.size(); i++) {
      radius = -INFINITY;
      for(const Vec3& vert : Renderer::objects[i].mesh->vertices) {
        const float dist = vert.magnitude();
        if(dist > radius) radius = dist;
      }

      Vec3 L = Renderer::objects[i].transform.position - rayOrigin;
      float tca = Vec3::dot(L, rayDirection);
      float d2 = Vec3::dot(L, L) - tca * tca;
      float r2 = radius * radius;
  
      if (d2 > r2) continue;

      Renderer::selected = &Renderer::objects[i];
      break;
    }

  }
}