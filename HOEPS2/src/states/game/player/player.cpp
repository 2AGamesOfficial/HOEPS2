#include "states/game/player/player.hpp"
#include "states/game/colliders.hpp"

using Tyra::FileUtils;
using Tyra::Math;
using Tyra::ObjLoader;
using Tyra::ObjLoaderOptions;
using Tyra::DynPipOptions;

namespace Demo {

Player::Player(Engine* engine, Pad* t_pad, int t_playerIndex)
    : position(0.0F), weapon(engine, t_pad, t_playerIndex) {
  playerIndex = t_playerIndex;
  isJoined = (t_playerIndex == 0);  // P1 joined by default, P2 must press Start
  pair = new RendererStaticPair{weapon.mesh, weapon.options};
  pad = t_pad;
  position = Vec4(0.0F, 50.0F, 0.0F);
  aimDirection = Vec4(0.0F, 0.0F, -1.0F);
  aimAngle = 0.0F;
  speed = 5.0F;
  maxHealth = 100;
  health = maxHealth;
  spawnInvulnTimer = 120;  // 2s invuln on game start

  // Start with revolver only
  currentWeapon = WEAPON_REVOLVER;
  for (int i = 0; i < WEAPON_COUNT; i++) ownedWeapons[i] = false;
  ownedWeapons[WEAPON_REVOLVER] = true;

  // Load all 4 torso meshes
  ObjLoader loader;
  ObjLoaderOptions objOptions;
  objOptions.scale = 250.0F;

  auto* repo = &engine->renderer.getTextureRepository();

  weapon.setWeapon(WEAPON_REVOLVER);

  // Load legs (animated DynamicMesh)
  {
    ObjLoaderOptions legsOpts;
    legsOpts.animation.count = 12;
    legsOpts.flipUVs = false;
    legsOpts.scale = 25.0F;
    auto legsData = loader.load(
        FileUtils::fromCwd("game/models/player/legs/PlayerLegs1.obj"), legsOpts);
    legsData->loadNormals = false;
    legsMesh = new DynamicMesh(legsData.get());
    repo->addByMesh(legsMesh, FileUtils::fromCwd("game/models/player/legs/"), "png");
    legsMesh->animation.setSequence({0,1,2,3,4,5,6,7,8,9,10,11});
    legsMesh->animation.loop = true;
    legsMesh->animation.speed = 0.20F;
    legsMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
    legsOptions = new DynPipOptions();
    legsOptions->frustumCulling = Tyra::PipelineFrustumCulling_None;
    legsPair = new RendererDynamicPair{legsMesh, legsOptions};
  }
  isMoving = false;

  // For P2, look up the alt body texture once
  Tyra::Texture* p2BodyTex = nullptr;
  if (playerIndex == 1) {
    for (auto* tex : *repo->getAll()) {
      if (tex->name == "Player2Albedo.png") p2BodyTex = tex;
    }
    // Ensure Player2Albedo is in the repo even if no mesh loaded it yet
    if (p2BodyTex == nullptr) {
      p2BodyTex = repo->add(FileUtils::fromCwd("game/models/player/Player2Albedo.png"));
    }
  }

  for (int i = 0; i < WEAPON_COUNT; i++) {
    auto data = loader.load(
        FileUtils::fromCwd(WEAPON_DATA[i].torsoObjPath), objOptions);
    data->loadNormals = false;
    playerMeshes[i] = new StaticMesh(data.get());
    repo->addByMesh(playerMeshes[i],
                    FileUtils::fromCwd("game/models/player/"), "png");

    // P2: swap body texture. addByMesh creates one Texture per material, so
    // there may be many Textures named "PlayerAlbedo.png" in the repo. Iterate
    // all of them, find the ones linked to this mesh's mat ids, remove+relink.
    if (playerIndex == 1 && p2BodyTex != nullptr) {
      for (auto* mat : playerMeshes[i]->materials) {
        for (auto* tex : *repo->getAll()) {
          if (tex->name == "PlayerAlbedo.png" && tex->isLinkedWith(mat->id)) {
            tex->removeLinkById(mat->id);
            p2BodyTex->addLink(mat->id);
            break;
          }
        }
      }
    }

    playerOptions[i] = new StaPipOptions();
    playerOptions[i]->frustumCulling = Tyra::PipelineFrustumCulling_None;

    modelPairs[i] = new RendererStaticPair{playerMeshes[i], playerOptions[i]};
  }
}

Player::~Player() {
  delete pair;
  delete legsPair;
  delete legsMesh;
  delete legsOptions;
  for (int i = 0; i < WEAPON_COUNT; i++) {
    delete modelPairs[i];
    delete playerMeshes[i];
    delete playerOptions[i];
  }
}

void Player::update(const Heightmap& heightmap) {
  if (spawnInvulnTimer > 0) spawnInvulnTimer--;
  float terrainHeight = heightmap.getHeightOffset(getPosition());
  handlePlayerPosition(heightmap, terrainHeight);
  handleAiming();
  handleWeaponSwitch();
  weapon.update();

  // Legs animation
  {
    const auto& lj = pad->getLeftJoyPad();
    bool moving = (lj.v <= 100 || lj.v >= 200 || lj.h <= 100 || lj.h >= 200);
    if (moving) {
      legsMesh->animation.speed = 0.20F;
    } else {
      legsMesh->animation.speed = 0.0F;
    }
    legsMesh->setPosition(Vec4(position.x, position.y - 30.0F, position.z));
    legsMesh->rotation.identity();
    legsMesh->rotation.rotateY(aimAngle + 1.5708F);
    legsMesh->update();
  }

  // Position ONLY the current weapon's torso; hide the rest
  for (int i = 0; i < WEAPON_COUNT; i++) {
    if (i == currentWeapon) {
      playerMeshes[i]->setPosition(position);
      playerMeshes[i]->rotation.identity();
      playerMeshes[i]->rotation.rotateY(aimAngle);
    } else {
      playerMeshes[i]->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
    }
  }
}

void Player::handleWeaponSwitch() {
  // D-pad Left = previous owned weapon, D-pad Right = next owned weapon
  const auto& clicked = pad->getClicked();
  if (!clicked.DpadLeft && !clicked.DpadRight) return;
  // Weapon switch cancels current reload

  int dir = clicked.DpadRight ? 1 : -1;
  int next = currentWeapon;
  for (int i = 0; i < WEAPON_COUNT; i++) {
    next = (next + dir + WEAPON_COUNT) % WEAPON_COUNT;
    if (ownedWeapons[next]) {
      currentWeapon = (WeaponType)next;
      weapon.setWeapon(currentWeapon);
      return;
    }
  }
}

void Player::pickupWeapon(WeaponType t) {
  ownedWeapons[t] = true;
  currentWeapon = t;  // auto-switch to pickup
  weapon.setWeapon(t);
}

PlayerShootAction Player::getShootAction() const {
  PlayerShootAction action;
  action.isShooting = weapon.isShooting;
  if (action.isShooting) {
    Vec4 rayOrigin(position.x, 0.0F, position.z);
    action.ray = Ray(rayOrigin, aimDirection);
  }
  return action;
}

void Player::handleAiming() {
  const auto& rightJoy = pad->getRightJoyPad();
  const float deadzone = 40.0F;

  float rx = (float)rightJoy.h - 128.0F;
  float ry = (float)rightJoy.v - 128.0F;

  if (rx * rx + ry * ry > deadzone * deadzone) {
    aimAngle = atan2(rx, ry);
    aimDirection.x = sin(aimAngle);
    aimDirection.z = cos(aimAngle);
    aimDirection.normalize();
  }
}

static bool isInsideBox(const Vec4& pos, const ColliderBox& box) {
  return pos.x >= box.minX && pos.x <= box.maxX &&
         pos.z >= box.minZ && pos.z <= box.maxZ;
}

static bool isValidPosition(const Vec4& pos) {
  float dx = pos.x - OUTER_CENTER_X;
  float dz = pos.z - OUTER_CENTER_Z;
  if (dx * dx + dz * dz > OUTER_RADIUS * OUTER_RADIUS) return false;
  for (int i = 0; i < NUM_OBSTACLES; i++) {
    if (isInsideBox(pos, OBSTACLES[i])) return false;
  }
  return true;
}

void Player::handlePlayerPosition(const Heightmap& heightmap,
                                  const float& terrainHeight) {
  const auto& leftJoy = pad->getLeftJoyPad();

  float dx = 0.0F, dz = 0.0F;

  if (leftJoy.v <= 100) dz -= speed;
  else if (leftJoy.v >= 200) dz += speed;

  if (leftJoy.h <= 100) dx -= speed;
  else if (leftJoy.h >= 200) dx += speed;

  Vec4 tryX = position;
  tryX.x += dx;
  tryX.y = terrainHeight + 50.0F;
  if (isValidPosition(tryX)) position.x = tryX.x;

  Vec4 tryZ = position;
  tryZ.z += dz;
  tryZ.y = terrainHeight + 50.0F;
  if (isValidPosition(tryZ)) position.z = tryZ.z;

  position.y = terrainHeight + 50.0F;
}


bool Player::isPositionValid(const Vec4& p) { return isValidPosition(p); }

void Player::takeDamage(int amount) {
  if (spawnInvulnTimer > 0) return;  // invulnerable during grace period
  health -= amount;
  if (health < 0) health = 0;
}

}  // namespace Demo
