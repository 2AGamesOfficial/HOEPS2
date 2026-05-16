#include "states/game/pickup/pickup_manager.hpp"
#include <tyra>

using Tyra::FileUtils;
using Tyra::ObjLoader;
using Tyra::ObjLoaderOptions;
using Tyra::StaticMesh;
using Tyra::StaPipOptions;

namespace Demo {

PickupManager::PickupManager(Engine* t_engine) {
  engine = t_engine;
  pickupSound = engine->audio.adpcm.load(
      FileUtils::fromCwd("game/audio/gun_pickup.adpcm"));
}

PickupManager::~PickupManager() {
  for (auto* p : pickups) delete p;
  // pickupSound is engine-owned; do not delete
}

static void buildPickup(Pickup* p, PickupType type, Engine* engine) {
  p->type = type;
  p->active = false;
  p->spinAngle = 0.0F;
  p->bobPhase = 0.0F;
  p->basePosition = Vec4(0.0F, -9999.0F, 0.0F, 1.0F);

  const auto& data = PICKUP_DATA[type];
  ObjLoader loader;
  ObjLoaderOptions opts;
  opts.scale = data.scale;
  opts.flipUVs = true;
  auto meshData = loader.load(FileUtils::fromCwd(data.objPath), opts);
  meshData->loadNormals = false;
  p->mesh = new StaticMesh(meshData.get());
  engine->renderer.getTextureRepository().addByMesh(
      p->mesh, FileUtils::fromCwd(data.texDir), "png");
  p->mesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
  p->options = new StaPipOptions();
  p->options->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
  p->pair = new RendererStaticPair{p->mesh, p->options};

  ObjLoaderOptions glowOpts;
  glowOpts.scale = 80.0F;
  glowOpts.flipUVs = true;
  const char* glowObj = (type == PICKUP_HEALTH)
      ? "game/models/pickups_green/pickup_glow_green.obj"
      : "game/models/pickups/pickup_glow.obj";
  const char* glowDir = (type == PICKUP_HEALTH)
      ? "game/models/pickups_green/"
      : "game/models/pickups/";
  auto glowData = loader.load(FileUtils::fromCwd(glowObj), glowOpts);
  glowData->loadNormals = false;
  p->glowMesh = new StaticMesh(glowData.get());
  engine->renderer.getTextureRepository().addByMesh(
      p->glowMesh, FileUtils::fromCwd(glowDir), "png");
  p->glowMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
  p->glowOptions = new StaPipOptions();
  p->glowOptions->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
  p->glowOptions->blendingEnabled = true;
  p->glowPair = new RendererStaticPair{p->glowMesh, p->glowOptions};
}

void PickupManager::init() {
  // Pre-allocate POOL_PER_TYPE pickups for each type, all inactive
  static const int POOL_PER_TYPE = 2;
  for (int t = 0; t < PICKUP_TYPE_COUNT; t++) {
    for (int n = 0; n < POOL_PER_TYPE; n++) {
      auto* p = new Pickup();
      buildPickup(p, (PickupType)t, engine);
      pickups.push_back(p);
    }
  }
}

void PickupManager::spawn(PickupType type, const Vec4& position) {
  // Find an inactive pickup of this type and reuse it
  for (auto* p : pickups) {
    if (p->type == type && !p->active) {
      p->basePosition = position;
      p->active = true;
      p->spinAngle = 0.0F;
      p->bobPhase = 0.0F;
      p->lifetime = 600;  // 10 seconds @ 60 FPS
      p->mesh->setPosition(position);
      p->glowMesh->setPosition(Vec4(position.x, 5.0F, position.z));
      return;
    }
  }
  // Pool exhausted — silently drop
}

int PickupManager::update(const Vec4& playerPosition, const Player* player) {
  int picked = -1;
  for (auto* p : pickups) {
    if (!p->active) continue;

    // Lifetime expiry
    if (p->lifetime > 0) {
      p->lifetime--;
      if (p->lifetime == 0) {
        p->active = false;
        p->mesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
        p->glowMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
        continue;
      }
    }

    const auto& data = PICKUP_DATA[p->type];

    // Spin + bob
    p->spinAngle += data.spinSpeed;
    p->bobPhase += 0.08F;
    float bobY = sinf(p->bobPhase) * data.bobHeight;

    Vec4 pos = p->basePosition;
    pos.y += bobY;
    p->mesh->setPosition(pos);
    p->mesh->rotation.identity();
    p->mesh->rotation.rotateY(p->spinAngle);

    // Glow: stay flat on ground, rotate opposite direction
    p->glowMesh->setPosition(Vec4(p->basePosition.x, 5.0F, p->basePosition.z));
    p->glowMesh->rotation.identity();
    p->glowMesh->rotation.rotateY(-p->spinAngle * 0.5F);

    // Proximity check (XZ distance only)
    float dx = playerPosition.x - p->basePosition.x;
    float dz = playerPosition.z - p->basePosition.z;
    if (dx*dx + dz*dz < PICKUP_RADIUS * PICKUP_RADIUS) {
      // Block weapon pickup if player already owns it
      if (p->type == PICKUP_WEAPON_SMG && player->hasWeapon(WEAPON_SMG)) continue;
      if (p->type == PICKUP_WEAPON_SHOTGUN && player->hasWeapon(WEAPON_SHOTGUN)) continue;
      if (p->type == PICKUP_WEAPON_GRENADE_LAUNCHER && player->hasWeapon(WEAPON_GRENADE_LAUNCHER)) continue;
      if (p->type == PICKUP_WEAPON_GUNCON && player->hasWeapon(WEAPON_GUNCON)) continue;
      p->active = false;
      p->mesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
      p->glowMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
      picked = (int)p->type;
      engine->audio.adpcm.playPooled(pickupSound, 14, 2);
    }
  }
  return picked;
}

void PickupManager::clearAll() {
  // Hide all pickups (pool retained)
  for (auto* p : pickups) {
    p->active = false;
    p->mesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
    p->glowMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
  }
}

std::vector<RendererStaticPair*> PickupManager::getActivePairs() const {
  std::vector<RendererStaticPair*> result;
  for (auto* p : pickups) {
    if (p->active) {
      result.push_back(p->glowPair);  // render glow first so pickup sits on top
      result.push_back(p->pair);
    }
  }
  return result;
}

}  // namespace Demo
