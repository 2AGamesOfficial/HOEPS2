#pragma once
#include <tyra>
#include "states/game/renderer/renderer_dynamic_pair.hpp"
#include "states/game/renderer/renderer_static_pair.hpp"
#include "states/game/terrain/heightmap.hpp"
#include "states/game/player/player_shoot_action.hpp"
#include <functional>
#include "./enemy_info.hpp"

using Tyra::AnimationSequenceCallback;
using Tyra::Audio;
using Tyra::DynamicMesh;
using Tyra::DynPipOptions;
using Tyra::Engine;
using Tyra::Renderer;
using Tyra::Renderer3DUtility;
using Tyra::StaPipOptions;
using Tyra::StaticMesh;
using Tyra::TextureRepository;
using Tyra::Vec4;

namespace Demo {

class Enemy {
 public:
  Enemy(Engine* engine, const EnemyInfo& info);
  ~Enemy();

  // Ophanim mesh
  DynamicMesh* ophanimMesh;
  DynPipOptions* ophanimOptions;
  RendererDynamicPair* ophanimPair;

  // Demon meshes
  DynamicMesh* demonLegsMesh;
  DynPipOptions* demonLegsOptions;
  RendererDynamicPair* demonLegsPair;
  StaticMesh* demonUpperMesh;
  StaPipOptions* demonUpperOptions;
  RendererStaticPair* demonUpperPair;

  // Currently active mesh (alias to ophanimMesh or demonLegsMesh depending on type)
  DynamicMesh* mesh;
  DynPipOptions* options;
  RendererDynamicPair* pair;

  EnemyType currentType;

  StaticMesh* pentagramMesh;
  StaPipOptions* pentagramOptions;
  RendererStaticPair* pentagramPair;

  StaticMesh* bloodMesh;
  StaPipOptions* bloodOptions;
  RendererStaticPair* bloodPair;
  int bloodTimer;
  bool pendingExplosion;
  Vec4 pendingExplosionPos;
  bool recentlyKilled;  // set true on any death, GameState consumes and resets
  Tyra::Vec4 deathPos;  // position where enemy died, for drops

  int spawnTimer;
  int preSpawnTimer;
  Vec4 pendingSpawnPos;
  int hp;
  int currentMaxHp;
  float currentSpeed;
  int currentDamageAmount;
  static const float MIN_SPAWN_DIST_FROM_PLAYER;
  static Vec4 lastKnownPlayerPos;
  static float currentOphanimChance;
  bool takeDamage(int amount);

  void update(const Heightmap& heightmap, const Vec4& playerPosition,
              const PlayerShootAction& shootAction);

  EnemyInfo info;

 private:
  std::vector<u32> ophanimSequence;
  std::vector<u32> demonSequence;
  void walk(const Heightmap& heightmap, const Vec4& positionDiff);
  void animationCallback(const AnimationSequenceCallback& callback);
  void fight();
  void setMeshToSpawn();
  void activateType(EnemyType t);
  bool isWalking = true;
  bool isFighting = false;
  Vec4 terrainLeftUp;
  Vec4 terrainRightDown;
  Audio* audio;
  std::function<void(const AnimationSequenceCallback&)> _animCb;
};

}  // namespace Demo
