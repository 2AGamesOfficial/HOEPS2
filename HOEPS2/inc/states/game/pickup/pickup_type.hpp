#pragma once
namespace Demo {
enum PickupType {
  PICKUP_WEAPON_SMG = 0,
  PICKUP_WEAPON_SHOTGUN = 1,
  PICKUP_WEAPON_GRENADE_LAUNCHER = 2,
  PICKUP_WEAPON_GUNCON = 3,
  PICKUP_HEALTH = 4,
  PICKUP_TYPE_COUNT = 5
};
struct PickupData {
  const char* objPath;
  const char* texDir;
  float scale;
  float spinSpeed;
  float bobHeight;
  int effectValue;
};
static const PickupData PICKUP_DATA[PICKUP_TYPE_COUNT] = {
  { "game/models/player/SMG.obj",
    "game/models/player/", 200.0F, 0.05F, 10.0F, 0 },
  { "game/models/player/Shotgun.obj",
    "game/models/player/", 200.0F, 0.05F, 10.0F, 0 },
  { "game/models/player/GL.obj",
    "game/models/player/", 200.0F, 0.05F, 10.0F, 0 },
  { "game/models/player/gcon.obj",
    "game/models/player/", 200.0F, 0.05F, 10.0F, 0 },
  { "game/models/medkit/medkit.obj",
    "game/models/medkit/", 200.0F, 0.05F, 10.0F, 100 },
};
}  // namespace Demo
