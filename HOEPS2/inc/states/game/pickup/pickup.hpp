#pragma once
#include <tyra>
#include "states/game/renderer/renderer_static_pair.hpp"
#include "./pickup_type.hpp"

using Tyra::StaPipOptions;
using Tyra::StaticMesh;
using Tyra::Vec4;

namespace Demo {

class Pickup {
 public:
  PickupType type;
  Vec4 basePosition;   // ground position (excl. bob)
  bool active;
  float spinAngle;
  float bobPhase;
  int lifetime;  // frames remaining; 0 = no expiry

  StaticMesh* mesh;
  StaPipOptions* options;
  RendererStaticPair* pair;

  StaticMesh* glowMesh;
  StaPipOptions* glowOptions;
  RendererStaticPair* glowPair;

  Pickup();
  ~Pickup();
};

}  // namespace Demo
