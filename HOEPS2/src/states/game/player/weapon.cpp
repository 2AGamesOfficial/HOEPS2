#include "states/game/player/weapon.hpp"
#include <string>

using Tyra::FileUtils;
using Tyra::ObjLoader;
using Tyra::ObjLoaderOptions;

namespace Demo {

Weapon::Weapon(Engine* engine, Tyra::Pad* t_pad, int t_playerIndex) {
  ObjLoader loader;
  ObjLoaderOptions objOptions;
  objOptions.flipUVs = true;
  objOptions.scale = .5F;

  pad = t_pad;
  audio = &engine->audio;
  auto* repo = &engine->renderer.core.texture.repository;

  // Invisible placeholder mesh (FPS-era holdover, not rendered in top-down)
  auto data =
      loader.load(FileUtils::fromCwd("game/models/tracer/tracer.obj"), objOptions);
  data->loadNormals = false;
  mesh = new StaticMesh(data.get());
  mesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
  repo->addByMesh(mesh, FileUtils::fromCwd("game/models/tracer/"), "png");

  allocateOptions();

  for (int i = 0; i < WEAPON_COUNT; i++) {
    shootAdpcms[i] = audio->adpcm.load(
        FileUtils::fromCwd(WEAPON_DATA[i].shootAudioPath));
    // Reuse same handle if reload path matches fire path (e.g. guncon)
    if (std::string(WEAPON_DATA[i].reloadAudioPath) == std::string(WEAPON_DATA[i].shootAudioPath)) {
      reloadAdpcms[i] = shootAdpcms[i];
    } else {
      reloadAdpcms[i] = audio->adpcm.load(
          FileUtils::fromCwd(WEAPON_DATA[i].reloadAudioPath));
    }
    // Same dedup for empty path
    if (std::string(WEAPON_DATA[i].emptyAudioPath) == std::string(WEAPON_DATA[i].shootAudioPath)) {
      emptyAdpcms[i] = shootAdpcms[i];
    } else if (std::string(WEAPON_DATA[i].emptyAudioPath) == std::string(WEAPON_DATA[i].reloadAudioPath)) {
      emptyAdpcms[i] = reloadAdpcms[i];
    } else {
      emptyAdpcms[i] = audio->adpcm.load(
          FileUtils::fromCwd(WEAPON_DATA[i].emptyAudioPath));
    }
  }
  dryFireLatch = false;

  adpcmChannelsCount = 4;  // 4 channels per player
  adpcmChannelBase = (t_playerIndex == 0) ? 0 : 4;
  adpcmCurrentChannel = 0;

  const u8 shootVolume = 90;
  for (u8 i = 0; i < adpcmChannelsCount; i++) {
    audio->adpcm.setVolume(shootVolume, adpcmChannelBase + i);
  }

  // Init all weapons to full mag
  for (int i = 0; i < WEAPON_COUNT; i++) {
    ammoPerWeapon[i] = WEAPON_DATA[i].magSize;
  }

  currentWeapon = WEAPON_REVOLVER;
  ammo = ammoPerWeapon[currentWeapon];
  maxAmmo = WEAPON_DATA[currentWeapon].magSize;
  reloadTimer = 0;
  fireCooldownTimer = 0;
}

Weapon::~Weapon() {
  delete mesh;
  delete options;
  // shootAdpcms + reloadAdpcm are engine-owned; do not delete
}

void Weapon::setWeapon(WeaponType t) {
  currentWeapon = t;
  ammo = ammoPerWeapon[t];
  maxAmmo = WEAPON_DATA[t].magSize;
  reloadTimer = 0;
  fireCooldownTimer = 0;
}

u8 Weapon::getShootChannel() {
  auto result = adpcmChannelBase + adpcmCurrentChannel;
  adpcmCurrentChannel++;
  if (adpcmCurrentChannel >= adpcmChannelsCount) {
    adpcmCurrentChannel = 0;
  }
  return result;
}

void Weapon::update() {
  isShooting = false;

  // Sync public fields with current weapon
  ammo = ammoPerWeapon[currentWeapon];
  maxAmmo = WEAPON_DATA[currentWeapon].magSize;

  if (reloadTimer > 0) {
    reloadTimer--;
    if (reloadTimer == 0) {
      // Reload animation completed — refill
      ammoPerWeapon[currentWeapon] = WEAPON_DATA[currentWeapon].magSize;
      ammo = ammoPerWeapon[currentWeapon];
    }
  }
  if (fireCooldownTimer > 0) fireCooldownTimer--;

  // Reload on L1
  if (pad->getClicked().L1 && ammo < maxAmmo && reloadTimer == 0) {
    reload();
    reloadTimer = WEAPON_DATA[currentWeapon].reloadFrames;
  }

  // Shoot — SMG is auto (fireCooldown == 0), others are semi-auto (tap)
  const auto& data = WEAPON_DATA[currentWeapon];
  bool fireInput = data.isAutomatic
                       ? pad->getPressed().R1
                       : pad->getClicked().R1;

  if (fireInput && ammo > 0 && reloadTimer == 0 && fireCooldownTimer == 0) {
    shoot();
    dryFireLatch = false;
  } else if (fireInput && ammo == 0 && reloadTimer == 0 && !dryFireLatch) {
    // Dry fire: empty click, once per press (auto weapons wont spam)
    audio->adpcm.tryPlay(emptyAdpcms[currentWeapon], getShootChannel());
    dryFireLatch = true;
  } else if (!pad->getPressed().R1) {
    dryFireLatch = false;  // release = allow next click
  }
}

void Weapon::shoot() {
  isShooting = true;
  shootTimer.prime();
  audio->adpcm.tryPlay(shootAdpcms[currentWeapon], getShootChannel());
  ammoPerWeapon[currentWeapon]--;
  ammo = ammoPerWeapon[currentWeapon];
  fireCooldownTimer = WEAPON_DATA[currentWeapon].fireCooldown;
}

void Weapon::reload() {
  // Start reload: just play sound. Ammo refills when reloadTimer completes.
  audio->adpcm.tryPlay(reloadAdpcms[currentWeapon], getShootChannel());
}

void Weapon::allocateOptions() {
  options = new StaPipOptions();
  options->textureMappingType = Tyra::TyraNearest;
  options->shadingType = Tyra::TyraShadingFlat;
  options->blendingEnabled = false;
  options->fullClipChecks = false;
  options->frustumCulling = Tyra::PipelineFrustumCulling_None;
  options->transformationType = Tyra::TyraMP;
  options->antiAliasingEnabled = false;
}

}  // namespace Demo
