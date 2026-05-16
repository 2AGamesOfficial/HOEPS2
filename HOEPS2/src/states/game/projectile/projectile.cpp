#include "states/game/projectile/projectile.hpp"

namespace Demo {

Projectile::Projectile() {
  position = Vec4(0.0F, -9999.0F, 0.0F);
  velocity = Vec4(0.0F, 0.0F, 0.0F);
  lifetime = 0;
  alive = false;
  mesh = nullptr;
  options = nullptr;
  pair = nullptr;
}

Projectile::~Projectile() {
  if (pair) delete pair;
  if (options) delete options;
  if (mesh) delete mesh;
}

void Projectile::hide() {
  alive = false;
  lifetime = 0;
  if (mesh) mesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
}

}  // namespace Demo
