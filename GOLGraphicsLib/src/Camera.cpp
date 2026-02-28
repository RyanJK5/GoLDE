#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <print>

#include "Camera.h"
#include "Graphics2D.h"

void gol::GraphicsCamera::ZoomBy(Vec2F screenPos, const RectF &viewBounds,
                                 float zoom) {
  Zoom *= 1.f + zoom;
  if (Zoom < MaxZoom)
    Center += (ScreenToWorldPos(screenPos, viewBounds) - Center) * zoom;
  Zoom = std::clamp(Zoom, MinZoom, MaxZoom);
}

void gol::GraphicsCamera::Translate(glm::vec2 delta) { Center -= delta / Zoom; }

glm::vec2 gol::GraphicsCamera::ScreenToWorldPos(Vec2F pos,
                                                const Rect &viewBounds) const {
  auto vec = glm::vec2{pos.X - viewBounds.X, pos.Y - viewBounds.Y};
  vec -= glm::vec2{viewBounds.Width / 2, viewBounds.Height / 2};
  vec /= Zoom;
  vec += Center;
  return vec;
}

glm::vec2 gol::GraphicsCamera::WorldToScreenPos(Vec2F pos,
                                                const Rect &viewBounds,
                                                Size2F worldSize) const {
  auto vec = glm::vec2(pos);
  vec.x -= Center.x;
  vec.y += Center.y;
  vec *= Zoom;
  vec += glm::vec2{viewBounds.Width / 2.f,
                   viewBounds.Height / 2.f - worldSize.Height * Zoom};
  return vec;
}

glm::mat4 gol::GraphicsCamera::OrthographicProjection(Size2 viewSize) const {
  return glm::ortho(Center.x - (viewSize.Width / 2.f) / Zoom,
                    Center.x + (viewSize.Width / 2.f) / Zoom,
                    Center.y + (viewSize.Height / 2.f) / Zoom,
                    Center.y - (viewSize.Height / 2.f) / Zoom, -1.f, 1.f);
}
