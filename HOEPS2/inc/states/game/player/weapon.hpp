#pragma once
#include <tyra>
#include "./weapon_type.hpp"

using Tyra::Audio;
using Tyra::Engine;
using Tyra::Pad;
using Tyra::Renderer;
using Tyra::StaPipOptions;
using Tyra::StaticMesh;
using Tyra::TextureRepository;
using Tyra::Timer;
using Tyra::Vec4;

namespace Demo {

class Weapon {
 public:
  Weapon(Engine* engine, Tyra::Pad* pad, int playerIndex = 0);
  ~Weapon();

  StaticMesh* mesh;  // invisible placeholder (FPS holdover)
  StaPipOptions* options;
  bool isShooting = false;

  void update();
  int reloadTimer;

  // Weapon switching
  void setWeapon(WeaponType t);
  WeaponType getCurrentWeapon() const { return currentWeapon; }

  // Ammo API (per-weapon)
  int getAmmo() const { return ammoPerWeapon[currentWeapon]; }
  int getMaxAmmo() const { return WEAPON_DATA[currentWeapon].magSize; }
  void resetAmmo() {
    for (int i = 0; i < WEAPON_COUNT; i++) {
      ammoPerWeapon[i] = WEAPON_DATA[i].magSize;
    }
  }

  // Legacy accessors (keep field names for now to avoid breaking other code)
  int ammo;     // mirrors ammoPerWeapon[currentWeapon], synced in update()
  int maxAmmo;  // mirrors WEAPON_DATA[currentWeapon].magSize

 private:
  WeaponType currentWeapon;
  int ammoPerWeapon[WEAPON_COUNT];
  int fireCooldownTimer;

  Pad* pad;
  Audio* audio;
  Timer shootTimer;
  u8 adpcmCurrentChannel;
  u8 adpcmChannelsCount;
  u8 adpcmChannelBase;  // 0 for P1 (0-3), 4 for P2 (4-7)
  audsrv_adpcm_t* shootAdpcms[WEAPON_COUNT];
  audsrv_adpcm_t* reloadAdpcms[WEAPON_COUNT];
  audsrv_adpcm_t* emptyAdpcms[WEAPON_COUNT];
  bool dryFireLatch;  // prevents empty click spam for auto weapons

  void shoot();
  void reload();
  void allocateOptions();
  u8 getShootChannel();
};

}  // namespace Demo
