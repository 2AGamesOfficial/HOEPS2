#include "states/game/hud/hud.hpp"

using Tyra::FileUtils;

namespace Demo {

Hud::Hud(TextureRepository* t_repo, Hud::Anchor t_anchor) {
  anchor = t_anchor;
  repo = t_repo;
  maxAmmoCount = WEAPON_DATA[WEAPON_REVOLVER].magSize;
  currentWeapon = WEAPON_REVOLVER;

  // Health bar + cross. X flips for RIGHT anchor.
  float healthCrossX = (anchor == LEFT) ? 10.0F : (512.0F - 80.0F - 10.0F);
  float healthBarX = healthCrossX + (80.0F - 30.0F) / 2.0F;
  healthBarSprite = std::make_unique<Sprite>();
  healthBarSprite->mode = Tyra::MODE_STRETCH;
  healthBarSprite->size.set(30.0F, 60.0F);
  healthBarSprite->position.set(healthBarX, 426.0F - 50.0F);
  healthBarTexture = repo->add(FileUtils::fromCwd("game/hud/HealthBar.png"));
  healthBarTexture->addLink(healthBarSprite->id);

  healthSprite = std::make_unique<Sprite>();
  healthSprite->mode = Tyra::MODE_STRETCH;
  healthSprite->size.set(80.0F, 80.0F);
  healthSprite->position.set(healthCrossX, 448.0F - 80.0F - 10.0F);
  healthTexture = repo->add(FileUtils::fromCwd("game/hud/HealthOverlay.png"));
  healthTexture->addLink(healthSprite->id);

  // Gun background box — P1 top-left, P2 top-right
  float gunX = (anchor == LEFT) ? 10.0F : (512.0F - 100.0F - 10.0F);
  gunBgSprite = std::make_unique<Sprite>();
  gunBgSprite->mode = Tyra::MODE_STRETCH;
  gunBgSprite->size.set(100.0F, 100.0F);
  gunBgSprite->position.set(gunX, 10.0F);
  gunBgTexture = repo->add(FileUtils::fromCwd("game/hud/GunBG.png"));
  gunBgTexture->addLink(gunBgSprite->id);

  // Preload all 4 weapon icon textures
  for (int i = 0; i < WEAPON_COUNT; i++) {
    weaponIconTextures[i] = repo->add(FileUtils::fromCwd(WEAPON_DATA[i].iconPath));
  }

  gunIconSprite = std::make_unique<Sprite>();
  gunIconSprite->mode = Tyra::MODE_STRETCH;
  gunIconSprite->size.set(100.0F, 100.0F);
  gunIconSprite->position.set(gunX, 10.0F);
  weaponIconTextures[currentWeapon]->addLink(gunIconSprite->id);

  // Preload all 4 bullet textures
  for (int i = 0; i < WEAPON_COUNT; i++) {
    weaponBulletTextures[i] = repo->add(FileUtils::fromCwd(WEAPON_DATA[i].bulletIconPath));
  }

  // Preallocate MAX_BULLETS sprites — P1 grows rightward from right of gun; P2 grows leftward from left of gun
  const float bulletW = 9.0F;
  const float bulletH = 18.0F;
  const float bulletY = 10.0F + (100.0F - bulletH) / 2.0F;  // vertically center with gun box
  // For P1: first bullet at gunX + 110. For P2: last bullet ends at gunX - 10.
  const float bulletRightEdge = (anchor == LEFT)
      ? (gunX + 110.0F + MAX_BULLETS * (bulletW + 1.0F))  // will re-center via setAmmo
      : (gunX - 10.0F);
  for (int i = 0; i < MAX_BULLETS; i++) {
    auto sprite = std::make_unique<Sprite>();
    sprite->mode = Tyra::MODE_STRETCH;
    sprite->size.set(bulletW, bulletH);
    sprite->position.set(bulletRightEdge - (maxAmmoCount - i) * (bulletW + 1.0F), bulletY);
    weaponBulletTextures[currentWeapon]->addLink(sprite->id);
    bulletSprites.push_back(std::move(sprite));
  }

  // Crosshair (kept for compatibility but not rendered in game_state)
  crosshairSprite = std::make_unique<Sprite>();
  crosshairSprite->mode = Tyra::MODE_STRETCH;
  crosshairSprite->size.set(16.0F, 16.0F);
  crosshairSprite->position.set(512.0F / 2.0F - 16.0F / 2,
                                448.0F / 2.0F - 16.0F / 2);
  crosshairTexture = repo->add(FileUtils::fromCwd("game/crosshair.png"));
  crosshairTexture->addLink(crosshairSprite->id);
}

Hud::~Hud() {
  // Textures are owned by the repository and shared across HUD instances.
  // Engine shutdown cleans them up.
}

void Hud::setWeapon(WeaponType t) {
  if (t == currentWeapon) return;

  // Swap gun icon: unlink old, link new
  weaponIconTextures[currentWeapon]->removeLinkById(gunIconSprite->id);
  weaponIconTextures[t]->addLink(gunIconSprite->id);

  // Swap bullet texture on all sprites
  for (auto& sprite : bulletSprites) {
    weaponBulletTextures[currentWeapon]->removeLinkById(sprite->id);
    weaponBulletTextures[t]->addLink(sprite->id);
  }

  currentWeapon = t;
  maxAmmoCount = WEAPON_DATA[t].magSize;
}

void Hud::setAmmo(int count) {
  const float bulletW = 9.0F;
  const float bulletH = 18.0F;
  const float bulletY = 10.0F + (100.0F - bulletH) / 2.0F;
  // Recompute the edge from which bullets stack
  float gunX = (anchor == LEFT) ? 10.0F : (512.0F - 100.0F - 10.0F);
  // Visibility flips based on anchor so bullets always shrink TOWARD the gun.
  // LEFT: gun on left, low i = near gun. Show i < count → far bullets disappear first.
  // RIGHT: gun on right, high i = near gun. Show i >= (maxAmmoCount - count) → far bullets disappear first.
  // SMG splits into 2 rows of 10; other weapons single row
  const bool twoRows = (currentWeapon == WEAPON_SMG);
  const int rowSize = 10;
  const float rowGap = bulletH + 2.0F;
  // Per-anchor stacking edges:
  //   LEFT: bullets grow rightward from gunX+110
  //   RIGHT: bullets grow leftward from gunX-10
  const float leftStart = gunX + 110.0F;           // leftmost bullet X (LEFT anchor)
  const float rightEnd = gunX - 10.0F;             // rightmost bullet X edge (RIGHT anchor)
  for (int i = 0; i < (int)bulletSprites.size(); i++) {
    bool visible;
    if (i >= maxAmmoCount) {
      visible = false;
    } else if (anchor == LEFT) {
      visible = (i < count);
    } else {
      visible = (i >= (maxAmmoCount - count));
    }
    if (visible) {
      bulletSprites[i]->size.set(bulletW, bulletH);
      int col = twoRows ? (i % rowSize) : i;
      int row = twoRows ? (i / rowSize) : 0;
      int rowMax = twoRows ? rowSize : maxAmmoCount;
      float x;
      if (anchor == LEFT) {
        x = leftStart + col * (bulletW + 1.0F);
      } else {
        x = rightEnd - (rowMax - col) * (bulletW + 1.0F);
      }
      float y = bulletY + row * rowGap;
      bulletSprites[i]->position.set(x, y);
    } else {
      bulletSprites[i]->position.set(-100.0F, -100.0F);
    }
  }
}

void Hud::setHealth(int hp, int maxHp) {
  float ratio = (float)hp / (float)maxHp;
  if (ratio < 0.0F) ratio = 0.0F;
  float fullHeight = 50.0F;
  float newHeight = fullHeight * ratio;
  healthBarSprite->size.set(30.0F, newHeight);
  healthBarSprite->position.set((anchor == LEFT ? 10.0F : (512.0F - 80.0F - 10.0F)) + (80.0F - 30.0F) / 2.0F, 426.0F - newHeight);
}

}  // namespace Demo
