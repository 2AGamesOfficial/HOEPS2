#pragma once
namespace Demo {
enum WeaponType {
  WEAPON_REVOLVER = 0,
  WEAPON_SMG = 1,
  WEAPON_SHOTGUN = 2,
  WEAPON_GRENADE_LAUNCHER = 3,
  WEAPON_GUNCON = 4,
  WEAPON_COUNT = 5
};
struct WeaponData {
  const char* name;
  const char* torsoObjPath;
  const char* iconPath;
  const char* bulletIconPath;
  const char* shootAudioPath;
  const char* reloadAudioPath;
  const char* emptyAudioPath;
  int magSize;
  int damage;
  int fireCooldown;
  int reloadFrames;
  int numPellets;
  float spreadRadians;
  bool isProjectile;
  float aoeRadius;
  bool isAutomatic;
  const char* muzzleFlashObjPath;
  bool isPiercing;             // true = ray pierces through all enemies in its line
};
// Stats table
static const WeaponData WEAPON_DATA[WEAPON_COUNT] = {
  // REVOLVER
  { "Revolver",
    "game/models/player/PlayerHandgun.obj",
    "game/hud/RevolverIcon.png",
    "game/hud/BulletRevolver.png",
    "game/audio/revolver_fire.adpcm",
    "game/audio/revolver_reload.adpcm",
    "game/audio/revolver_empty.adpcm",
    6, 100, 12, 120, 2, 0.15F, false, 0.0F, false,
    "game/models/muzzleflash/muzzleflash_Revolver.obj",
    false },
  // SMG
  { "SMG",
    "game/models/player/PlayerSMG.obj",
    "game/hud/SMGIcon.png",
    "game/hud/BulletSMG.png",
    "game/audio/smg_fire.adpcm",
    "game/audio/smg_reload.adpcm",
    "game/audio/smg_empty.adpcm",
    20, 50, 6, 120, 1, 0.0F, false, 0.0F, true,
    "game/models/muzzleflash/muzzleflash_SMG.obj",
    false },
  // SHOTGUN
  { "Shotgun",
    "game/models/player/PlayerShotgun.obj",
    "game/hud/ShotgunIcon.png",
    "game/hud/ShellShotgun.png",
    "game/audio/shotgun_fire.adpcm",
    "game/audio/shotgun_reload.adpcm",
    "game/audio/shotgun_empty.adpcm",
    2, 40, 20, 75, 5, 0.5F, false, 0.0F, false,
    "game/models/muzzleflash/muzzleflash_Shotgun.obj",
    false },
  // GRENADE LAUNCHER
  { "Grenade Launcher",
    "game/models/player/PlayerGL.obj",
    "game/hud/GrenadeLauncherIcon.png",
    "game/hud/BulletGrenadeLauncher.png",
    "game/audio/gl_fire.adpcm",
    "game/audio/grenade_launcher_reload.adpcm",
    "game/audio/grenade_launcher_empty.adpcm",
    4, 150, 30, 120, 1, 0.0F, true, 112.5F, false,
    nullptr,
    false },
  // GUNCON (PS2 Light Gun) - piercing laser, arcade feel
  { "PS2 Light Gun",
    "game/models/player/Player_gcon.obj",
    "game/hud/gconIcon.png",
    "game/hud/BulletGcon.png",
    "game/audio/gcon_fire.adpcm",
    "game/audio/gcon_reload.adpcm",
    "game/audio/gcon_empty.adpcm",
    6, 250, 20, 120, 1, 0.0F, false, 0.0F, false,
    nullptr,
    true },
};
}  // namespace Demo
