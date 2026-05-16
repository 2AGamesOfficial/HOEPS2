/*
# _____        ____   ___
#   |     \/   ____| |___|
#   |     |   |   \  |   |
#-----------------------------------------------------------------------
# Copyright 2022, tyra - https://github.com/h4570/tyra
# Licensed under Apache License 2.0
# Sandro Sobczyński <sandro.sobczynski@gmail.com>
*/
#pragma once
#include <tyra>
#include <vector>
#include "states/game/terrain/heightmap.hpp"
#include "states/game/player/player_shoot_action.hpp"
#include "./enemy.hpp"
using Tyra::DynamicMesh;
using Tyra::Engine;
using Tyra::StaticMesh;
using Tyra::Texture;
using Tyra::TextureRepository;
using Tyra::Vec4;
namespace Demo {
class EnemyManager {
 public:
  EnemyManager(Engine* engine, const Heightmap& heightmap);
  ~EnemyManager();
  void update(const Heightmap& heightmap, const Vec4& playerPosition,
              const PlayerShootAction& shootAction);
  // 2P version: each enemy picks nearest living player
  void update(const Heightmap& heightmap,
              const Vec4& p1Position, bool p1Alive,
              const Vec4& p2Position, bool p2Alive,
              const PlayerShootAction& shootAction);
  float applyShot(const Tyra::Ray& ray, int damage);
  float applyShotPiercing(const Tyra::Ray& ray, int damage);
  void applyAoeDamage(const Tyra::Vec4& center, float radius, int damage);
  void setDifficulty(int kills);  // adjusts preSpawn ranges + unlocks slots
  int activeCount;  // number of currently unlocked enemies (2-4)
  int preSpawnMin;  // current minimum pre-spawn delay (frames)
  int preSpawnMax;  // current maximum
  float lastShotDistance;
  bool lastShotHitEnemy;
  const std::vector<RendererDynamicPair*>& getPairs();
  const std::vector<RendererStaticPair*>& getPentagramPairs();
  const std::vector<RendererStaticPair*>& getUpperBodyPairs();
  const std::vector<RendererStaticPair*>& getBloodPairs();
  const std::vector<Enemy*>& getEnemies() const { return enemies; }
 private:
  TextureRepository* textureRepo;
  std::vector<Enemy*> enemies;
  // Ophanim assets
  DynamicMesh* ophanimMotherMesh;
  Texture* ophanimBodyTexture;
  Texture* ophanimClothTexture;
  // Demon assets
  StaticMesh* demonUpperMotherMesh;
  Texture* demonUpperTex1;
  Texture* demonUpperTex2;
  DynamicMesh* demonLegsMotherMesh;
  Texture* demonLegsTex;
  // Shared pentagram (loaded once, copied per enemy)
  StaticMesh* pentagramMotherMesh;
  Texture* pentagramTex;
  // Shared blood splash
  StaticMesh* bloodMotherMesh;
  Texture* bloodTex;
  // Cached output vectors to avoid per-frame allocation
  std::vector<RendererDynamicPair*> _cachedDynPairs;
  std::vector<RendererStaticPair*> _cachedPentagramPairs;
  std::vector<RendererStaticPair*> _cachedUpperBodyPairs;
  std::vector<RendererStaticPair*> _cachedBloodPairs;
};
}  // namespace Demo
