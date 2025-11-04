#include "Camera.h"
#include "Logging.h"

void gol::Camera::ZoomBy(Vec2F screenPos, const RectF& viewBounds, float zoom)
{
    Zoom *= 1.f + zoom;
    if (Zoom < MaxZoom)
        Center += (ScreenToWorldPos(screenPos, viewBounds) - Center) * zoom;
    Zoom = std::min(Zoom, MaxZoom);
}

void gol::Camera::Translate(glm::vec2 delta)
{
    Center -= delta / Zoom;
}

glm::vec2 gol::Camera::ScreenToWorldPos(Vec2F cursor, const Rect& viewBounds) const
{
    glm::vec2 vec = {
        (cursor.X - viewBounds.X) ,
        (cursor.Y - viewBounds.Y) };
    glm::vec2 center = { viewBounds.Width / 2, viewBounds.Height / 2 };
    vec -= center;
    vec /= Zoom;
    vec += Center;
    return vec;
}

glm::mat4 gol::Camera::OrthographicProjection(Size2 viewSize) const
{
	return glm::ortho(
		Center.x - (viewSize.Width / 2.f)  / Zoom,
        Center.x + (viewSize.Width / 2.f)  / Zoom,
        Center.y + (viewSize.Height / 2.f) / Zoom,
        Center.y - (viewSize.Height / 2.f)  / Zoom,
		-1.f, 1.f
	);
}
