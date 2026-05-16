#pragma once
#include "states/game/renderer/renderer_dynamic_pair.hpp"
#include "states/game/renderer/renderer_static_pair.hpp"
#include "states/game/terrain/heightmap.hpp"
#include "./weapon.hpp"
#include "./weapon_type.hpp"
#include "./camera.hpp"
#include "./player_shoot_action.hpp"
#include <tyra>

using Tyra::Engine;
using Tyra::Pad;
using Tyra::Renderer3DUtility;
using Tyra::DynamicMesh;
using Tyra::DynPipOptions;
using Tyra::StaticMesh;
using Tyra::StaPipOptions;
using Tyra::Vec4;

namespace Demo {

class Player {
 public:
  Player(Engine* engine, Pad* pad, int playerIndex);
  ~Player();

  int playerIndex;  // 0 or 1
  bool isJoined;    // true if active in game

  const Vec4& getPosition() const { return position; }
  const Vec4& getAimDirection() const { return aimDirection; }
  int getAmmo() const { return weapon.ammo; }
  bool isReloading() const { return weapon.reloadTimer > 0; }
  int getHealth() const { return health; }
  int getMaxHealth() const { return maxHealth; }
  void takeDamage(int amount);
  int spawnInvulnTimer;  // frames of post-spawn invuln
  void overridePosition(const Vec4& p) { position = p; }
  static bool isPositionValid(const Vec4& p);
  void resetHealth() { health = maxHealth; weapon.resetAmmo(); }
  void heal() { health = maxHealth; }
  void resetPosition() {
    Vec4 p = (playerIndex == 0) ? Vec4(-20.0F, 50.0F, -60.0F) : Vec4(20.0F, 50.0F, -60.0F);
    position = Player::isPositionValid(p) ? p : Vec4(0.0F, 50.0F, 0.0F);
    spawnInvulnTimer = 120;
  }
  void resetPositionTo(const Vec4& p) { position = p; spawnInvulnTimer = 120; }
  void resetWeapons() {
    for (int i = 0; i < WEAPON_COUNT; i++) ownedWeapons[i] = false;
    ownedWeapons[WEAPON_REVOLVER] = true;
    currentWeapon = WEAPON_REVOLVER;
    weapon.resetAmmo();
    weapon.setWeapon(WEAPON_REVOLVER);
  }
  WeaponType getCurrentWeapon() const { return currentWeapon; }
  bool hasWeapon(WeaponType t) const { return ownedWeapons[t]; }
  void pickupWeapon(WeaponType t);
  RendererStaticPair* getCurrentModelPair() const {
    return modelPairs[currentWeapon];
  }

  RendererStaticPair* pair;
  RendererDynamicPair* legsPair;

  void update(const Heightmap& heightmap);
  PlayerShootAction getShootAction() const;

 private:
  void handlePlayerPosition(const Heightmap& heightmap,
                            const float& terrainHeight);
  void handleAiming();
  void handleWeaponSwitch();

  float speed;
  int health;
  int maxHealth;
  float aimAngle;
  Pad* pad;
  Vec4 position;
  Vec4 aimDirection;
  Weapon weapon;

  WeaponType currentWeapon;
  bool ownedWeapons[WEAPON_COUNT];

  StaticMesh* playerMeshes[WEAPON_COUNT];
  StaPipOptions* playerOptions[WEAPON_COUNT];
  RendererStaticPair* modelPairs[WEAPON_COUNT];
  DynamicMesh* legsMesh;
  DynPipOptions* legsOptions;
  bool isMoving;
};

}  // namespace Demo
