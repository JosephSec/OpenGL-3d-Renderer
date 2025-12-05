#include <Renderer.hpp>
#include <System.hpp>
#include <User.hpp>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>


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

std::vector<GameObject> Renderer::objects;
std::vector<Mesh> Renderer::meshs;

std::vector<Light> Renderer::lights;

GameObject* Renderer::selected;
Line Renderer::line;

std::vector<DebugRadius> Renderer::debugRadiusCalls;


void (*DebugRadius::DebugDrawRadius)(const Vec3&, float, const Color&) = &Renderer::DebugDrawRadius;


static bool DrawLightRange = false;
static void ToggleDrawLightRange() {
  system("cls");
  DrawLightRange = !DrawLightRange;
  std::cout << "Draw Light Range: " << (DrawLightRange? "True" : "False");
}
void Renderer::init() {
  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antiAliasingLevel = 4;
  settings.majorVersion = 3;
  settings.minorVersion = 3;
  settings.attributeFlags = sf::ContextSettings::Core;

  window = new sf::RenderWindow(sf::VideoMode(Vec2(800,600)), "Window", sf::State::Windowed, settings);
  window->setFramerateLimit(120);
  window->requestFocus();

  initGL();
  UpdateViewMatrix();
  HandleResize();
  update();

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

  const std::vector<string> names = {"Pyramid", "Circle", "Triangle", "Quad", "Cube"};

  const float offset = ((meshs.size()-1) * 4)/2;
  for(int i = 0; i < meshs.size(); i++) {
    objects.push_back(GameObject(Transform(Vec3right * (i*4 - offset)), &meshs[i]));
    objects.back().name = names[i];
  }

  for(int i = 1; i < objects.size(); i++) objects[i].transform.rotation = Quaternion::Euler(90,0,0);


  lights.push_back(Light(Vec3(), Color::Red));
  // lights.push_back(Light(Vec3(), Color::Yellow));
  lights.push_back(Light(Vec3(), Color::Green));
  // lights.push_back(Light(Vec3(), Color::Cyan));
  lights.push_back(Light(Vec3(), Color::Blue));
  // lights.push_back(Light(Vec3(), Color::Magenta));
  // lights.push_back(Light(Vec3(), Color::White));


  static const unsigned int pointC = 24;
  for(int i = 0; i < pointC; i++) {
    const float radians = (i / (float)pointC) * M_PI * 2;
    line.vertices.push_back(Vec3(-cos(radians), sin(radians), 0));
  }

  for(int i = 0; i < pointC; i++) {
    line.indecies.push_back(i);
    line.indecies.push_back((i+1) % pointC);
  }

  line.colors = std::vector<Color>(line.indecies.size(), Color::Green);


  ToggleDrawLightRange();
}
void Renderer::initGL() {
  GLenum err = glewInit();
  if(err != GLEW_OK) std::cerr << "GLEW init error: " << glewGetErrorString(err) << '\n';

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);

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

  if(User::GetKeyDown(Keyboard::Key::X)) ToggleDrawLightRange();


  for(GameObject& object : objects) object.update();

  const float seg = M_PI * 2 / (float)lights.size();
  static float t = 0;
  t += .05f * System::deltaTime;
  const float radians = t * M_PI * 2;

  for(int i = 0; i < lights.size(); i++) {
    const float rVal = radians + seg * i;
    lights[i].transform.position = Vec3(cos(rVal) * 12, sin(rVal*5)*2, sin(rVal*5) * 4);

    lights[i].update();
  }
}
void Renderer::draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  { //Lit Shader
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

  { //Unlit Shader
    glUseProgram(UnlitTriangleShader);
    UnlitTriangleShader.SetUniform("projection", glm::make_mat4(projection));
    UnlitTriangleShader.SetUniform("view", glm::make_mat4(view));

    for(const Light& light : lights) {
      light.draw();
      if(DrawLightRange) DrawRadius(light.transform.position, light.range, light.color);
    }
  }

  { //Line Shader
    glUseProgram(LineShader);
    LineShader.SetUniform("projection", glm::make_mat4(projection));
    LineShader.SetUniform("view", glm::make_mat4(view));

    for(DebugRadius& debug : debugRadiusCalls) debug();
    debugRadiusCalls.clear();

    if(selected) {
      float model[16];
      Transform transform(selected->transform.position, Quaternion(), Vec3one*GetRaycastRadius(selected->mesh));
      transform.getMatrix(model);
      LineShader.SetUniform("model", glm::make_mat4(model));
      line.colors = std::vector<Color>(line.indecies.size(), Color::Blue);
      line.draw();

      transform.rotation = Quaternion::Euler(90,0,0);
      transform.getMatrix(model);
      LineShader.SetUniform("model", glm::make_mat4(model));    
      line.colors = std::vector<Color>(line.indecies.size(), Color::Green);
      line.draw();

      transform.rotation = Quaternion::Euler(0,90,0);
      transform.getMatrix(model);
      LineShader.SetUniform("model", glm::make_mat4(model));    
      line.colors = std::vector<Color>(line.indecies.size(), Color::Red);
      line.draw();

      transform.rotation = camera.rotation;
      transform.getMatrix(model);
      LineShader.SetUniform("model", glm::make_mat4(model));    
      line.colors = std::vector<Color>(line.indecies.size(), Color::White);
      line.draw();
    }
  }

  window->display();
}

void Renderer::clear() {
  glDeleteProgram(LitTriangleShader);
  glDeleteProgram(UnlitTriangleShader);
  glDeleteProgram(LineShader);

  delete window;
}