#include <Engine/Gizmos.hpp>
#include <Renderer.hpp>

#include <Engine/TransformGizmo.hpp>


void Gizmo::draw() const {
  float model[16];
  transform.getMatrix(model);
  Renderer::UnlitTriangleShader.SetUniform("model", glm::make_mat4(model));
  mesh.draw();
}


void (*DebugRadius::DebugDrawRadius)(const Vec3&, float, const Color&) = nullptr;


Line Gizmos::circle;
Mesh Gizmos::arrow;

std::vector<DebugRadius> Gizmos::debugRadiusCalls;
std::vector<Gizmo> Gizmos::gizmos;


void Gizmos::init() {
  DebugRadius::DebugDrawRadius = &DebugDrawRadius;

  { //Radius
    static const unsigned int pointC = 24;
    for(int i = 0; i < pointC; i++) {
      const float radians = (i / (float)pointC) * M_PI * 2;
      circle.vertices.push_back(Vec3(-cos(radians), sin(radians), 0));
    }

    for(int i = 0; i < pointC; i++) {
      circle.indecies.push_back(i);
      circle.indecies.push_back((i+1) % pointC);
    }

    circle.colors = std::vector<Color>(circle.indecies.size(), Color::Green);
  }

  { //Arrow
    float model[16];
    Transform transform(Vec3(), Quaternion::Euler(90,0,0));
    transform.getMatrix(model);
    glm::mat4 matrix = glm::make_mat4(model);

    const float height = .3f;
    arrow = Mesh::GenerateCone(8, height, .1f);
    for(Vec3& vert : arrow.vertices) {
      vert += Vec3up * (1+height/2.0f);
      glm::vec3 result = matrix * glm::vec4(vert.x, vert.y, vert.z, 1);
      vert = Vec3(result.x, result.y, result.z);
    }

    const float width = .05f;
    const float widthHalf = width/2.0f;
    
    std::vector<Vec3> vertices = {
      Vec3(-widthHalf, widthHalf,0), Vec3(-widthHalf, widthHalf,1), Vec3( widthHalf, widthHalf,1), Vec3( widthHalf, widthHalf,0),
      Vec3(-widthHalf,-widthHalf,0), Vec3(-widthHalf,-widthHalf,1), Vec3( widthHalf,-widthHalf,1), Vec3( widthHalf,-widthHalf,0)
    };
    std::vector<unsigned int> indecies = {0,1,2, 0,2,3, 0,4,5, 0,5,1, 3,2,6, 3,6,7, 4,7,6, 4,6,5, 0,3,7, 0,7,4};
    
    const unsigned int quadStart = arrow.vertices.size();
    for(const unsigned int val : indecies) arrow.indecies.push_back(quadStart + val);
    for(const Vec3& val : vertices) arrow.vertices.push_back(val);
  }


  TransformGizmo::init();
}
void Gizmos::update() {
  TransformGizmo::update();
}
void Gizmos::draw() {
  TransformGizmo::draw();
}


void Gizmos::DrawRotateTool(const GameObject* object) {
  float model[16];
  Transform transform(object->transform.position, Quaternion(), Vec3one*GetRaycastRadius(object->mesh));
  transform.getMatrix(model);
  Renderer::LineShader.SetUniform("model", glm::make_mat4(model));
  circle.colors = std::vector<Color>(circle.indecies.size(), Color::Blue);
  circle.draw();

  transform.rotation = Quaternion::Euler(90,0,0);
  transform.getMatrix(model);
  Renderer::LineShader.SetUniform("model", glm::make_mat4(model));    
  circle.colors = std::vector<Color>(circle.indecies.size(), Color::Green);
  circle.draw();

  transform.rotation = Quaternion::Euler(0,90,0);
  transform.getMatrix(model);
  Renderer::LineShader.SetUniform("model", glm::make_mat4(model));    
  circle.colors = std::vector<Color>(circle.indecies.size(), Color::Red);
  circle.draw();

  transform.rotation = Renderer::camera.rotation;
  transform.getMatrix(model);
  Renderer::LineShader.SetUniform("model", glm::make_mat4(model));    
  circle.colors = std::vector<Color>(circle.indecies.size(), Color::White);
  circle.draw();
}



void Gizmos::DebugDrawRadius(const Vec3& center, float radius, const Color& color) {
  static float model[16];

  Transform transform(center, Quaternion(), Vec3one*radius);
  transform.getMatrix(model);
  Renderer::LineShader.SetUniform("model", glm::make_mat4(model));
  circle.colors = std::vector<Color>(circle.indecies.size(), color);
  circle.draw();

  transform.rotation = Quaternion::Euler(90,0,0);
  transform.getMatrix(model);
  Renderer::LineShader.SetUniform("model", glm::make_mat4(model));    
  circle.draw();

  transform.rotation = Quaternion::Euler(0,90,0);
  transform.getMatrix(model);
  Renderer::LineShader.SetUniform("model", glm::make_mat4(model));    
  circle.draw();
}