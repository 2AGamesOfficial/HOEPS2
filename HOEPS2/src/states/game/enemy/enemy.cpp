#include "states/game/enemy/enemy.hpp"
#include "states/game/colliders.hpp"
#include <functional>

using Tyra::Color;
using Tyra::FileUtils;
using Tyra::Math;
using Tyra::ObjLoader;
using Tyra::ObjLoaderOptions;

namespace Demo {

const float Enemy::MIN_SPAWN_DIST_FROM_PLAYER = 250.0F;
Vec4 Enemy::lastKnownPlayerPos = Vec4(0.0F, 50.0F, 0.0F);
float Enemy::currentOphanimChance = 0.0F;  // set by EnemyManager::setDifficulty

static bool enemyIsInBox(float x, float z, const ColliderBox& box) {
  return x >= box.minX && x <= box.maxX && z >= box.minZ && z <= box.maxZ;
}

static bool enemyIsValidPos(float x, float z) {
  for (int i = 0; i < NUM_OBSTACLES; i++) {
    if (enemyIsInBox(x, z, OBSTACLES[i])) return false;
  }
  return true;
}

Enemy::Enemy(Engine* engine, const EnemyInfo& t_info) {
  info = t_info;

  // Build ophanim mesh
  ophanimMesh = new DynamicMesh(*info.motherMesh);
  {
    auto* bodyMat = ophanimMesh->getMaterialByName("Material.006");
    auto* clothMat = ophanimMesh->getMaterialByName("Material.007");
    TYRA_ASSERT(bodyMat != nullptr && clothMat != nullptr, "Ophanim materials not found!");
    info.bodyTexture->addLink(bodyMat->id);
    info.clothTexture->addLink(clothMat->id);
  }
  ophanimOptions = new DynPipOptions();
  ophanimOptions->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
  ophanimPair = new RendererDynamicPair{ophanimMesh, ophanimOptions};
  ophanimSequence = {1, 2, 3, 4, 5, 6};

  // Build demon legs mesh
  demonLegsMesh = new DynamicMesh(*info.legsMotherMesh);
  {
    auto* legsMat = demonLegsMesh->getMaterialByName("Material.003");
    TYRA_ASSERT(legsMat != nullptr, "Demon legs material not found!");
    info.legsTex->addLink(legsMat->id);
  }
  demonLegsOptions = new DynPipOptions();
  demonLegsOptions->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
  demonLegsPair = new RendererDynamicPair{demonLegsMesh, demonLegsOptions};
  demonSequence = {0,1,2,3,4,5,6,7,8,9,10,11,12};

  // Build demon upper body
  demonUpperMesh = new StaticMesh(*info.upperMotherMesh);
  {
    auto* torsoMat = demonUpperMesh->getMaterialByName("Material.001");
    auto* hornMat = demonUpperMesh->getMaterialByName("Material.002");
    TYRA_ASSERT(torsoMat != nullptr && hornMat != nullptr, "Demon upper materials not found!");
    info.upperTex1->addLink(torsoMat->id);
    info.upperTex2->addLink(hornMat->id);
  }
  demonUpperOptions = new StaPipOptions();
  demonUpperOptions->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
  demonUpperPair = new RendererStaticPair{demonUpperMesh, demonUpperOptions};

  terrainLeftUp = info.terrainLeftUp;
  terrainRightDown = info.terrainRightDown;

  // Pentagram — copy from shared master mesh (loaded once by EnemyManager)
  pentagramMesh = new StaticMesh(*info.pentagramMotherMesh);
  if (info.pentagramTex != nullptr) {
    for (auto* mat : pentagramMesh->materials) {
      info.pentagramTex->addLink(mat->id);
    }
  }
  pentagramOptions = new StaPipOptions();
  pentagramOptions->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
  pentagramOptions->blendingEnabled = true;
  pentagramPair = new RendererStaticPair{pentagramMesh, pentagramOptions};

  // Blood splash (copy from shared master)
  bloodMesh = new StaticMesh(*info.bloodMotherMesh);
  if (info.bloodTex != nullptr) {
    for (auto* mat : bloodMesh->materials) {
      info.bloodTex->addLink(mat->id);
    }
  }
  bloodMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
  bloodOptions = new StaPipOptions();
  bloodOptions->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
  bloodOptions->blendingEnabled = true;
  bloodPair = new RendererStaticPair{bloodMesh, bloodOptions};
  bloodTimer = 0;
  pendingExplosion = false;
  recentlyKilled = false;

  audio = &engine->audio;
  audio->adpcm.setVolume(30, info.adpcmChannel);
  _animCb = std::bind(&Enemy::animationCallback, this, std::placeholders::_1);

  // Hide both meshes initially
  ophanimMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
  demonLegsMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
  demonUpperMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));

  preSpawnTimer = 0;
  // Pick initial type (ophanim chance governed by current difficulty)
  currentType = (Math::randomf(0.0F, 1.0F) < currentOphanimChance) ? ENEMY_OPHANIM : ENEMY_DEMON;
  activateType(currentType);
  setMeshToSpawn();
}

