#pragma once
#include <tyra>
#include "states/game/renderer/renderer_static_pair.hpp"

using Tyra::StaPipOptions;
using Tyra::StaticMesh;
using Tyra::Vec4;

namespace Demo {

class Projectile {
 public:
  Vec4 position;
  Vec4 velocity;
  int lifetime;
  bool alive;

  StaticMesh* mesh;
  StaPipOptions* options;
  RendererStaticPair* pair;

  Projectile();
  ~Projectile();

  void hide();
};

}  // namespace Demo
