#include <System.hpp>

#include <SFML/Graphics.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/Camera.hpp>

#include <GL/glew.h>

#include <iostream>


struct Vertex {
public:
  glm::vec3 position;
  glm::vec4 color;
};

int main(int argc, char *argv[]) {
  System::init();

  sf::RenderWindow window = sf::RenderWindow(sf::VideoMode{{800,600}}, "3D Rendering Library");
  glm::ivec2 windowSize = glm::ivec2{window.getSize().x, window.getSize().y};

  glewInit();

  Renderer::Shader unlitShader("Unlit");
  std::cout << "Shader Program ID: " << unlitShader.program << '\n';


  Renderer::Camera camera;

  GLuint vao;
  GLuint vbo;
  GLsizei indexCount = 0;
  { //gl objects
    std::vector<Vertex> vertices = {
      Vertex{{-1,-1,-3}, {1,0,0,1}},
      Vertex{{ 0, 1,-3}, {0,1,0,1}},
      Vertex{{ 1,-1,-3}, {0,0,1,1}},
    };
    indexCount = vertices.size();

    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);

    glNamedBufferData(vbo, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));


    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    glVertexArrayAttribFormat(vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribBinding(vao, 1, 0);
  } //gl objects


  while(window.isOpen()) {
    while(const auto &eventOpt = window.pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) window.close();
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) {
        window.setView(sf::View{sf::FloatRect{{0,0}, sf::Vector2f{resized->size}}});
      }
    }

    { //Update
      windowSize = glm::ivec2{window.getSize().x, window.getSize().y};

      glUseProgram(unlitShader.program);
      unlitShader.SetUniform("uProjection", camera.getProjectionMatrix(windowSize));
      unlitShader.SetUniform("uView", camera.getViewMatrix());
    } //Update

    { //Render
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(unlitShader.program);
      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, indexCount);

      window.display();
    } //Render
  }

  return 0;
}