#include "states/game/player/camera.hpp"

namespace Demo {

Camera::Camera() : lookAt(0.0F), position(0.0F), up(0.0F, 1.0F, 0.0F) {
  cameraHeight = 600.0F;
}

Camera::~Camera() {}

void Camera::update(const Vec4& focusPoint, const float& terrainHeight, float zoomMul) {
  position.x = focusPoint.x;
  position.y = focusPoint.y + cameraHeight * zoomMul;
  position.z = focusPoint.z + 1.0F;

  lookAt.x = focusPoint.x;
  lookAt.y = focusPoint.y;
  lookAt.z = focusPoint.z;
}

}  // namespace Demo
