#include <Renderer/Debug/Graph.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>

#include <iostream>
#include <memory>


LineGraph::LineGraph(int _size, float _min, float _max) : m_size(_size), min(_min), max(_max) {
  m_data = reinterpret_cast<float*>(malloc(m_size * sizeof(float)));
  memset(m_data, 0, m_size * sizeof(float));
}
LineGraph::~LineGraph() {
  free(m_data);
}


void LineGraph::log() const noexcept {
  std::stringstream ss;

  for(int i = 0; i < m_size; i++) ss << "[" << i << "]: " << m_data[i] << '\n';

  std::cout << ss.str();
}

void LineGraph::update(float _x, float _dt) {
  m_plotTimeRemaining -= _dt;
  if(m_plotTimeRemaining <= 0) {
    plot(_x);
    m_plotTimeRemaining = m_plotTimeLength;
  }
}
void LineGraph::draw(const sf::FloatRect &_rect, const sf::Color _color) const {
  const glm::ivec2 &winSize = Renderer::Core::windowSize;

  sf::VertexArray line(sf::PrimitiveType::LineStrip, m_size);
  for(int i = 0; i < m_size; i++) {
    line[i] = sf::Vertex{getPlotPosition(_rect, i), _color};
  }
  Renderer::Core::window->draw(line);
}

void LineGraph::resize(int _size) {
  float *newData = reinterpret_cast<float*>(realloc(m_data, _size));
  if(newData == nullptr) free(m_data);

  m_data = newData;
}
void LineGraph::setTimerLength(float _seconds) {
  m_plotTimeLength = _seconds;
}

sf::Vector2f LineGraph::getPlotPosition(const sf::FloatRect &_rect, int _index) const {
  const float val = std::min<float>(max, std::max<float>(min, m_data[_index]));
  const float t = (val - min) / max;
  return _rect.position + sf::Vector2f{_rect.size.x / (m_size - 1) * _index, (1 - t) * _rect.size.y};
}


void LineGraph::plot(float _x) {
  memmove(m_data, m_data + 1, (m_size - 1) * sizeof(float));
  m_data[m_size - 1] = _x;
}