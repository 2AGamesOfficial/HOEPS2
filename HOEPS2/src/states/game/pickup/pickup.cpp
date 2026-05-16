#include "states/game/pickup/pickup.hpp"

namespace Demo {

Pickup::Pickup() {
  type = PICKUP_WEAPON_SMG;
  basePosition = Vec4(0.0F, -9999.0F, 0.0F);
  active = false;
  spinAngle = 0.0F;
  bobPhase = 0.0F;
  lifetime = 0;
  mesh = nullptr;
  options = nullptr;
  pair = nullptr;
  glowMesh = nullptr;
  glowOptions = nullptr;
  glowPair = nullptr;
}

Pickup::~Pickup() {
  if (pair) delete pair;
  if (options) delete options;
  if (mesh) delete mesh;
  if (glowPair) delete glowPair;
  if (glowOptions) delete glowOptions;
  if (glowMesh) delete glowMesh;
}

}  // namespace Demo
