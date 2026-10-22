#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Texture.hpp>


namespace Editor {
  namespace UI {
    class UIButton {
    public:
      UIButton(const glm::vec4 _rect = glm::vec4{0,0, 15,15}, const glm::vec4 _clr = glm::vec4(1)) {
        setRect(_rect);
        setColor(_clr);
      }


      void draw() const;


      inline void setRect(const glm::vec4 _rect) {
        setPosition(glm::vec2{_rect.x, _rect.y});
        setSize(glm::vec2{_rect.z, _rect.w});
      }
      inline void setPosition(const glm::vec2 _pos) {
        m_rect.position = sf::Vector2f{_pos.x, _pos.y};
      }
      inline void setSize(const glm::vec2 _siz) {
        m_rect.size = sf::Vector2f{_siz.x, _siz.y};
      }

      inline void setColor(const glm::vec4 _clr) {
        const glm::ivec4 ui8_clr = _clr * 255.0f;
        m_color = sf::Color{
          static_cast<uint8_t>(ui8_clr.r),
          static_cast<uint8_t>(ui8_clr.g),
          static_cast<uint8_t>(ui8_clr.b),
          static_cast<uint8_t>(ui8_clr.a)
        };
      }

      inline void setTexture(sf::Texture *_tex) {
        m_texture = _tex;
      }


      inline glm::vec4 getRect() {
        return glm::vec4{m_rect.position.x,m_rect.position.y, m_rect.size.x,m_rect.size.y};
      }
      inline glm::vec2 getPosition() {
        return glm::vec2{m_rect.position.x,m_rect.position.y};
      }
      inline glm::vec2 getSize() {
        return glm::vec2{m_rect.size.x,m_rect.size.y};
      }

      inline glm::vec4 getColor() {
        return glm::vec4{m_color.r,m_color.g, m_color.b,m_color.a} / 255.0f;
      }

      inline sf::Texture *getTexture() {
        return m_texture;
      }


    private:
      sf::FloatRect m_rect = sf::FloatRect{{0.0f,0.0f}, {15.0f,15.0f}};
      sf::Color m_color = sf::Color::White;
      sf::Texture *m_texture = nullptr;
    };
  }
}