#include <Engine/TransformGizmo.hpp>
#include <System.hpp>
#include <User.hpp>
#include <Renderer.hpp>

#include <Engine/Gizmos.hpp>


GameObject* TransformGizmo::gameObject;

Gizmo TransformGizmo::translationGizmo;

std::optional<Vec3> TransformGizmo::translationAxis;
Vec3 TransformGizmo::grabOrigin;
Vec3 TransformGizmo::grabOffset;


void TransformGizmo::init() {
  const unsigned int A = 0;
  Transform meshTransform;
  Mesh translationMesh;

  translationMesh = Gizmos::arrow;
  for(Vec3& vert : translationMesh.vertices) ApplyTransform(vert, meshTransform, false);
  translationMesh.colors = std::vector<Color>(translationMesh.indecies.size(), Color(0,0,255,200));
  const unsigned int B = translationMesh.vertices.size();

  meshTransform.rotation = Quaternion::Euler(-90,0,0);
  for(const Vec3& vert : Gizmos::arrow.vertices) {
    translationMesh.vertices.push_back(ApplyTransform(vert, meshTransform, false));
  }
  for(const uint32_t index : Gizmos::arrow.indecies) {
    translationMesh.indecies.push_back(B + index);
    translationMesh.colors.push_back(Color(0,255,0,200));
  }
  const unsigned int C = translationMesh.vertices.size();

  meshTransform.rotation = Quaternion::Euler(-90,0,-90);
  for(const Vec3& vert : Gizmos::arrow.vertices) {
    translationMesh.vertices.push_back(ApplyTransform(vert, meshTransform, false));
  }
  for(const uint32_t index : Gizmos::arrow.indecies) {
    translationMesh.indecies.push_back(C + index);
    translationMesh.colors.push_back(Color(255,0,0,200));
  }


  translationGizmo.mesh = translationMesh;
}
void TransformGizmo::update() {
  if(gameObject != nullptr) {
    translationGizmo.transform = gameObject->transform;

    if(translationAxis.has_value()) {
      const Vec3 origin = ScreenToWorld(User::mousePos, 0);
      const Vec3 direction = (ScreenToWorld(User::mousePos, 1) - origin).normalize();
      Vec3 closestPoint = GetClosestPointToAxis(translationGizmo.transform.position, ApplyTransform(*translationAxis, gameObject->transform), origin, direction);
      gameObject->transform.position = closestPoint + grabOffset;
    }
  }
}
void TransformGizmo::draw() {
  if(!gameObject) return;

  Gizmos::gizmos.push_back(translationGizmo);
}
void TransformGizmo::drawUI() {
  sf::RectangleShape background(Vec2(200, Renderer::windowSize.y));
  background.setPosition(Vec2(Renderer::windowSize.x - 200, 0));
  background.setFillColor(Color(15,15,15));
  Renderer::draw(background);

  sf::Text text = Renderer::TextPrefab;
  text.setPosition(background.getPosition() + Vec2one*10);
  text.setCharacterSize(15);
  text.setString("Selected GameObject");
  Renderer::draw(text);

  sf::VertexArray splitter(sf::PrimitiveType::Lines, 2);
  splitter[0] = sf::Vertex{background.getPosition() + Vec2(5,10 + text.getCharacterSize() + 5), Color(200,200,200)};
  splitter[1] = sf::Vertex{splitter[0].position + Vec2right * (background.getSize().x - 10), Color(200,200,200)};
  Renderer::draw(splitter);

  const unsigned int charSize = text.getCharacterSize();
  { //Name
    const Vec2 namePosition = splitter[0].position + Vec2(5, 15);
    text.setPosition(namePosition);
    text.setString("Name: ");
    Renderer::draw(text);

    if(gameObject != nullptr) {
      text.setString(gameObject->name);
      text.setPosition(Vec2(Renderer::windowSize.x - 15 - text.getLocalBounds().size.x, namePosition.y));
      Renderer::draw(text);
    }
  }

  { //Position
    const Vec2 posPosition = splitter[0].position + Vec2(5, 15 + (charSize + 10));
    text.setPosition(posPosition);
    text.setString("Position: ");
    Renderer::draw(text);

    if(gameObject != nullptr) {
      std::stringstream ss; ss << gameObject->transform.position.precision(100);
      text.setString(ss.str());
      text.setPosition(Vec2(Renderer::windowSize.x - 15 - text.getLocalBounds().size.x, posPosition.y));
      Renderer::draw(text);
    }
  }

  { //Rotation
    const Vec2 rotPosition = splitter[0].position + Vec2(5, 15 + (charSize + 10) * 2);
    text.setPosition(rotPosition);
    text.setString("Rotation: ");
    Renderer::draw(text);

    if(gameObject != nullptr) {
      std::stringstream ss; ss << gameObject->transform.rotation.getEuler().precision(100);
      text.setString(ss.str());
      text.setPosition(Vec2(Renderer::windowSize.x - 15 - text.getLocalBounds().size.x, rotPosition.y));
      Renderer::draw(text);
    }
  }

  { //Scale
    const Vec2 rotPosition = splitter[0].position + Vec2(5, 15 + (charSize + 10) * 3);
    text.setPosition(rotPosition);
    text.setString("Scale: ");
    Renderer::draw(text);

    if(gameObject != nullptr) {
      std::stringstream ss; ss << gameObject->transform.scale.precision(100);
      text.setString(ss.str());
      text.setPosition(Vec2(Renderer::windowSize.x - 15 - text.getLocalBounds().size.x, rotPosition.y));
      Renderer::draw(text);
    }
  }
}


void TransformGizmo::HandleTranslationPress(uint32_t faceIndex) {
  const unsigned int arrowVertC = translationGizmo.mesh.vertices.size()/3;
  const unsigned int arrowVertexC = translationGizmo.mesh.indecies.size()/3;
  const unsigned int arrowIndex = faceIndex / arrowVertC;
  
  for(Color& color : translationGizmo.mesh.colors) color.a = 50;
  for(int i = 0; i < arrowVertexC; i++) {
    translationGizmo.mesh.colors[arrowIndex*arrowVertexC + i].a = 255;
  }

  translationAxis = GetTranslateDir(arrowIndex);
}
void TransformGizmo::HandleTranslationRelease() {  
  TransformGizmo::translationAxis.reset();
  for(Color& color : translationGizmo.mesh.colors) color.a = 200;  
}