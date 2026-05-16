#include "states/game/enemy/enemy_manager.hpp"
#include <functional>
#include "game_settings.hpp"
#include "states/game/colliders.hpp"

using Tyra::FileUtils;
using Tyra::Math;
using Tyra::ObjLoader;
using Tyra::ObjLoaderOptions;

namespace Demo {

EnemyManager::EnemyManager(Engine* engine, const Heightmap& heightmap) {
  textureRepo = &engine->renderer.core.texture.repository;

  ObjLoader loader;

  // Load ophanim
  {
    ObjLoaderOptions opts;
    opts.animation.count = 8;
    opts.flipUVs = true;
    opts.scale = 250.0F;
    auto data = loader.load(FileUtils::fromCwd("game/models/ophanim/ophanim.obj"), opts);
    data->loadNormals = false;
    ophanimMotherMesh = new DynamicMesh(data.get());
    ophanimBodyTexture = textureRepo->add(FileUtils::fromCwd("game/models/ophanim/OphanimAlbedo.png"));
    ophanimClothTexture = textureRepo->add(FileUtils::fromCwd("game/models/ophanim/OphanimEyeAlbedo.png"));
  }

  // Load demon upper body
  {
    ObjLoaderOptions opts;
    opts.scale = 25.0F;
    opts.flipUVs = false;
    auto data = loader.load(FileUtils::fromCwd("game/models/demon/demon.obj"), opts);
    data->loadNormals = false;
    demonUpperMotherMesh = new StaticMesh(data.get());
    demonUpperTex1 = textureRepo->add(FileUtils::fromCwd("game/models/demon/DemonTorso.png"));
    demonUpperTex2 = textureRepo->add(FileUtils::fromCwd("game/models/demon/Horn.png"));
  }

  // Load demon legs
  {
    ObjLoaderOptions opts;
    opts.animation.count = 13;
    opts.scale = 25.0F;
    opts.flipUVs = false;
    auto data = loader.load(FileUtils::fromCwd("game/models/demon/demon_legs.obj"), opts);
    data->loadNormals = false;
    demonLegsMotherMesh = new DynamicMesh(data.get());
    demonLegsTex = textureRepo->add(FileUtils::fromCwd("game/models/demon/DemonLegs.png"));
  }

  // Load shared pentagram once (3 enemies = 3x fewer USB reads)
  {
    ObjLoaderOptions opts;
    opts.scale = 80.0F;
    opts.flipUVs = true;
    auto data = loader.load(FileUtils::fromCwd("game/models/pentagram/pentagram.obj"), opts);
    data->loadNormals = false;
    pentagramMotherMesh = new StaticMesh(data.get());
    textureRepo->addByMesh(pentagramMotherMesh, FileUtils::fromCwd("game/models/pentagram/"), "png");
    pentagramTex = nullptr;
    for (auto* tex : *textureRepo->getAll()) {
      if (tex->name == "Pentagram.png") { pentagramTex = tex; break; }
    }
  }

  // Load shared blood splash
  {
    ObjLoaderOptions opts;
    opts.scale = 150.0F;
    opts.flipUVs = true;
    auto data = loader.load(FileUtils::fromCwd("game/models/blood/blood.obj"), opts);
    data->loadNormals = false;
    bloodMotherMesh = new StaticMesh(data.get());
    textureRepo->addByMesh(bloodMotherMesh, FileUtils::fromCwd("game/models/blood/"), "png");
    bloodTex = nullptr;
    for (auto* tex : *textureRepo->getAll()) {
      if (tex->name == "Blood.png") { bloodTex = tex; break; }
    }
  }

  auto* punch = engine->audio.adpcm.load(FileUtils::fromCwd("game/models/zombie/punch.adpcm"));
  auto* death = engine->audio.adpcm.load(FileUtils::fromCwd("game/models/zombie/death.adpcm"));
  auto* hit = engine->audio.adpcm.load(FileUtils::fromCwd("game/audio/enemy_hit.adpcm"));
  auto* demonDeath = engine->audio.adpcm.load(FileUtils::fromCwd("game/audio/demon_death.adpcm"));

  const int enemyCount = 4;  // always load 4, gate activity via preSpawnTimer
  activeCount = 2;
  preSpawnMin = 60;
  preSpawnMax = 180;
  for (int i = 0; i < enemyCount; i++) {
    EnemyInfo info;
    info.adpcmChannel = 9 + i;
    info.adpcmPunch = punch;
    info.adpcmDeath = death;
    info.adpcmHit = hit;
    info.adpcmDemonDeath = demonDeath;
    info.terrainLeftUp = heightmap.leftUp;
    info.terrainRightDown = heightmap.rightDown;

    // Every enemy gets ALL mesh sets; enemyType picks the starting active one.
    info.motherMesh = ophanimMotherMesh;
    info.bodyTexture = ophanimBodyTexture;
    info.clothTexture = ophanimClothTexture;
    info.upperMotherMesh = demonUpperMotherMesh;
    info.upperTex1 = demonUpperTex1;
    info.upperTex2 = demonUpperTex2;
    info.legsMotherMesh = demonLegsMotherMesh;
    info.legsTex = demonLegsTex;
    info.pentagramMotherMesh = pentagramMotherMesh;
    info.pentagramTex = pentagramTex;
    info.bloodMotherMesh = bloodMotherMesh;
    info.bloodTex = bloodTex;
    enemies.push_back(new Enemy(engine, info));
  }
  // Park enemies beyond activeCount (slots 2, 3 initially)
  for (int i = activeCount; i < (int)enemies.size(); i++) {
    enemies[i]->ophanimMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
    enemies[i]->demonLegsMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
    enemies[i]->demonUpperMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
    enemies[i]->pentagramMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
    enemies[i]->preSpawnTimer = 999999;  // effectively disabled
    enemies[i]->spawnTimer = 0;
  }
}

void EnemyManager::setDifficulty(int kills) {
  // Ophanim rarity: 0 before 5 kills, 10% up to 15, 30% after
  if (kills < 5) Enemy::currentOphanimChance = 0.0F;
  else if (kills < 15) Enemy::currentOphanimChance = 0.1F;
  else Enemy::currentOphanimChance = 0.3F;
  // Every 5 kills shortens preSpawn by 10% (floor 30-90)
  int tier = kills / 5;
  float factor = 1.0F;
  for (int i = 0; i < tier; i++) factor *= 0.9F;
  preSpawnMin = (int)(60.0F * factor);
  if (preSpawnMin < 30) preSpawnMin = 30;
  preSpawnMax = (int)(180.0F * factor);
  if (preSpawnMax < 90) preSpawnMax = 90;
  // Unlock thresholds
  int unlocked = 2;
  if (kills >= 10) unlocked = 3;
  if (kills >= 20) unlocked = 4;
  if (unlocked > (int)enemies.size()) unlocked = (int)enemies.size();
  // Wake new slots: set preSpawnTimer to a short random value
  for (int i = activeCount; i < unlocked; i++) {
    enemies[i]->preSpawnTimer = (int)Math::randomf(30.0F, 120.0F);
  }
  activeCount = unlocked;
}

EnemyManager::~EnemyManager() {
  for (auto* enemy : enemies) delete enemy;
  delete ophanimMotherMesh;
  delete demonUpperMotherMesh;
  delete demonLegsMotherMesh;
  textureRepo->free(ophanimBodyTexture->id);
  textureRepo->free(ophanimClothTexture->id);
  textureRepo->free(demonUpperTex1->id);
  textureRepo->free(demonUpperTex2->id);
  textureRepo->free(demonLegsTex->id);
}

const std::vector<RendererDynamicPair*>& EnemyManager::getPairs() {
  _cachedDynPairs.clear();
  for (auto* enemy : enemies) _cachedDynPairs.push_back(enemy->pair);
  return _cachedDynPairs;
}

const std::vector<RendererStaticPair*>& EnemyManager::getPentagramPairs() {
  _cachedPentagramPairs.clear();
  for (auto* enemy : enemies) _cachedPentagramPairs.push_back(enemy->pentagramPair);
  return _cachedPentagramPairs;
}

const std::vector<RendererStaticPair*>& EnemyManager::getUpperBodyPairs() {
  _cachedUpperBodyPairs.clear();
  for (auto* enemy : enemies) {
    if (enemy->currentType == ENEMY_DEMON) _cachedUpperBodyPairs.push_back(enemy->demonUpperPair);
  }
  return _cachedUpperBodyPairs;
}

const std::vector<RendererStaticPair*>& EnemyManager::getBloodPairs() {
  _cachedBloodPairs.clear();
  for (auto* enemy : enemies) {
    if (enemy->bloodTimer > 0) _cachedBloodPairs.push_back(enemy->bloodPair);
  }
  return _cachedBloodPairs;
}

void EnemyManager::update(const Heightmap& heightmap,
                          const Vec4& playerPosition,
                          const PlayerShootAction& shootAction) {
  Enemy::lastKnownPlayerPos = playerPosition;
  (void)shootAction;
  for (auto* enemy : enemies) {
    PlayerShootAction noShoot;
    noShoot.isShooting = false;
    enemy->update(heightmap, playerPosition, noShoot);
  }
}

float EnemyManager::applyShot(const Tyra::Ray& ray, int damage) {
  float closestObstacleDist = 1e9f;
  for (int i = 0; i < NUM_OBSTACLES; i++) {
    if (!OBSTACLES[i].blocksProjectiles) continue;
    Vec4 boxMin(OBSTACLES[i].minX, OBSTACLES[i].minY, OBSTACLES[i].minZ, 1.0F);
    Vec4 boxMax(OBSTACLES[i].maxX, OBSTACLES[i].maxY, OBSTACLES[i].maxZ, 1.0F);
    float dist = 0.0F;
    if (ray.intersectBox(boxMin, boxMax, &dist)) {
      if (dist < closestObstacleDist) closestObstacleDist = dist;
    }
  }
  Enemy* closestHit = nullptr;
  float closestEnemyDist = 1e9f;
  for (auto* enemy : enemies) {
    if (enemy->spawnTimer > 0) continue;
    float dist = 0.0F;
    if (enemy->currentType == ENEMY_DEMON) {
      auto bbox = enemy->mesh->getCurrentBoundingBox().getTransformed(enemy->mesh->getModelMatrix());
      // Flatten bbox Y to match ray Y=0
      Vec4 bmin(bbox.min().x, -10.0F, bbox.min().z, 1.0F);
      Vec4 bmax(bbox.max().x, 10.0F, bbox.max().z, 1.0F);
      if (!ray.intersectBox(bmin, bmax, &dist)) continue;
    } else {
      auto bbox = enemy->mesh->getCurrentBoundingBox().getTransformed(enemy->mesh->getModelMatrix());
      if (!ray.intersectBox(bbox.min(), bbox.max(), &dist)) continue;
    }
    if (dist < closestEnemyDist && dist < closestObstacleDist) {
      closestEnemyDist = dist;
      closestHit = enemy;
    }
  }
  float hitDist = (closestHit != nullptr) ? closestEnemyDist : closestObstacleDist;
  lastShotDistance = hitDist;
  lastShotHitEnemy = (closestHit != nullptr);
  if (closestHit != nullptr) closestHit->takeDamage(damage);
  return hitDist;
}

float EnemyManager::applyShotPiercing(const Tyra::Ray& ray, int damage) {
  float closestObstacleDist = 1e9f;
  for (int i = 0; i < NUM_OBSTACLES; i++) {
    if (!OBSTACLES[i].blocksProjectiles) continue;
    Vec4 boxMin(OBSTACLES[i].minX, OBSTACLES[i].minY, OBSTACLES[i].minZ, 1.0F);
    Vec4 boxMax(OBSTACLES[i].maxX, OBSTACLES[i].maxY, OBSTACLES[i].maxZ, 1.0F);
    float dist = 0.0F;
    if (ray.intersectBox(boxMin, boxMax, &dist)) {
      if (dist < closestObstacleDist) closestObstacleDist = dist;
    }
  }
  bool hitAny = false;
  for (auto* enemy : enemies) {
    if (enemy->spawnTimer > 0) continue;
    auto bbox = enemy->mesh->getCurrentBoundingBox().getTransformed(enemy->mesh->getModelMatrix());
    Vec4 bmin = bbox.min();
    Vec4 bmax = bbox.max();
    if (enemy->currentType == ENEMY_DEMON) {
      bmin.y = -10.0F;
      bmax.y = 10.0F;
    }
    float dist = 0.0F;
    if (ray.intersectBox(bmin, bmax, &dist)) {
      if (dist < closestObstacleDist) {
        enemy->takeDamage(damage);
        hitAny = true;
      }
    }
  }
  lastShotHitEnemy = hitAny;
  return closestObstacleDist;
}

void EnemyManager::applyAoeDamage(const Tyra::Vec4& center, float radius, int damage) {
  float r2 = radius * radius;
  for (auto* enemy : enemies) {
    auto* ep = enemy->mesh->getPosition();
    float dx = ep->x - center.x;
    float dz = ep->z - center.z;
    if (dx*dx + dz*dz <= r2) enemy->takeDamage(damage);
  }
}

void EnemyManager::update(const Heightmap& heightmap,
                          const Vec4& p1Position, bool p1Alive,
                          const Vec4& p2Position, bool p2Alive,
                          const PlayerShootAction& shootAction) {
  if (p1Alive && p2Alive) {
    Enemy::lastKnownPlayerPos = Vec4(
        (p1Position.x + p2Position.x) * 0.5F,
        p1Position.y,
        (p1Position.z + p2Position.z) * 0.5F);
  } else if (p1Alive) {
    Enemy::lastKnownPlayerPos = p1Position;
  } else if (p2Alive) {
    Enemy::lastKnownPlayerPos = p2Position;
  }
  bool anyAlive = p1Alive || p2Alive;
  for (auto* enemy : enemies) {
    if (!anyAlive) {
      // Freeze: skip update while both players are dead
      continue;
    }
    Vec4 target;
    if (p1Alive && p2Alive) {
      auto* epos = enemy->mesh->getPosition();
      float d1 = (epos->x - p1Position.x) * (epos->x - p1Position.x) +
                 (epos->z - p1Position.z) * (epos->z - p1Position.z);
      float d2 = (epos->x - p2Position.x) * (epos->x - p2Position.x) +
                 (epos->z - p2Position.z) * (epos->z - p2Position.z);
      target = (d1 <= d2) ? p1Position : p2Position;
    } else if (p1Alive) {
      target = p1Position;
    } else {
      target = p2Position;
    }
    enemy->update(heightmap, target, shootAction);
  }
}

}  // namespace Demo
