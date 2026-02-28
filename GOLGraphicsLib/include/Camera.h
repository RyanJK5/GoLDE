#ifndef __Camera_h__
#define __Camera_h__

#include <glm/glm.hpp>

#include "Graphics2D.h"

namespace gol {
class GraphicsCamera {
public:
  static constexpr float MinZoom = 0.0001f;
  static constexpr float MaxZoom = 60.f;

public:
  float Zoom = 1.f;
  glm::vec2 Center = {};

public:
  GraphicsCamera() = default;
  GraphicsCamera(float zoom, glm::vec2 center) : Zoom(zoom), Center(center) {}

public:
  void ZoomBy(Vec2F screenPos, const RectF &viewBounds, float zoom);
  void Translate(glm::vec2 translation);

  glm::vec2 ScreenToWorldPos(Vec2F pos, const Rect &viewBounds) const;
  glm::vec2 WorldToScreenPos(Vec2F pos, const Rect &viewBounds,
                             Size2F worldSize) const;

  glm::mat4 OrthographicProjection(Size2 viewSize) const;
};
} // namespace gol

#endif