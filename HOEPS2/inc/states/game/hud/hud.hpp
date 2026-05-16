#pragma once
#include <tyra>
#include <memory>
#include <vector>
#include "states/game/player/weapon_type.hpp"

using std::unique_ptr;
using std::vector;
using Tyra::Sprite;
using Tyra::Texture;
using Tyra::TextureRepository;

namespace Demo {

class Hud {
 public:
  enum Anchor { LEFT = 0, RIGHT = 1 };
  Hud(TextureRepository* repo, Anchor anchor = LEFT);
  ~Hud();

  Anchor anchor;

  TextureRepository* repo;

  unique_ptr<Sprite> crosshairSprite;
  Texture* crosshairTexture;

  // Health
  unique_ptr<Sprite> healthBarSprite;
  unique_ptr<Sprite> healthSprite;
  Texture* healthBarTexture;
  Texture* healthTexture;
  void setHealth(int hp, int maxHp);

  // Current gun icon
  unique_ptr<Sprite> gunBgSprite;
  unique_ptr<Sprite> gunIconSprite;
  Texture* gunBgTexture;
  Texture* weaponIconTextures[WEAPON_COUNT];  // preloaded icons

  // Ammo bullets (preallocated to max of any weapon's mag)
  vector<unique_ptr<Sprite>> bulletSprites;
  Texture* weaponBulletTextures[WEAPON_COUNT];  // preloaded bullet icons

  void setAmmo(int count);
  void setWeapon(WeaponType t);

 private:
  static const int MAX_BULLETS = 20;  // SMG has biggest mag
  WeaponType currentWeapon;
  int maxAmmoCount;
};

}  // namespace Demo
