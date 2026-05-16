#pragma once
#include <tyra>

using Tyra::CameraInfo3D;
using Tyra::Pad;
using Tyra::Vec4;

namespace Demo {

class Camera {
 public:
  Camera();
  ~Camera();

  Vec4 lookAt;
  Vec4 position;
  Vec4 up;

  CameraInfo3D getCameraInfo() { return CameraInfo3D(&position, &lookAt, &up); }
  // Center on a single point (solo or midpoint)
  void update(const Vec4& focusPoint, const float& terrainHeight, float zoomMul = 1.0F);

 private:
  float cameraHeight;
};

}  // namespace Demo
