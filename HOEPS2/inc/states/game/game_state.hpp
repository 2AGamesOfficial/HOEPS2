#pragma once
#include "state/state.hpp"
#include "state/global_state_type.hpp"
#include "./player/player.hpp"
#include "./player/camera.hpp"
#include "./renderer/game_renderer.hpp"
#include "./terrain/terrain.hpp"
#include "./ship/ship.hpp"
#include "./hud/hud.hpp"
#include "./enemy/enemy_manager.hpp"
#include "./projectile/projectile.hpp"
#include "./pickup/pickup_manager.hpp"

using std::unique_ptr;
using Tyra::StaticMesh;
using Tyra::StaPipOptions;

namespace Demo {

class GameState : public State<GlobalStateType> {
 public:
  GameState(Engine* t_engine);
  ~GameState();

  const GlobalStateType& getState() const { return state; }
  const bool& wantFinish() const { return _wantFinish; };

  void onStart();
  void update();
  GlobalStateType onFinish();

 private:
  GlobalStateType state;
  bool _wantFinish;
  bool initialized;
  u8 fpsChecker;
  u8 tracerTimer;
  u8 damageTimer;
  int deathTimer;
  int kills;
  audsrv_adpcm_t* deathVoiceAdpcm;
  audsrv_adpcm_t* introVoice;
  int introVoiceDelay;
  int songStartDelay;
  bool songStarted;
  bool paused;
  int pauseSelection;
  int tkState;  // 0=OFF, 1=P1_REQ, 2=P2_REQ, 3=ON
  Tyra::Sprite* tkSprite;
  Tyra::Texture* tkOnTexture;
  Tyra::Texture* tkReqTexture;
  Tyra::Sprite* respawnSprite;
  Tyra::Texture* respawnTexture;  // 0 = Resume, 1 = Quit
  bool returnToMenu;
  bool assetsLoaded;
  Tyra::Sprite* pauseSprite;
  Tyra::Texture* pauseTexture;
  Tyra::Sprite* resumeSprite;
  Tyra::Texture* resumeTexture;
  Tyra::Texture* resumeHlTexture;
  Tyra::Sprite* retrySprite;
  Tyra::Texture* retryTexture;
  Tyra::Texture* retryHlTexture;
  Tyra::Sprite* quitSprite;
  Tyra::Texture* quitTexture;
  Tyra::Texture* quitHlTexture;
  audsrv_adpcm_t* playerHitAdpcm;
  audsrv_adpcm_t* gameMusicAdpcm;
  Tyra::Vec4 tracerFrom;
  Tyra::Vec4 tracerTo;

  GameRenderer renderer;
  unique_ptr<Player> player;   // P1 (alias: first player)
  unique_ptr<Player> player2;  // P2
  unique_ptr<Tyra::Pad> pad2;  // P2 controller (port 1)
  Camera camera;               // shared camera
  Tyra::Vec4 cameraFocus;
  float cameraZoom;      // smoothed camera focus for interpolation
  int rumbleTimerP1;           // frames remaining of rumble
  u8 rumbleSmallP1;            // small actuator intensity
  u8 rumbleBigP1;              // big actuator intensity
  int rumbleTimerP2;
  u8 rumbleSmallP2;
  u8 rumbleBigP2;
  unique_ptr<EnemyManager> enemyManager;
  unique_ptr<Terrain> terrain;
  unique_ptr<Ship> ship;
  unique_ptr<Hud> hud;
  unique_ptr<Hud> hud2;
  unique_ptr<PickupManager> pickupManager;
  int deathTimer2;  // P2 death timer (P1 uses deathTimer)
  int damageTimer2; // P2 damage cooldown
  int playerBloodTimer2;

  StaticMesh* lavaMesh;
  StaPipOptions* lavaOptions;
  RendererStaticPair* lavaPair;

  static const int MAX_TRACERS = 5;  // shotgun pellet count
  StaticMesh* tracerMeshes[MAX_TRACERS];
  StaPipOptions* tracerOptions[MAX_TRACERS];
  RendererStaticPair* tracerPairs[MAX_TRACERS];

  // Blue tracer for guncon (piercing laser)
  StaticMesh* blueTracerMesh;
  StaPipOptions* blueTracerOptions;
  RendererStaticPair* blueTracerPair;

  // P2 tracer pool
  StaticMesh* tracerMeshes2[MAX_TRACERS];
  StaPipOptions* tracerOptions2[MAX_TRACERS];
  RendererStaticPair* tracerPairs2[MAX_TRACERS];
  StaticMesh* blueTracerMesh2;
  StaPipOptions* blueTracerOptions2;
  RendererStaticPair* blueTracerPair2;
  u8 tracerTimer2;
  StaticMesh* muzzleFlashMeshes2[WEAPON_COUNT];
  StaPipOptions* muzzleFlashOptions2[WEAPON_COUNT];
  RendererStaticPair* muzzleFlashPairs2[WEAPON_COUNT];

  // One muzzle flash per weapon type (nullptr if weapon has no flash)
  StaticMesh* muzzleFlashMeshes[WEAPON_COUNT];
  StaPipOptions* muzzleFlashOptions[WEAPON_COUNT];
  RendererStaticPair* muzzleFlashPairs[WEAPON_COUNT];

  static const int MAX_PROJECTILES = 4;
  Projectile projectiles[MAX_PROJECTILES];

  // Explosion effect pool (mirrors MAX_PROJECTILES)
  static const int MAX_EXPLOSIONS = 4;
  StaticMesh* explosionMeshes[MAX_EXPLOSIONS];
  StaPipOptions* explosionOptions[MAX_EXPLOSIONS];
  RendererStaticPair* explosionPairs[MAX_EXPLOSIONS];
  int explosionLifetimes[MAX_EXPLOSIONS];
  float explosionRotations[MAX_EXPLOSIONS];
  float explosionScaleMuls[MAX_EXPLOSIONS];
  Tyra::Vec4 explosionPositions[MAX_EXPLOSIONS];
  audsrv_adpcm_t* glImpactAdpcm;
  audsrv_adpcm_t* shotgunPickupAdpcm;

  // Sparks pool (ophanim death effect)
  static const int MAX_SPARKS = 4;
  StaticMesh* sparksMeshes[MAX_SPARKS];
  StaPipOptions* sparksOptions[MAX_SPARKS];
  RendererStaticPair* sparksPairs[MAX_SPARKS];
  int sparksLifetimes[MAX_SPARKS];
  float sparksRotations[MAX_SPARKS];
  Tyra::Vec4 sparksPositions[MAX_SPARKS];
  audsrv_adpcm_t* ricochetAdpcm;
  audsrv_adpcm_t* ophanimDeathAdpcm;
  StaticMesh* playerBloodMesh;
  StaPipOptions* playerBloodOptions;
  RendererStaticPair* playerBloodPair;
  int playerBloodTimer;

  void spawnProjectile(const Tyra::Vec4& origin, const Tyra::Vec4& dir);
  void updateProjectiles();
  void spawnExplosion(const Tyra::Vec4& position, float scaleMul = 1.0F);
  void updateExplosions();
  void spawnSparks(const Tyra::Vec4& position);
  void updateSparks();
};

}  // namespace Demo