Enemy::~Enemy() {
  delete ophanimMesh;
  delete ophanimOptions;
  delete ophanimPair;
  delete demonLegsMesh;
  delete demonLegsOptions;
  delete demonLegsPair;
  delete demonUpperMesh;
  delete demonUpperOptions;
  delete demonUpperPair;
  delete pentagramMesh;
  delete pentagramOptions;
  delete pentagramPair;
  delete bloodMesh;
  delete bloodOptions;
  delete bloodPair;
}

void Enemy::activateType(EnemyType t) {
  currentType = t;
  if (t == ENEMY_OPHANIM) {
    mesh = ophanimMesh;
    options = ophanimOptions;
    pair = ophanimPair;
    currentMaxHp = 250;
    currentSpeed = 3.75F;
    currentDamageAmount = 5;
    // Hide demon meshes
    demonLegsMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
    demonUpperMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
  } else {
    mesh = demonLegsMesh;
    options = demonLegsOptions;
    pair = demonLegsPair;
    currentMaxHp = 60;
    currentSpeed = 3.0F;
    currentDamageAmount = 34;
    // Hide ophanim mesh
    ophanimMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
  }

  mesh->animation.setSequence((t == ENEMY_OPHANIM) ? ophanimSequence : demonSequence);
  mesh->animation.loop = true;
  mesh->animation.speed = 0.15F;
  mesh->animation.setCallback(_animCb);
}

void Enemy::setMeshToSpawn() {
  // Randomize type on each spawn (ophanim rare early, more common later)
  EnemyType newType = (Math::randomf(0.0F, 1.0F) < currentOphanimChance) ? ENEMY_OPHANIM : ENEMY_DEMON;
  activateType(newType);

  const float maxRadius = 600.0F;
  float x = 0.0F, z = 0.0F;
  for (int attempt = 0; attempt < 20; attempt++) {
    float angle = Math::randomf(0.0F, 6.2831853F);
    float radius = Math::randomf(100.0F, maxRadius);
    x = cos(angle) * radius;
    z = sin(angle) * radius;
    if (!enemyIsValidPos(x, z)) continue;
    float dxp = x - lastKnownPlayerPos.x;
    float dzp = z - lastKnownPlayerPos.z;
    if (dxp*dxp + dzp*dzp < MIN_SPAWN_DIST_FROM_PLAYER * MIN_SPAWN_DIST_FROM_PLAYER) continue;
    break;
  }
  pendingSpawnPos = Vec4(x, 0.0F, z, 1.0F);
  pentagramMesh->setPosition(Vec4(pendingSpawnPos.x, 5.0F, pendingSpawnPos.z));
  mesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
  if (currentType == ENEMY_DEMON) {
    demonUpperMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
  }
  spawnTimer = 60;
  hp = currentMaxHp;
}

