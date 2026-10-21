#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>

#include <Renderer/API.hpp>


namespace Renderer {
  class RENDER3D_API LineGraph {
  public:
    LineGraph(int _size, float _min = 0, float _max = 1);
    ~LineGraph();


    void log() const noexcept;

    void update(float _x, float _dt);
    void draw(const sf::FloatRect &_rect, const sf::Color _color) const;

    void resize(int _size);
    void setTimerLength(float _seconds);

    inline int getSize() const noexcept {
      return m_size;
    }
    inline float getValue(int _index = -1) const noexcept {
      return m_data[_index > -1? _index : (m_size - 1)];
    }
    sf::Vector2f getPlotPosition(const sf::FloatRect &_rect, int _index) const;


    float min = 0;
    float max = 1;


  private:
    void plot(float _x);

    float m_plotTimeLength = 1;
    float m_plotTimeRemaining = m_plotTimeLength;

    int m_size = 0;
    float *m_data = nullptr;
  };
}