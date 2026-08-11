#include <Renderer.hpp>
#include <System.hpp>
#include <User.hpp>

#include <UI.hpp>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <Engine/Gizmos.hpp>
#include <Engine/TransformGizmo.hpp>


sf::RenderWindow* Renderer::window;
Vec2 Renderer::windowSize;
Vec2 Renderer::windowCenter;

sf::Font Renderer::font;
sf::Text Renderer::TextPrefab(font);


float Renderer::aspectRatio = 800.f / 600.f;
float Renderer::nearPlane = 0.1f;
float Renderer::farPlane = 50.f;
float Renderer::fov = 60.f * 3.14159f / 180.f;
float Renderer::f = 1.0f / std::tan(fov / 2.f);

float Renderer::projection[16];
float Renderer::view[16];

Transform Renderer::camera;


Shader Renderer::LitTriangleShader;
Shader Renderer::UnlitTriangleShader;
Shader Renderer::LineShader;

std::vector<Gizmo*> Renderer::gizmos;

std::vector<GameObject> Renderer::objects;
std::vector<Mesh> Renderer::meshs;

bool Renderer::DrawLightRange = false;
std::vector<Light> Renderer::lights;

unsigned int Renderer::coneFaceCount = 8;


void Renderer::init() {
  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antiAliasingLevel = 4;
  settings.majorVersion = 3;
  settings.minorVersion = 3;
  settings.attributeFlags = sf::ContextSettings::Default;

  window = new sf::RenderWindow(sf::VideoMode(Vec2(1600,900)), "Window", sf::State::Windowed, settings);
  window->setFramerateLimit(120);
  window->requestFocus();

  initGL();
  UpdateViewMatrix();
  update();
  HandleResize();

  if(!font.openFromFile(System::PATH+"/assets/Roboto.ttf"))
    Debug::error("font file missing or can't be loaded");
  else System::assets.push_back("Roboto.ttf");
  
  TextPrefab.setFont(font);
  TextPrefab.setString("[EMPTY STRING]");


  Mesh::InitPrimitives();
  meshs.push_back(Mesh::Pyramid);
  meshs.push_back(Mesh::Circle);
  meshs.push_back(Mesh::Triangle);
  meshs.push_back(Mesh::Quad);
  meshs.push_back(Mesh::Cube);
  meshs.push_back(Mesh::Cone);
  meshs.push_back(Mesh::GenerateCone(coneFaceCount, 2, 1));

  const std::vector<string> names = {"Pyramid", "Circle", "Triangle", "Quad", "Cube", "Cone", "Arrow Tip"};


  const float offset = ((meshs.size()-1) * 4)/2;
  for(int i = 0; i < meshs.size(); i++) {
    objects.push_back(GameObject(Transform(Vec3right * (i*4 - offset)), &meshs[i]));
    objects.back().name = names[i];
  }

  for(int i = 0; i < 3; i++) objects[i+1].transform.rotation = Quaternion::Euler(90,0,0);


  lights.push_back(Light(Vec3(), Color::Red));
  lights.push_back(Light(Vec3(), Color::Yellow));
  lights.push_back(Light(Vec3(), Color::Green));
  lights.push_back(Light(Vec3(), Color::Cyan));
  lights.push_back(Light(Vec3(), Color::Blue));
  lights.push_back(Light(Vec3(), Color::Magenta));
  lights.push_back(Light(Vec3(), Color::White));
}
void Renderer::initGL() {
  GLenum err = glewInit();
  if(err != GLEW_OK) std::cerr << "GLEW init error: " << glewGetErrorString(err) << '\n';

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);

  glClearColor(.1f,.1f,.15f, 1);

  LitTriangleShader = Shader("LitTriangle");
  UnlitTriangleShader = Shader("UnlitTriangle");
  LineShader = Shader("Line");

  { //Mesh VAO/VBO init
    glGenVertexArrays(1, &Mesh::VAO);
    glGenBuffers(1, &Mesh::VBO);

    glBindVertexArray(Mesh::VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh::VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  { //Line VAO/VBO init
    glGenVertexArrays(1, &Line::VAO);
    glGenBuffers(1, &Line::VBO);

    glBindVertexArray(Line::VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Line::VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}
void Renderer::update() {
  windowSize = window->getSize();
  windowCenter = windowSize/2;


  if(User::GetMouseButton(Mouse::Button::Right)) {
    static Vec2 mouseAnchor;
    static Vec2 mouseDelta;

    if(User::GetMouseButtonDown(Mouse::Button::Right)) mouseAnchor = User::mousePos;

    mouseDelta = User::mousePos-mouseAnchor;
    Mouse::setPosition(mouseAnchor, *Renderer::window);


    static const float moveSpeed = 3;
    static const float slowSpeed = .25f;
    static const float quickSpeed = 8;
    Vec3 moveDir;
    if(User::GetKey(Keyboard::Key::W)) moveDir += Vec3forward;
    if(User::GetKey(Keyboard::Key::S)) moveDir -= Vec3forward;
    if(User::GetKey(Keyboard::Key::A)) moveDir -= Vec3right;
    if(User::GetKey(Keyboard::Key::D)) moveDir += Vec3right;
    if(User::GetKey(Keyboard::Key::Q)) moveDir -= Vec3up;
    if(User::GetKey(Keyboard::Key::E)) moveDir += Vec3up;

    moveDir = Quaternion::Normalize(camera.rotation)*Quaternion(moveDir.normalize())*Quaternion::Normalize(Quaternion::Inverse(camera.rotation));
    if(User::GetKey(Keyboard::Key::LShift)) moveDir *= quickSpeed;
    else if(User::GetKey(Keyboard::Key::LControl)) moveDir *= slowSpeed;
    else moveDir *= moveSpeed;

    if(moveDir.magnitude() > 0) camera.position += moveDir * System::deltaTime;

    static Vec2 rot;
    rot.x += mouseDelta.y * System::deltaTime;
    rot.y += mouseDelta.x * System::deltaTime;

    camera.rotation = Quaternion::AngleAxis(rot.y, Vec3up) * Quaternion::AngleAxis(rot.x, Vec3right);

    Renderer::UpdateViewMatrix();
  }

  static unsigned int TEMP_coneFaceCount = coneFaceCount;
  if(TEMP_coneFaceCount != coneFaceCount) {
    coneFaceCount = std::max((unsigned int)3, coneFaceCount);
    meshs.back() = Mesh::GenerateCone(coneFaceCount, 2, 1);
  }


  for(GameObject& object : objects) object.update();

  { //Update Lights
    const float seg = M_PI * 2 / (float)lights.size();
    static float t = 0;
    t += .05f * System::deltaTime;
    const float radians = t * M_PI * 2;

    const float moveWidth = (objects.size()/2.0f) * 4;
    for(int i = 0; i < lights.size(); i++) {
      const float rVal = radians + seg * i;
      lights[i].transform.position = Vec3(cos(rVal) * moveWidth, sin(rVal*5)*2, sin(rVal*5) * 4);
      // lights[i].transform.position = Vec3(cos(rVal/3) * moveWidth, -sin(rVal*5)*4, cos(rVal*5) * 4);

      lights[i].update();
    }
  }
}
void Renderer::draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_CULL_FACE);
  
  { //Lit Objects
    glUseProgram(LitTriangleShader);
    LitTriangleShader.SetUniform("projection", glm::make_mat4(projection));
    LitTriangleShader.SetUniform("view", glm::make_mat4(view));
    

    LitTriangleShader.SetUniform("lightCount", (int)lights.size());
    
    for(int i = 0; i < lights.size(); i++) {
      string base = "lights[" + std::to_string(i) + "].";

      LitTriangleShader.SetUniform(base+"position", lights[i].transform.position);
      LitTriangleShader.SetUniform(base+"color", lights[i].getColorVec());
      LitTriangleShader.SetUniform(base+"range", lights[i].range);
    }

    for(const GameObject& object : objects) object.draw();
  }

  { //Unlit Objects
    glUseProgram(UnlitTriangleShader);
    UnlitTriangleShader.SetUniform("projection", glm::make_mat4(projection));
    UnlitTriangleShader.SetUniform("view", glm::make_mat4(view));

    for(const Light& light : lights) {
      light.draw();
      if(DrawLightRange) Gizmos::DrawRadius(light.transform.position, light.range, light.color);
    }

    if(System::rayCastHit) {
      float model[16];
      Transform transform(Vec3(), camera.rotation, Vec3one*.1f);

      Mesh mesh = Mesh::Circle;
      mesh.colors = std::vector<Color>(mesh.indecies.size(), Color::Red);

      for(const Vec3& vec : System::rayCastHit.intersects) {
        transform.position = vec;
        transform.getMatrix(model);
        UnlitTriangleShader.SetUniform("model", glm::make_mat4(model));
        mesh.draw();
      }
    }

    { //Gizmos
      Gizmos::draw();

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glClear(GL_DEPTH_BUFFER_BIT);

      for(const Gizmo& gizmo : Gizmos::gizmos) gizmo.draw();

      glDisable(GL_BLEND);
      glEnable(GL_DEPTH_TEST);
    }
  }

  { //Line Objects
    glUseProgram(LineShader);
    LineShader.SetUniform("projection", glm::make_mat4(projection));
    LineShader.SetUniform("view", glm::make_mat4(view));

    for(DebugRadius& debug : Gizmos::debugRadiusCalls) debug();
    Gizmos::debugRadiusCalls.clear();
  }
  
  {
    glDisable(GL_DEPTH_TEST);
    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    sf::RectangleShape shape(Vec2(200, windowSize.y));
    shape.setFillColor(Color(15,15,15));
    draw(shape);

    sf::Text text = TextPrefab;
    text.setPosition(Vec2one*10);
    text.setCharacterSize(15);
    text.setString("Hierarchy");
    draw(text);

    sf::VertexArray splitter(sf::PrimitiveType::Lines, 2);
    splitter[0] = sf::Vertex{Vec2(10,10 + text.getCharacterSize() + 5), Color(200,200,200)};
    splitter[1] = sf::Vertex{splitter[0].position + Vec2right * (shape.getSize().x - 20), Color(200,200,200)};
    draw(splitter);

    { //Draw Elements  
      const Vec2 listStart = Vec2(15, splitter[0].position.y + 15);
      const int charSize = text.getCharacterSize();
      const int lineSpacing = 10;
      bool mouseDown = User::GetMouseButtonDown(Mouse::Button::Left);
      
      sf::RectangleShape background(Vec2(splitter[1].position.x-splitter[0].position.x, charSize + lineSpacing/2.0f));      

      if(mouseDown && TransformGizmo::gameObject != nullptr) mouseDown = false;
      for(int i = 0; i < objects.size(); i++) {
        const Vec2 elementPos = listStart + Vec2up * (charSize + lineSpacing) * i;
        
        background.setPosition(elementPos - Vec2right * 5);

        const bool mouseIn = background.getGlobalBounds().contains(User::mousePos);
        background.setFillColor(mouseIn ? Color(100,100,100) : Color(40,40,40));
        draw(background);

        if(mouseIn && mouseDown) TransformGizmo::gameObject = &objects[i];

        text.setPosition(elementPos);
        text.setString(objects[i].name);
        draw(text);
      }
    }

    TransformGizmo::drawUI();


    UI::Manager::draw();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }

  window->display();
}

void Renderer::clear() {
  glDeleteProgram(LitTriangleShader);
  glDeleteProgram(UnlitTriangleShader);
  glDeleteProgram(LineShader);

  delete window;
}