bool Enemy::takeDamage(int amount) {
  if (spawnTimer > 0 || preSpawnTimer > 0) return false;
  hp -= amount;
  if (hp <= 0) {
    recentlyKilled = true;
    deathPos = *mesh->getPosition();
    // Type-specific death effects
    if (currentType == ENEMY_DEMON && info.adpcmDemonDeath != nullptr) {
      audio->adpcm.playPooled(info.adpcmDemonDeath, 17, 2);
      auto* pos = mesh->getPosition();
      bloodMesh->setPosition(Vec4(pos->x, 2.0F, pos->z));
      bloodMesh->rotation.identity();
      bloodMesh->rotation.rotateY(Math::randomf(0.0F, 6.2831853F));
      bloodTimer = 90;
    } else if (currentType == ENEMY_OPHANIM) {
      auto* pos = mesh->getPosition();
      pendingExplosionPos = Vec4(pos->x, 30.0F, pos->z);
      pendingExplosion = true;
    }
    // Hide everything (no pentagram yet) and wait 1-3 seconds before spawning next
    ophanimMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
    demonLegsMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
    demonUpperMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
    pentagramMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
    preSpawnTimer = (int)Math::randomf(60.0F, 180.0F);
    return true;
  }
  return false;
}

void Enemy::update(const Heightmap& heightmap, const Vec4& playerPosition,
                   const PlayerShootAction& shootAction) {
  if (bloodTimer > 0) {
    bloodTimer--;
    if (bloodTimer == 0) {
      bloodMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
    }
  }
  if (preSpawnTimer > 0) {
    preSpawnTimer--;
    if (preSpawnTimer == 0) {
      setMeshToSpawn();
    }
    return;
  }
  if (spawnTimer > 0) {
    spawnTimer--;
    if (spawnTimer == 0) {
      mesh->setPosition(pendingSpawnPos);
      if (currentType == ENEMY_DEMON) {
        demonUpperMesh->setPosition(Vec4(pendingSpawnPos.x, pendingSpawnPos.y + 20.0F, pendingSpawnPos.z));
      }
      pentagramMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
    }
    return;
  }

  auto* enemyPosition = mesh->getPosition();
  lastKnownPlayerPos = playerPosition;
  (void)shootAction;
  auto diff = *enemyPosition - playerPosition;
  auto ang = Math::atan2(diff.x, diff.z);
  if (diff.length() > 80.0F) {
    walk(heightmap, diff);
  } else {
    fight();
  }
  float naturalRotation = (currentType == ENEMY_DEMON) ? 3.1F - 1.5708F : 3.1F;
  mesh->rotation.identity();
  ang += naturalRotation;
  mesh->rotation.rotateByAngle(ang, Vec4(0.0F, 1.0F, 0.0F, 0.0F));
  mesh->update();

  if (currentType == ENEMY_DEMON) {
    auto* pos = mesh->getPosition();
    demonUpperMesh->setPosition(Vec4(pos->x, pos->y + 20.0F, pos->z));
    demonUpperMesh->rotation.identity();
    demonUpperMesh->rotation.rotateByAngle(ang, Vec4(0.0F, 1.0F, 0.0F, 0.0F));
  }
}

void Enemy::walk(const Heightmap& heightmap, const Vec4& positionDiff) {
  if (isFighting) {
    mesh->animation.setSequence((currentType == ENEMY_OPHANIM) ? ophanimSequence : demonSequence);
  }
  isFighting = false;
  isWalking = true;
  auto* enemyPosition = mesh->getPosition();
  auto normalized = positionDiff;
  normalized.normalize();

  Vec4 nextPos = *enemyPosition;
  float tryX = nextPos.x - normalized.x * currentSpeed;
  if (enemyIsValidPos(tryX, nextPos.z)) nextPos.x = tryX;
  float tryZ = nextPos.z - normalized.z * currentSpeed;
  if (enemyIsValidPos(nextPos.x, tryZ)) nextPos.z = tryZ;

  nextPos.y = heightmap.getHeightOffset(nextPos) + (currentType == ENEMY_DEMON ? 40.0F : -20.0F);
  mesh->translation.identity();
  mesh->translation.translate(nextPos);
}

void Enemy::fight() {
  if (isWalking) {
    mesh->animation.setSequence((currentType == ENEMY_OPHANIM) ? ophanimSequence : demonSequence);
  }
  isFighting = true;
  isWalking = false;
}

void Enemy::animationCallback(const AnimationSequenceCallback& callback) {
  if (callback == AnimationSequenceCallback::AnimationSequenceCallback_Loop) {
    if (isFighting && currentType == ENEMY_DEMON) {
      audio->adpcm.tryPlay(info.adpcmPunch, info.adpcmChannel);
    }
  }
}

}  // namespace Demo
