#include <User.hpp>
#include <System.hpp>
#include <Renderer.hpp>

#include <Engine/TransformGizmo.hpp>


std::vector<Keyboard::Key> User::curKeys;
std::vector<Keyboard::Key> User::prevKeys;

std::vector<Mouse::Button> User::curMButtons;
std::vector<Mouse::Button> User::prevMButtons;

Vec2 User::mousePos;


void User::update() {
  mousePos = Mouse::getPosition(*Renderer::window);
  
  prevKeys = curKeys;
  curKeys.clear();

  for(int i = 0; i < Keyboard::KeyCount; i++) {
    const Keyboard::Key key = static_cast<Keyboard::Key>(i);
    if(Keyboard::isKeyPressed(key)) curKeys.emplace_back(key);
  }

  prevMButtons = curMButtons;
  curMButtons.clear();

  for(int i = 0; i < Mouse::ButtonCount; i++) {
    const Mouse::Button button = static_cast<Mouse::Button>(i);
    if(Mouse::isButtonPressed(button)) curMButtons.emplace_back(button);
  }
}

void User::handle(Event event) {
  if(const auto* key = event.getIf<Event::KeyPressed>()) {
    if(key->code == Keyboard::Key::Escape) Renderer::window->close();

    // else if(key->code == Keyboard::Key::X)
  }

  else if(const auto* mouseButton = event.getIf<Event::MouseButtonPressed>()) {
    if(mouseButton->button == Mouse::Button::Left) {
      const Vec2 screen = User::mousePos;
      const Vec3 origin = ScreenToWorld(screen, 0);

      Editorcast editorCastHit = Editorcast(origin, (ScreenToWorld(screen, 1) - origin).normalize());
      for(Color& color : TransformGizmo::translationGizmo.mesh.colors) color.a = 200;
      TransformGizmo::translationAxis.reset();
      if(editorCastHit) {
        const uint32_t index = TransformGizmo::translationGizmo.mesh.indecies[editorCastHit.faceIndex*3];

        TransformGizmo::HandleTranslationPress(index);
        const Vec3 closest = GetClosestPointToAxis(TransformGizmo::gameObject->transform.position, ApplyTransform(*TransformGizmo::translationAxis, TransformGizmo::gameObject->transform), editorCastHit.origin, editorCastHit.direction);
        TransformGizmo::grabOrigin = TransformGizmo::gameObject->transform.position;
        TransformGizmo::grabOffset = TransformGizmo::grabOrigin - closest;

      } else {
        System::rayCastHit = Raycast(origin, (ScreenToWorld(screen, 1) - origin).normalize());
        TransformGizmo::gameObject = System::rayCastHit? System::rayCastHit.object : nullptr;
      }
    }
  }

  else if(const auto* mouseButton = event.getIf<Event::MouseButtonReleased>()) {
    if(mouseButton->button == Mouse::Button::Left) {
      if(TransformGizmo::gameObject != nullptr) {
        TransformGizmo::HandleTranslationRelease();
      }
    }
  }

  else if(const auto* mouseWheelScroll = event.getIf<Event::MouseWheelScrolled>()) {
    Renderer::coneFaceCount += mouseWheelScroll->delta;
    Renderer::coneFaceCount = std::max((int)Renderer::coneFaceCount, 3);

    Renderer::meshs.back() = Mesh::GenerateCone(Renderer::coneFaceCount, 2, 1);
  }
}