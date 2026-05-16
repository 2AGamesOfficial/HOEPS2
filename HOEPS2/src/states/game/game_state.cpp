#include "states/game/game_state.hpp"
#include <string>
#include "game_settings.hpp"
#include "states/game/colliders.hpp"
#include "states/intro/intro_state.hpp"

using std::make_unique;
using Tyra::FileUtils;
using Tyra::ObjLoader;
using Tyra::ObjLoaderOptions;

namespace Demo {

GameState::GameState(Engine* t_engine)
    : State(t_engine), renderer(&t_engine->renderer) {
  state = STATE_GAME;
  _wantFinish = false;
  initialized = false;
  fpsChecker = 0;
  lavaMesh = nullptr;
  lavaOptions = nullptr;
  lavaPair = nullptr;
  for (int i = 0; i < MAX_TRACERS; i++) {
    tracerMeshes[i] = nullptr;
    tracerOptions[i] = nullptr;
    tracerPairs[i] = nullptr;
  }
  blueTracerMesh = nullptr;
  blueTracerOptions = nullptr;
  blueTracerPair = nullptr;
  for (int i = 0; i < MAX_TRACERS; i++) {
    tracerMeshes2[i] = nullptr;
    tracerOptions2[i] = nullptr;
    tracerPairs2[i] = nullptr;
  }
  blueTracerMesh2 = nullptr;
  blueTracerOptions2 = nullptr;
  blueTracerPair2 = nullptr;
  tracerTimer2 = 0;
  for (int i = 0; i < WEAPON_COUNT; i++) {
    muzzleFlashMeshes2[i] = nullptr;
    muzzleFlashOptions2[i] = nullptr;
    muzzleFlashPairs2[i] = nullptr;
  }
  for (int i = 0; i < WEAPON_COUNT; i++) {
    muzzleFlashMeshes[i] = nullptr;
    muzzleFlashOptions[i] = nullptr;
    muzzleFlashPairs[i] = nullptr;
  }
  for (int i = 0; i < MAX_EXPLOSIONS; i++) {
    explosionMeshes[i] = nullptr;
    explosionOptions[i] = nullptr;
    explosionPairs[i] = nullptr;
    explosionLifetimes[i] = 0;
    explosionScaleMuls[i] = 1.0F;
  }
  for (int i = 0; i < MAX_SPARKS; i++) {
    sparksMeshes[i] = nullptr;
    sparksOptions[i] = nullptr;
    sparksPairs[i] = nullptr;
    sparksLifetimes[i] = 0;
  }
  glImpactAdpcm = nullptr;
  deathVoiceAdpcm = nullptr;
  introVoice = nullptr;
  introVoiceDelay = 0;
  songStartDelay = 0;
  songStarted = false;
  shotgunPickupAdpcm = nullptr;
  ricochetAdpcm = nullptr;
  ophanimDeathAdpcm = nullptr;
  playerHitAdpcm = nullptr;
  playerBloodMesh = nullptr;
  playerBloodOptions = nullptr;
  playerBloodPair = nullptr;
  playerBloodTimer = 0;
  deathTimer = 0;
  cameraFocus = Tyra::Vec4(0.0F, 50.0F, 0.0F);
  cameraZoom = 1.0F;
  rumbleTimerP1 = 0; rumbleSmallP1 = 0; rumbleBigP1 = 0;
  rumbleTimerP2 = 0; rumbleSmallP2 = 0; rumbleBigP2 = 0;
  deathTimer2 = 0;
  damageTimer2 = 0;
  playerBloodTimer2 = 0;
  kills = 0;
  damageTimer = 0;
  tracerTimer = 0;
  paused = false;
  pauseSelection = 0;
  tkState = 0;
  tkSprite = nullptr;
  tkOnTexture = nullptr;
  tkReqTexture = nullptr;
  respawnSprite = nullptr;
  respawnTexture = nullptr;
  returnToMenu = false;
  assetsLoaded = false;
  pauseSprite = nullptr;
  pauseTexture = nullptr;
  resumeSprite = nullptr;
  resumeTexture = nullptr;
  resumeHlTexture = nullptr;
  retrySprite = nullptr;
  retryTexture = nullptr;
  retryHlTexture = nullptr;
  quitSprite = nullptr;
  quitTexture = nullptr;
  quitHlTexture = nullptr;
}

GameState::~GameState() {
  if (lavaMesh) delete lavaMesh;
  if (lavaOptions) delete lavaOptions;
  if (lavaPair) delete lavaPair;
  for (int i = 0; i < MAX_TRACERS; i++) {
    if (tracerMeshes[i]) delete tracerMeshes[i];
    if (tracerOptions[i]) delete tracerOptions[i];
    if (tracerPairs[i]) delete tracerPairs[i];
  }
  if (blueTracerMesh) delete blueTracerMesh;
  if (blueTracerOptions) delete blueTracerOptions;
  if (blueTracerPair) delete blueTracerPair;
  for (int i = 0; i < MAX_TRACERS; i++) {
    if (tracerMeshes2[i]) delete tracerMeshes2[i];
    if (tracerOptions2[i]) delete tracerOptions2[i];
    if (tracerPairs2[i]) delete tracerPairs2[i];
  }
  if (blueTracerMesh2) delete blueTracerMesh2;
  if (blueTracerOptions2) delete blueTracerOptions2;
  if (blueTracerPair2) delete blueTracerPair2;
  for (int i = 0; i < WEAPON_COUNT; i++) {
    if (muzzleFlashMeshes2[i]) delete muzzleFlashMeshes2[i];
    if (muzzleFlashOptions2[i]) delete muzzleFlashOptions2[i];
    if (muzzleFlashPairs2[i]) delete muzzleFlashPairs2[i];
  }
  for (int i = 0; i < WEAPON_COUNT; i++) {
    if (muzzleFlashMeshes[i]) delete muzzleFlashMeshes[i];
    if (muzzleFlashOptions[i]) delete muzzleFlashOptions[i];
    if (muzzleFlashPairs[i]) delete muzzleFlashPairs[i];
  }
  for (int i = 0; i < MAX_EXPLOSIONS; i++) {
    if (explosionMeshes[i]) delete explosionMeshes[i];
    if (explosionOptions[i]) delete explosionOptions[i];
    if (explosionPairs[i]) delete explosionPairs[i];
  }
  for (int i = 0; i < MAX_SPARKS; i++) {
    if (sparksMeshes[i]) delete sparksMeshes[i];
    if (sparksOptions[i]) delete sparksOptions[i];
    if (sparksPairs[i]) delete sparksPairs[i];
  }
  // adpcm handles are engine-owned; do not delete them
  if (playerBloodMesh) delete playerBloodMesh;
  if (playerBloodOptions) delete playerBloodOptions;
  if (playerBloodPair) delete playerBloodPair;
  if (pauseSprite) delete pauseSprite;
  if (resumeSprite) delete resumeSprite;
  if (retrySprite) delete retrySprite;
  if (quitSprite) delete quitSprite;
  // textures freed by repository on engine shutdown
}

void GameState::onStart() {
  TYRA_LOG("Game. RAM: ", engine->info.getAvailableRAM(), "MB");
  if (assetsLoaded) {
    // Returning from menu — assets persist; level state was reset in Quit handler
    engine->audio.song.stop();
    engine->audio.song.inLoop = true;
    engine->audio.song.load(Tyra::FileUtils::fromCwd("game/game-audio.wav"));
    engine->audio.song.setVolume(0);
    songStartDelay = 30;
    songStarted = false;
    introVoiceDelay = 90;
    initialized = true;
    return;
  }
  assetsLoaded = true;
  renderer.dbgInfo = &engine->info;

  engine->audio.song.stop();
  engine->audio.song.inLoop = true;
  engine->audio.song.load(FileUtils::fromCwd("game/game-audio.wav"));
  engine->audio.song.setVolume(0);
  // Do NOT play() yet — deferred to update() after assets finish loading

  auto* repository = &engine->renderer.core.texture.repository;

  playerHitAdpcm = engine->audio.adpcm.load(
      FileUtils::fromCwd("game/audio/player_hit.adpcm"));
  // Player hit sound may play on any free channel (tryPlay with -1), so set
  // volume on a range of channels to be safe.
  for (u8 ch = 0; ch < 24; ch++) {
    engine->audio.adpcm.setVolume(90, ch);
  }
  // Explicit override: player hit sound on ch 8 — ensure audible
  engine->audio.adpcm.setVolume(100, 8);
  engine->audio.adpcm.setVolume(100, 9);
  engine->audio.adpcm.setVolume(100, 10);

  // P1 uses engine's built-in pad (port 0). P2 gets its own pad on port 1.
  player = make_unique<Player>(engine, &engine->pad, 0);
  pad2 = make_unique<Tyra::Pad>(1);
  pad2->init();  // manual init for port 1 (engine only inits port 0)
  player2 = make_unique<Player>(engine, pad2.get(), 1);
  // Offset P2 starting position so they don't overlap
  // (Player ctor sets position=(0,50,0); nudge P2)
  terrain = make_unique<Terrain>(repository);
  enemyManager = make_unique<EnemyManager>(engine, terrain->heightmap);
  ship = make_unique<Ship>(repository);
  hud = make_unique<Hud>(repository, Hud::LEFT);
  hud2 = make_unique<Hud>(repository, Hud::RIGHT);
  pickupManager = make_unique<PickupManager>(engine);

  // Team Kill icons (top-center, 64x64)
  {
    const auto& tkSettings = engine->renderer.core.getSettings();
    tkSprite = new Sprite;
    tkSprite->size.set(64.0F, 64.0F);
    tkSprite->scale = 0.5F;
    tkSprite->position.set((float)tkSettings.getWidth() / 2.0F - 16.0F, 8.0F);
  }
  tkOnTexture = engine->renderer.core.texture.repository.add(
      FileUtils::fromCwd("game/hud/TKon.png"));
  tkReqTexture = engine->renderer.core.texture.repository.add(
      FileUtils::fromCwd("game/hud/TKReq.png"));
  // No link until tkState != 0

  // Respawn waiting indicator (centered)
  {
    const auto& rsSettings = engine->renderer.core.getSettings();
    respawnSprite = new Sprite;
    respawnSprite->size.set(512.0F, 128.0F);
    respawnSprite->scale = 0.5F;
    respawnSprite->position.set((float)rsSettings.getWidth() / 2.0F - 128.0F,
                                (float)rsSettings.getHeight() / 2.0F - 32.0F);
  }
  respawnTexture = engine->renderer.core.texture.repository.add(
      FileUtils::fromCwd("game/hud/respawn.png"));
  respawnTexture->addLink(respawnSprite->id);

  pickupManager->init();

  ObjLoader loader;
  ObjLoaderOptions objOptions;
  objOptions.scale = 2000.0F;
  objOptions.flipUVs = true;
  auto data = loader.load(
      FileUtils::fromCwd("game/models/lava/lava.obj"), objOptions);
  data->loadNormals = false;
  lavaMesh = new StaticMesh(data.get());
  repository->addByMesh(lavaMesh, FileUtils::fromCwd("game/models/lava/"), "png");
  lavaMesh->setPosition(Vec4(0.0F, -50.0F, 0.0F));

  lavaOptions = new StaPipOptions();
  lavaOptions->frustumCulling = Tyra::PipelineFrustumCulling_None;
  lavaPair = new RendererStaticPair{lavaMesh, lavaOptions};

  // Load tracer (one master, then copy-construct MAX_TRACERS instances)
  ObjLoaderOptions tracerOpts;
  tracerOpts.scale = 1.0F;
  tracerOpts.flipUVs = true;
  auto tracerData = loader.load(
      FileUtils::fromCwd("game/models/tracer/tracer.obj"), tracerOpts);
  tracerData->loadNormals = false;
  // First mesh + add texture to repository
  tracerMeshes[0] = new StaticMesh(tracerData.get());
  repository->addByMesh(tracerMeshes[0], FileUtils::fromCwd("game/models/tracer/"), "png");
  tracerMeshes[0]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
  tracerOptions[0] = new StaPipOptions();
  tracerOptions[0]->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
  tracerPairs[0] = new RendererStaticPair{tracerMeshes[0], tracerOptions[0]};

  // Find tracer texture for re-linking copies
  Tyra::Texture* tracerTex = nullptr;
  for (auto* tex : *repository->getAll()) {
    if (tex->name == "tracer.png") {
      tracerTex = tex;
      break;
    }
  }

  // Copy-construct remaining tracers and link texture
  for (int i = 1; i < MAX_TRACERS; i++) {
    tracerMeshes[i] = new StaticMesh(*tracerMeshes[0]);
    tracerMeshes[i]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    if (tracerTex != nullptr) {
      for (auto* mat : tracerMeshes[i]->materials) {
        tracerTex->addLink(mat->id);
      }
    }
    tracerOptions[i] = new StaPipOptions();
    tracerOptions[i]->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
    tracerPairs[i] = new RendererStaticPair{tracerMeshes[i], tracerOptions[i]};
  }

  // Blue tracer for guncon
  {
    ObjLoaderOptions btOpts;
    btOpts.scale = 1.0F;
    btOpts.flipUVs = true;
    auto btData = loader.load(
        FileUtils::fromCwd("game/models/tracer_blue/tracer_blue.obj"), btOpts);
    btData->loadNormals = false;
    blueTracerMesh = new StaticMesh(btData.get());
    repository->addByMesh(blueTracerMesh,
        FileUtils::fromCwd("game/models/tracer_blue/"), "png");
    blueTracerMesh->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    blueTracerOptions = new StaPipOptions();
    blueTracerOptions->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
    blueTracerOptions->blendingEnabled = true;
    blueTracerPair = new RendererStaticPair{blueTracerMesh, blueTracerOptions};
  }

  // Build P2 tracer pool (copies of P1's meshes)
  {
    Tyra::Texture* tracerTex = nullptr;
    Tyra::Texture* blueTex = nullptr;
    for (auto* tex : *repository->getAll()) {
      if (tex->name == "tracer.png") tracerTex = tex;
      else if (tex->name == "tracer_blue.png") blueTex = tex;
    }
    for (int i = 0; i < MAX_TRACERS; i++) {
      tracerMeshes2[i] = new StaticMesh(*tracerMeshes[0]);
      tracerMeshes2[i]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
      if (tracerTex != nullptr) {
        for (auto* mat : tracerMeshes2[i]->materials) tracerTex->addLink(mat->id);
      }
      tracerOptions2[i] = new StaPipOptions();
      tracerOptions2[i]->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
      tracerPairs2[i] = new RendererStaticPair{tracerMeshes2[i], tracerOptions2[i]};
    }
    blueTracerMesh2 = new StaticMesh(*blueTracerMesh);
    blueTracerMesh2->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    if (blueTex != nullptr) {
      for (auto* mat : blueTracerMesh2->materials) blueTex->addLink(mat->id);
    }
    blueTracerOptions2 = new StaPipOptions();
    blueTracerOptions2->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
    blueTracerOptions2->blendingEnabled = true;
    blueTracerPair2 = new RendererStaticPair{blueTracerMesh2, blueTracerOptions2};
  }

  // Load one muzzle flash mesh per weapon that has a flash
  for (int i = 0; i < WEAPON_COUNT; i++) {
    if (WEAPON_DATA[i].muzzleFlashObjPath == nullptr) continue;
    ObjLoaderOptions mfOpts;
    mfOpts.scale = 1.0F;
    mfOpts.flipUVs = true;
    auto mfData = loader.load(
        FileUtils::fromCwd(WEAPON_DATA[i].muzzleFlashObjPath), mfOpts);
    mfData->loadNormals = false;
    muzzleFlashMeshes[i] = new StaticMesh(mfData.get());
    repository->addByMesh(muzzleFlashMeshes[i],
        FileUtils::fromCwd("game/models/muzzleflash/"), "png");
    muzzleFlashMeshes[i]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    muzzleFlashOptions[i] = new StaPipOptions();
    muzzleFlashOptions[i]->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
    muzzleFlashOptions[i]->blendingEnabled = true;
    muzzleFlashPairs[i] = new RendererStaticPair{muzzleFlashMeshes[i], muzzleFlashOptions[i]};
  }

  // P2 muzzle flash copies
  for (int i = 0; i < WEAPON_COUNT; i++) {
    if (muzzleFlashMeshes[i] == nullptr) continue;
    muzzleFlashMeshes2[i] = new StaticMesh(*muzzleFlashMeshes[i]);
    muzzleFlashMeshes2[i]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    // Link same texture as P1's flash (find by name)
    std::string texName;
    for (auto* mat : muzzleFlashMeshes[i]->materials) {
      for (auto* tex : *repository->getAll()) {
        if (tex->isLinkedWith(mat->id)) { texName = tex->name; break; }
      }
      if (!texName.empty()) break;
    }
    if (!texName.empty()) {
      for (auto* tex : *repository->getAll()) {
        if (tex->name == texName) {
          for (auto* mat : muzzleFlashMeshes2[i]->materials) tex->addLink(mat->id);
          break;
        }
      }
    }
    muzzleFlashOptions2[i] = new StaPipOptions();
    muzzleFlashOptions2[i]->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
    muzzleFlashOptions2[i]->blendingEnabled = true;
    muzzleFlashPairs2[i] = new RendererStaticPair{muzzleFlashMeshes2[i], muzzleFlashOptions2[i]};
  }

  // Projectile pool (4 grenades). Load grenade mesh once, copy for each.
  {
    ObjLoaderOptions grenadeOpts;
    grenadeOpts.scale = 1.0F;
    grenadeOpts.flipUVs = true;
    auto grenadeData = loader.load(
        FileUtils::fromCwd("game/models/grenade/grenade.obj"), grenadeOpts);
    grenadeData->loadNormals = false;

    // Master mesh + texture registration
    StaticMesh* masterGrenade = new StaticMesh(grenadeData.get());
    repository->addByMesh(masterGrenade,
        FileUtils::fromCwd("game/models/grenade/"), "png");

    // Find Grenade texture for re-linking copies
    Tyra::Texture* grenadeTex = nullptr;
    for (auto* tex : *repository->getAll()) {
      if (tex->name == "Grenade.png") {
        grenadeTex = tex;
        break;
      }
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) {
      projectiles[i].mesh = new StaticMesh(*masterGrenade);
      projectiles[i].mesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
      projectiles[i].mesh->scale.identity();
      projectiles[i].mesh->scale.scale(3.0F);

      if (grenadeTex != nullptr) {
        for (auto* mat : projectiles[i].mesh->materials) {
          grenadeTex->addLink(mat->id);
        }
      }

      projectiles[i].options = new StaPipOptions();
      projectiles[i].options->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
      projectiles[i].options->blendingEnabled = true;
      projectiles[i].pair = new RendererStaticPair{projectiles[i].mesh, projectiles[i].options};
    }

    // masterGrenade intentionally not deleted (copies share data)
  }

  // Explosion pool
  {
    ObjLoaderOptions explOpts;
    explOpts.scale = 1.0F;
    explOpts.flipUVs = true;
    auto explData = loader.load(
        FileUtils::fromCwd("game/models/explosion/explosion.obj"), explOpts);
    explData->loadNormals = false;

    StaticMesh* masterExpl = new StaticMesh(explData.get());
    repository->addByMesh(masterExpl,
        FileUtils::fromCwd("game/models/explosion/"), "png");

    Tyra::Texture* explTex = nullptr;
    for (auto* tex : *repository->getAll()) {
      if (tex->name == "Explosion.png") {
        explTex = tex;
        break;
      }
    }

    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
      explosionMeshes[i] = new StaticMesh(*masterExpl);
      explosionMeshes[i]->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
      if (explTex != nullptr) {
        for (auto* mat : explosionMeshes[i]->materials) {
          explTex->addLink(mat->id);
        }
      }
      explosionOptions[i] = new StaPipOptions();
      explosionOptions[i]->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
      explosionOptions[i]->blendingEnabled = true;
      explosionPairs[i] = new RendererStaticPair{explosionMeshes[i], explosionOptions[i]};
    }
    // masterExpl intentionally not deleted (copies share data)
  }

  // Sparks pool (ophanim death)
  {
    ObjLoaderOptions sparksOpts;
    sparksOpts.scale = 1.0F;
    sparksOpts.flipUVs = true;
    auto sparksData = loader.load(
        FileUtils::fromCwd("game/models/sparks/sparks.obj"), sparksOpts);
    sparksData->loadNormals = false;
    StaticMesh* masterSparks = new StaticMesh(sparksData.get());
    repository->addByMesh(masterSparks,
        FileUtils::fromCwd("game/models/sparks/"), "png");
    Tyra::Texture* sparksTex = nullptr;
    for (auto* tex : *repository->getAll()) {
      if (tex->name == "Sparks.png") { sparksTex = tex; break; }
    }
    for (int i = 0; i < MAX_SPARKS; i++) {
      sparksMeshes[i] = new StaticMesh(*masterSparks);
      sparksMeshes[i]->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
      if (sparksTex != nullptr) {
        for (auto* mat : sparksMeshes[i]->materials) {
          sparksTex->addLink(mat->id);
        }
      }
      sparksOptions[i] = new StaPipOptions();
      sparksOptions[i]->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
      sparksOptions[i]->blendingEnabled = true;
      sparksPairs[i] = new RendererStaticPair{sparksMeshes[i], sparksOptions[i]};
    }
    // masterSparks intentionally not deleted (copies share data)
  }

  glImpactAdpcm = engine->audio.adpcm.load(
      FileUtils::fromCwd("game/audio/gl_impact.adpcm"));
  engine->audio.adpcm.setVolume(80, 21);
  engine->audio.adpcm.setVolume(80, 16);
  engine->audio.adpcm.setVolume(80, 17);

  // Song volume set when song actually starts playing (see update())
  songStartDelay = 30;
  songStarted = false;

  engine->renderer.setFrameLimit(true);  // cap at 60 for native PS2 feel

  paused = false;
  pauseSprite = new Tyra::Sprite();
  pauseSprite->mode = Tyra::MODE_STRETCH;
  pauseSprite->size.set(154.0F, 77.0F);
  pauseSprite->position.set(512.0F / 2.0F - 77.0F, 100.0F);
  pauseTexture = repository->add(FileUtils::fromCwd("game/hud/pause.png"));
  pauseTexture->addLink(pauseSprite->id);

  // Pause menu options (Resume / Quit)
  resumeSprite = new Tyra::Sprite();
  resumeSprite->mode = Tyra::MODE_STRETCH;
  resumeSprite->size.set(128.0F, 64.0F);
  resumeSprite->position.set(512.0F / 2.0F - 64.0F, 200.0F);
  resumeTexture = repository->add(FileUtils::fromCwd("game/hud/resume.png"));
  resumeHlTexture = repository->add(FileUtils::fromCwd("game/hud/resume_hl.png"));
  resumeHlTexture->addLink(resumeSprite->id);  // start highlighted

  retrySprite = new Tyra::Sprite();
  retrySprite->mode = Tyra::MODE_STRETCH;
  retrySprite->size.set(128.0F, 64.0F);
  retrySprite->position.set(512.0F / 2.0F - 64.0F, 270.0F);
  retryTexture = repository->add(FileUtils::fromCwd("game/hud/retry.png"));
  retryHlTexture = repository->add(FileUtils::fromCwd("game/hud/retry_hl.png"));
  retryTexture->addLink(retrySprite->id);

  quitSprite = new Tyra::Sprite();
  quitSprite->mode = Tyra::MODE_STRETCH;
  quitSprite->size.set(128.0F, 64.0F);
  quitSprite->position.set(512.0F / 2.0F - 64.0F, 340.0F);
  quitTexture = repository->add(FileUtils::fromCwd("game/hud/quit.png"));
  quitHlTexture = repository->add(FileUtils::fromCwd("game/hud/quit_hl.png"));
  quitTexture->addLink(quitSprite->id);

  deathVoiceAdpcm = engine->audio.adpcm.load(
      FileUtils::fromCwd("game/audio/death_voice.adpcm"));

  shotgunPickupAdpcm = engine->audio.adpcm.load(
      FileUtils::fromCwd("game/audio/shotgun_pickup.adpcm"));

  ricochetAdpcm = engine->audio.adpcm.load(
      FileUtils::fromCwd("game/audio/ricochet.adpcm"));

  ophanimDeathAdpcm = engine->audio.adpcm.load(
      FileUtils::fromCwd("game/audio/ophanim_death.adpcm"));

  // Player death blood splash — use same shared Blood.png as enemies to match look
  {
    ObjLoaderOptions opts;
    opts.scale = 180.0F;
    opts.flipUVs = true;
    auto data = loader.load(FileUtils::fromCwd("game/models/blood/blood.obj"), opts);
    data->loadNormals = false;
    playerBloodMesh = new StaticMesh(data.get());
    // Do NOT addByMesh (that creates a duplicate Blood.png texture).
    // Find the already-loaded Blood.png (from enemy manager) and link to it.
    Tyra::Texture* existingBlood = nullptr;
    for (auto* tex : *repository->getAll()) {
      if (tex->name == "Blood.png") { existingBlood = tex; break; }
    }
    if (existingBlood != nullptr) {
      for (auto* mat : playerBloodMesh->materials) {
        existingBlood->addLink(mat->id);
      }
    } else {
      // Fallback — shouldn't happen since enemy_manager loads Blood.png first
      repository->addByMesh(playerBloodMesh, FileUtils::fromCwd("game/models/blood/"), "png");
    }
    playerBloodMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
    playerBloodOptions = new StaPipOptions();
    playerBloodOptions->frustumCulling = Tyra::PipelineFrustumCulling_Simple;
    playerBloodOptions->blendingEnabled = true;
    playerBloodPair = new RendererStaticPair{playerBloodMesh, playerBloodOptions};
  }

  introVoice = engine->audio.adpcm.load(
      FileUtils::fromCwd("game/cant_have_that.adpcm"));
  // Delay 90 frames (~1.8s) before playing — avoids initial song glitch
  introVoiceDelay = 90;

  initialized = true;
}

GlobalStateType GameState::onFinish() {
  _wantFinish = false;
  if (!initialized) return STATE_EXIT;
  initialized = false;
  bool toMenu = returnToMenu;
  returnToMenu = false;
  return toMenu ? STATE_INTRO : STATE_EXIT;
}

void GameState::update() {
  if (pad2 && pad2->isReady) pad2->update();  // manually pump P2 pad (P1 updated by engine loop)

  // Team kill toggle: requires both players to consent
  {
    bool p1Sel = engine->pad.getClicked().Select;
    bool p2Sel = (player2->isJoined && pad2 && pad2->isReady) ? pad2->getClicked().Select : false;
    if (player2->isJoined) {
      // OFF -> request
      if (tkState == 0 && p1Sel) tkState = 1;
      else if (tkState == 0 && p2Sel) tkState = 2;
      // P1 requested, P2 confirms
      else if (tkState == 1 && p2Sel) tkState = 3;
      // P2 requested, P1 confirms
      else if (tkState == 2 && p1Sel) tkState = 3;
      // ON -> back to OFF (either player)
      else if (tkState == 3 && (p1Sel || p2Sel)) tkState = 0;
      // Same player presses again to cancel their own request
      else if (tkState == 1 && p1Sel) tkState = 0;
      else if (tkState == 2 && p2Sel) tkState = 0;
    } else {
      tkState = 0;
    }
    // Update sprite link based on state (track current link to avoid double-link/remove)
    static int prevTkState = -1;
    if (tkState != prevTkState) {
      if (prevTkState == 1 || prevTkState == 2) tkReqTexture->removeLinkById(tkSprite->id);
      else if (prevTkState == 3) tkOnTexture->removeLinkById(tkSprite->id);
      if (tkState == 1 || tkState == 2) tkReqTexture->addLink(tkSprite->id);
      else if (tkState == 3) tkOnTexture->addLink(tkSprite->id);
      prevTkState = tkState;
    }
  }

  // Rumble tick — decrement timers and apply
  if (rumbleTimerP1 > 0) {
    rumbleTimerP1--;
    engine->pad.setRumble(rumbleSmallP1, rumbleBigP1);
    if (rumbleTimerP1 == 0) { rumbleSmallP1 = 0; rumbleBigP1 = 0; engine->pad.setRumble(0, 0); }
  }
  if (rumbleTimerP2 > 0 && pad2 && pad2->isReady) {
    rumbleTimerP2--;
    pad2->setRumble(rumbleSmallP2, rumbleBigP2);
    if (rumbleTimerP2 == 0) { rumbleSmallP2 = 0; rumbleBigP2 = 0; pad2->setRumble(0, 0); }
  }
  if (introVoiceDelay > 0) {
    introVoiceDelay--;
    if (introVoiceDelay == 0 && introVoice != nullptr) {
      engine->audio.adpcm.tryPlay(introVoice, 20);
    }
  }
  if (!songStarted && songStartDelay > 0) {
    songStartDelay--;
    if (songStartDelay == 0) {
      engine->audio.song.play();
      engine->audio.song.setVolume(85);
      songStarted = true;
    }
  }
  if (fpsChecker++ > 50) {
    TYRA_LOG("FPS: ", engine->info.getFps(),
             " RAM: ", engine->info.getAvailableRAM());
    fpsChecker = 0;
  }

  if (engine->pad.getClicked().Start || (player2->isJoined && pad2 && pad2->isReady && pad2->getClicked().Start)) {
    paused = !paused;
    if (!paused && pauseSelection != 0) {
      // Restore sprites to Resume-highlighted default
      if (pauseSelection == 1) {
        retryHlTexture->removeLinkById(retrySprite->id);
        retryTexture->addLink(retrySprite->id);
      } else {
        quitHlTexture->removeLinkById(quitSprite->id);
        quitTexture->addLink(quitSprite->id);
      }
      resumeTexture->removeLinkById(resumeSprite->id);
      resumeHlTexture->addLink(resumeSprite->id);
      pauseSelection = 0;
    }
    if (paused) pauseSelection = 0;
  }

  if (paused) {
    // Navigation — either controller
    const auto& clicked = engine->pad.getClicked();
    Tyra::PadButtons clicked2 = (pad2 && pad2->isReady) ? pad2->getClicked() : Tyra::PadButtons{};
    bool up = clicked.DpadUp || (player2->isJoined && clicked2.DpadUp);
    bool down = clicked.DpadDown || (player2->isJoined && clicked2.DpadDown);
    bool cross = clicked.Cross || (player2->isJoined && clicked2.Cross);
    int oldSel = pauseSelection;
    if (down && pauseSelection < 2) pauseSelection++;
    if (up && pauseSelection > 0) pauseSelection--;
    if (pauseSelection != oldSel) {
      // Un-highlight old
      if (oldSel == 0) { resumeHlTexture->removeLinkById(resumeSprite->id); resumeTexture->addLink(resumeSprite->id); }
      else if (oldSel == 1) { retryHlTexture->removeLinkById(retrySprite->id); retryTexture->addLink(retrySprite->id); }
      else { quitHlTexture->removeLinkById(quitSprite->id); quitTexture->addLink(quitSprite->id); }
      // Highlight new
      if (pauseSelection == 0) { resumeTexture->removeLinkById(resumeSprite->id); resumeHlTexture->addLink(resumeSprite->id); }
      else if (pauseSelection == 1) { retryTexture->removeLinkById(retrySprite->id); retryHlTexture->addLink(retrySprite->id); }
      else { quitTexture->removeLinkById(quitSprite->id); quitHlTexture->addLink(quitSprite->id); }
    }
    if (cross) {
      if (pauseSelection == 0) {
        paused = false;
      } else if (pauseSelection == 1) {
        // Retry: soft reset state for both players
        player->resetHealth();
        player->resetPosition();
        player->resetWeapons();
        deathTimer = 0;
        damageTimer = 0;
        playerBloodTimer = 0;
        if (player2->isJoined) {
          player2->resetHealth();
          player2->resetPosition();
          player2->resetWeapons();
          deathTimer2 = 0;
          damageTimer2 = 0;
          playerBloodTimer2 = 0;
        }
        pickupManager->clearAll();
        kills = 0;
        enemyManager->setDifficulty(0);
        // Park ALL enemies: 0+1 restart pre-spawn, 2+3 locked until difficulty
        auto& es = const_cast<std::vector<Enemy*>&>(enemyManager->getEnemies());
        for (int i = 0; i < (int)es.size(); i++) {
          es[i]->recentlyKilled = false;
          es[i]->pendingExplosion = false;
          es[i]->bloodTimer = 0;
          es[i]->hp = es[i]->currentMaxHp;
        }
        for (int i = 0; i < (int)es.size(); i++) {
          es[i]->ophanimMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
          es[i]->demonLegsMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
          es[i]->demonUpperMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
          es[i]->pentagramMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
          es[i]->spawnTimer = 0;
          if (i >= 2) {
            es[i]->preSpawnTimer = 999999;
          } else {
            es[i]->preSpawnTimer = (int)Tyra::Math::randomf(
                (float)enemyManager->preSpawnMin,
                (float)enemyManager->preSpawnMax);
          }
        }
        // Restore highlight to Resume for next pause
        retryHlTexture->removeLinkById(retrySprite->id);
        retryTexture->addLink(retrySprite->id);
        resumeTexture->removeLinkById(resumeSprite->id);
        resumeHlTexture->addLink(resumeSprite->id);
        pauseSelection = 0;
        paused = false;
      } else if (pauseSelection == 2) {
        // Quit to start menu: reset level state for next entry
        player->resetHealth();
        player->resetPosition();
        player->resetWeapons();
        deathTimer = 0;
        damageTimer = 0;
        playerBloodTimer = 0;
        if (player2->isJoined) {
          player2->resetHealth();
          player2->resetPosition();
          player2->resetWeapons();
          deathTimer2 = 0;
          damageTimer2 = 0;
          playerBloodTimer2 = 0;
        }
        pickupManager->clearAll();
        kills = 0;
        enemyManager->setDifficulty(0);
        {
          auto& es = const_cast<std::vector<Enemy*>&>(enemyManager->getEnemies());
          for (int i = 0; i < (int)es.size(); i++) {
            es[i]->recentlyKilled = false;
            es[i]->pendingExplosion = false;
            es[i]->bloodTimer = 0;
            es[i]->hp = es[i]->currentMaxHp;
            es[i]->ophanimMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
            es[i]->demonLegsMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
            es[i]->demonUpperMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
            es[i]->pentagramMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
            es[i]->spawnTimer = 0;
            if (i >= 2) {
              es[i]->preSpawnTimer = 999999;
            } else {
              es[i]->preSpawnTimer = (int)Tyra::Math::randomf(
                  (float)enemyManager->preSpawnMin,
                  (float)enemyManager->preSpawnMax);
            }
          }
        }
        // Park all transient projectiles/effects
        for (int t = 0; t < MAX_TRACERS; t++) {
          tracerMeshes[t]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
          tracerMeshes2[t]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
        }
        for (int i = 0; i < MAX_EXPLOSIONS; i++) {
          explosionLifetimes[i] = 0;
          explosionMeshes[i]->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
        }
        for (int i = 0; i < MAX_SPARKS; i++) {
          sparksLifetimes[i] = 0;
          sparksMeshes[i]->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
        }
        for (int i = 0; i < MAX_PROJECTILES; i++) {
          projectiles[i].alive = false;
          projectiles[i].hide();
        }
        for (int w = 0; w < WEAPON_COUNT; w++) {
          if (muzzleFlashMeshes[w]) muzzleFlashMeshes[w]->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
          if (muzzleFlashMeshes2[w]) muzzleFlashMeshes2[w]->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
        }
        // Reset camera + multiplayer state
        cameraFocus = Tyra::Vec4(0.0F, 50.0F, 0.0F);
        cameraZoom = 1.0F;
        player2->isJoined = false;
        // Restore pause highlight to Resume for next pause
        quitHlTexture->removeLinkById(quitSprite->id);
        quitTexture->addLink(quitSprite->id);
        resumeTexture->removeLinkById(resumeSprite->id);
        resumeHlTexture->addLink(resumeSprite->id);
        pauseSelection = 0;
        tkState = 0;
        rumbleTimerP1 = 0; rumbleSmallP1 = 0; rumbleBigP1 = 0;
        rumbleTimerP2 = 0; rumbleSmallP2 = 0; rumbleBigP2 = 0;
        engine->audio.song.stop();
        IntroState::skipToPressKey = true;
        returnToMenu = true;
        _wantFinish = true;
        paused = false;
      }
    }
    engine->renderer.beginFrame(camera.getCameraInfo());
    {
      renderer.clear();
      {
        renderer.add(lavaPair);
        renderer.add(terrain->pair);
        if (deathTimer == 0) {
          renderer.add(player->getCurrentModelPair());
          renderer.add(player->legsPair);
        }
        if (player2->isJoined && deathTimer2 == 0) {
          renderer.add(player2->getCurrentModelPair());
          renderer.add(player2->legsPair);
        }
        renderer.add(enemyManager->getPentagramPairs());
        renderer.add(enemyManager->getPairs());
        renderer.add(enemyManager->getUpperBodyPairs());
        renderer.add(enemyManager->getBloodPairs());
        renderer.add(pauseSprite);
        renderer.add(resumeSprite);
        renderer.add(retrySprite);
        renderer.add(quitSprite);
      }
      renderer.render();
    }
    engine->renderer.endFrame();
    return;
  }

  // P2 join: Start on pad2 when not yet joined
  if (!player2->isJoined && pad2 && pad2->isReady && pad2->getClicked().Start) {
    player2->isJoined = true;
    // Place P2 near P1, with validation
    const auto& p1pos = player->getPosition();
    bool placed = false;
    for (int attempt = 0; attempt < 20; attempt++) {
      float angle = Tyra::Math::randomf(0.0F, 6.2831853F);
      float dist = Tyra::Math::randomf(60.0F, 120.0F);
      Vec4 tryPos(p1pos.x + cosf(angle) * dist, 50.0F, p1pos.z + sinf(angle) * dist);
      if (Player::isPositionValid(tryPos)) {
        player2->resetPositionTo(tryPos);
        placed = true;
        break;
      }
    }
    if (!placed) player2->resetPosition();
  }

  if (deathTimer == 0) player->update(terrain->heightmap);
  if (player2->isJoined && deathTimer2 == 0) player2->update(terrain->heightmap);

  // Separation cap (only when both alive+joined)

  // Shared camera: midpoint if both alive, else center on whichever is alive
  Vec4 focus;
  bool p1cam = (deathTimer == 0);
  bool p2cam = (player2->isJoined && deathTimer2 == 0);
  if (p1cam && p2cam) {
    focus = player->getPosition();
    focus.x = (player->getPosition().x + player2->getPosition().x) * 0.5F;
    focus.z = (player->getPosition().z + player2->getPosition().z) * 0.5F;
  } else if (p2cam) {
    focus = player2->getPosition();
  } else {
    focus = player->getPosition();
  }
  // Smooth camera: lerp cameraFocus toward desired focus (20% per frame)
  cameraFocus.x += (focus.x - cameraFocus.x) * 0.2F;
  cameraFocus.y += (focus.y - cameraFocus.y) * 0.2F;
  cameraFocus.z += (focus.z - cameraFocus.z) * 0.2F;
  float tHeight = terrain->heightmap.getHeightOffset(cameraFocus);

  // Dynamic zoom: scale out when players separate beyond 450 units.
  // Solo play stays at 1.0; 2P baseline is 1.10x; max clamp 2.5x.
  float targetZoom = 1.0F;
  if (player2->isJoined) {
    targetZoom = 1.10F;
    if (deathTimer == 0 && deathTimer2 == 0) {
      float ddx = player2->getPosition().x - player->getPosition().x;
      float ddz = player2->getPosition().z - player->getPosition().z;
      float dd = sqrtf(ddx*ddx + ddz*ddz);
      if (dd > 450.0F) {
        targetZoom = 1.10F * (dd / 450.0F);
        if (targetZoom > 2.5F) targetZoom = 2.5F;
      }
    }
  }
  cameraZoom += (targetZoom - cameraZoom) * 0.1F;
  camera.update(cameraFocus, tHeight, cameraZoom);

  auto shootAction = player->getShootAction();
  if (deathTimer > 0) shootAction.isShooting = false;
  auto shootAction2 = player2->getShootAction();
  if (!player2->isJoined || deathTimer2 > 0) shootAction2.isShooting = false;
  // Per-enemy nearest-player targeting
  bool p1alive = (deathTimer == 0 && player->getHealth() > 0);
  bool p2alive = (player2->isJoined && deathTimer2 == 0 && player2->getHealth() > 0);
  enemyManager->update(terrain->heightmap,
                       player->getPosition(), p1alive,
                       player2->getPosition(), p2alive,
                       shootAction);

  // Kill counter: poll recentlyKilled flag, update difficulty, override preSpawn range
  for (auto* enemy : enemyManager->getEnemies()) {
    if (enemy->recentlyKilled) {
      kills++;
      enemyManager->setDifficulty(kills);
      // Override the preSpawnTimer set by enemy to use current difficulty range
      enemy->preSpawnTimer = (int)Tyra::Math::randomf(
          (float)enemyManager->preSpawnMin,
          (float)enemyManager->preSpawnMax);
      enemy->recentlyKilled = false;

      // Roll drops
      Vec4 dropPos(enemy->deathPos.x, 50.0F, enemy->deathPos.z, 1.0F);
      float r = Tyra::Math::randomf(0.0F, 1.0F);
      // Health: 6%
      if (r < 0.06F) {
        pickupManager->spawn(PICKUP_HEALTH, dropPos);
      } else {
        // Weapon rolls — only drop if not owned by all active players
        bool p2J = player2->isJoined;
        auto needsWeapon = [&](WeaponType w) {
          bool p1Has = player->hasWeapon(w);
          bool p2Has = !p2J || player2->hasWeapon(w);
          return !(p1Has && p2Has);
        };
        if (r < 0.14F && needsWeapon(WEAPON_SMG)) {
          pickupManager->spawn(PICKUP_WEAPON_SMG, dropPos);
        } else if (r < 0.19F && needsWeapon(WEAPON_SHOTGUN)) {
          pickupManager->spawn(PICKUP_WEAPON_SHOTGUN, dropPos);
        } else if (r < 0.22F && needsWeapon(WEAPON_GRENADE_LAUNCHER)) {
          pickupManager->spawn(PICKUP_WEAPON_GRENADE_LAUNCHER, dropPos);
        } else if (r < 0.24F && needsWeapon(WEAPON_GUNCON)) {
          pickupManager->spawn(PICKUP_WEAPON_GUNCON, dropPos);
        }
      }
    }
  }

  // Ophanim death explosions (polled from enemies each frame)
  for (auto* enemy : enemyManager->getEnemies()) {
    if (enemy->pendingExplosion) {
      const float radius = 84.0F;
      const int dmg = 50;
      spawnExplosion(enemy->pendingExplosionPos, 0.75F);
      const auto& epos = enemy->pendingExplosionPos;
      // P1 damage
      float dx = player->getPosition().x - epos.x;
      float dz = player->getPosition().z - epos.z;
      if (dx*dx + dz*dz <= radius * radius && damageTimer == 0 && deathTimer == 0 && player->spawnInvulnTimer == 0) {
        player->takeDamage(dmg);
        engine->audio.adpcm.playPooled(playerHitAdpcm, 8, 3);
        damageTimer = 60;
      }
      // P2 damage
      if (player2->isJoined && deathTimer2 == 0 && damageTimer2 == 0) {
        float dx2 = player2->getPosition().x - epos.x;
        float dz2 = player2->getPosition().z - epos.z;
        if (dx2*dx2 + dz2*dz2 <= radius * radius && player2->spawnInvulnTimer == 0) {
          player2->takeDamage(dmg);
          engine->audio.adpcm.playPooled(playerHitAdpcm, 8, 3);
          damageTimer2 = 60;
        }
      }
      enemy->pendingExplosion = false;
    }
  }

  // Damage from nearby enemies — check both players
  if (damageTimer > 0) damageTimer--;
  if (damageTimer2 > 0) damageTimer2--;
  for (auto* enemy : enemyManager->getEnemies()) {
    if (enemy->spawnTimer > 0 || enemy->preSpawnTimer > 0) continue;
    auto* epos = enemy->mesh->getPosition();
    // P1 proximity
    float dx = epos->x - player->getPosition().x;
    float dz = epos->z - player->getPosition().z;
    if (dx*dx + dz*dz < 80.0F*80.0F && damageTimer == 0 && deathTimer == 0) {
      if (enemy->currentType == ENEMY_OPHANIM) {
        enemy->takeDamage(enemy->hp);
      } else if (player->spawnInvulnTimer == 0) {
        player->takeDamage(enemy->currentDamageAmount);
        engine->audio.adpcm.playPooled(playerHitAdpcm, 8, 3);
        damageTimer = 60;
      }
      continue;
    }
    // P2 proximity
    if (player2->isJoined && deathTimer2 == 0 && damageTimer2 == 0) {
      float dx2 = epos->x - player2->getPosition().x;
      float dz2 = epos->z - player2->getPosition().z;
      if (dx2*dx2 + dz2*dz2 < 80.0F*80.0F) {
        if (enemy->currentType == ENEMY_OPHANIM) {
          enemy->takeDamage(enemy->hp);
        } else if (player2->spawnInvulnTimer == 0) {
          player2->takeDamage(enemy->currentDamageAmount);
          engine->audio.adpcm.playPooled(playerHitAdpcm, 8, 3);
          damageTimer2 = 60;
        }
      }
    }
  }

  // Pickups: check P1 first, then P2
  int picked = pickupManager->update(player->getPosition(), player.get());
  Player* pickerPlayer = (picked >= 0) ? player.get() : nullptr;
  if (picked < 0 && player2->isJoined && deathTimer2 == 0) {
    picked = pickupManager->update(player2->getPosition(), player2.get());
    if (picked >= 0) pickerPlayer = player2.get();
  }
  if (picked == PICKUP_WEAPON_SMG) {
    pickerPlayer->pickupWeapon(WEAPON_SMG);
  } else if (picked == PICKUP_WEAPON_SHOTGUN) {
    pickerPlayer->pickupWeapon(WEAPON_SHOTGUN);
    engine->audio.adpcm.tryPlay(shotgunPickupAdpcm, 21);
  } else if (picked == PICKUP_WEAPON_GRENADE_LAUNCHER) {
    pickerPlayer->pickupWeapon(WEAPON_GRENADE_LAUNCHER);
  } else if (picked == PICKUP_WEAPON_GUNCON) {
    pickerPlayer->pickupWeapon(WEAPON_GUNCON);
  } else if (picked == PICKUP_HEALTH) {
    pickerPlayer->heal();
  }

  // Show 0 health/ammo while dead so HUD doesn't lie
  hud->setHealth(deathTimer > 0 ? 0 : player->getHealth(), player->getMaxHealth());
  if (player2->isJoined) {
    hud2->setHealth(deathTimer2 > 0 ? 0 : player2->getHealth(), player2->getMaxHealth());
  }

  // Restart if dead — independent P1/P2
  if (player->getHealth() <= 0 && deathTimer == 0) {
    bool runOver = !player2->isJoined || deathTimer2 > 0 || player2->getHealth() <= 0;
    if (runOver) engine->audio.adpcm.tryPlay(deathVoiceAdpcm, 22);
    deathTimer = 300;
    // If P2 was already dying, sync both timers so they respawn together
    if (player2->isJoined && deathTimer2 > 0) {
      int maxT = (deathTimer > deathTimer2) ? deathTimer : deathTimer2;
      deathTimer = maxT;
      deathTimer2 = maxT;
    }
    const auto& ppos = player->getPosition();
    playerBloodMesh->setPosition(Vec4(ppos.x, 2.0F, ppos.z));
    playerBloodMesh->rotation.identity();
    playerBloodMesh->rotation.rotateY(Tyra::Math::randomf(0.0F, 6.2831853F));
    playerBloodTimer = 150;
  }
  if (player2->isJoined && player2->getHealth() <= 0 && deathTimer2 == 0) {
    bool runOver = deathTimer > 0 || player->getHealth() <= 0;
    if (runOver) engine->audio.adpcm.tryPlay(deathVoiceAdpcm, 23);
    deathTimer2 = 300;
    // If P1 was already dying, sync both timers so they respawn together
    if (deathTimer > 0) {
      int maxT = (deathTimer > deathTimer2) ? deathTimer : deathTimer2;
      deathTimer = maxT;
      deathTimer2 = maxT;
    }
    // No second blood mesh — reuse playerBloodMesh if free, otherwise skip visual
    if (playerBloodTimer == 0) {
      const auto& p2pos = player2->getPosition();
      playerBloodMesh->setPosition(Vec4(p2pos.x, 2.0F, p2pos.z));
      playerBloodMesh->rotation.identity();
      playerBloodMesh->rotation.rotateY(Tyra::Math::randomf(0.0F, 6.2831853F));
      playerBloodTimer = 150;
    }
  }
  if (playerBloodTimer > 0) {
    playerBloodTimer--;
    if (playerBloodTimer == 0) {
      playerBloodMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
    }
  }
  if (deathTimer > 0) {
    deathTimer--;
    if (deathTimer == 0) {
      player->resetHealth();
      // Respawn near P2 if alive+joined, with up to 20 retries for valid spot
      bool placed = false;
      if (player2->isJoined && deathTimer2 == 0 && player2->getHealth() > 0) {
        const auto& p2pos = player2->getPosition();
        for (int attempt = 0; attempt < 20; attempt++) {
          float angle = Tyra::Math::randomf(0.0F, 6.2831853F);
          float dist = Tyra::Math::randomf(60.0F, 120.0F);
          Vec4 tryPos(p2pos.x + cosf(angle) * dist, 50.0F, p2pos.z + sinf(angle) * dist);
          if (Player::isPositionValid(tryPos)) {
            player->resetPositionTo(tryPos);
            placed = true;
            break;
          }
        }
      }
      if (!placed) player->resetPosition();
      player->resetWeapons();
      // Only reset pickups + difficulty if BOTH are dead (or P2 not joined)
      bool bothDead = (!player2->isJoined) || (deathTimer2 > 0 && player2->getHealth() <= 0);
      if (bothDead) {
        pickupManager->clearAll();
        kills = 0;
        enemyManager->setDifficulty(0);
        auto& es = const_cast<std::vector<Enemy*>&>(enemyManager->getEnemies());
        // Park ALL enemies: slots 2+3 permanently parked (await difficulty re-unlock),
        // slots 0+1 re-enter the pre-spawn cycle so they respawn away from player.
        for (int i = 0; i < (int)es.size(); i++) {
          es[i]->ophanimMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
          es[i]->demonLegsMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
          es[i]->demonUpperMesh->setPosition(Vec4(-99999.0F, -9999.0F, -99999.0F));
          es[i]->pentagramMesh->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
          es[i]->spawnTimer = 0;
          if (i >= 2) {
            es[i]->preSpawnTimer = 999999;  // locked until difficulty unlocks
          } else {
            es[i]->preSpawnTimer = (int)Tyra::Math::randomf(
                (float)enemyManager->preSpawnMin,
                (float)enemyManager->preSpawnMax);
          }
        }
      }
    }
  }
  if (deathTimer2 > 0) {
    deathTimer2--;
    if (deathTimer2 == 0) {
      player2->resetHealth();
      bool placed2 = false;
      if (deathTimer == 0 && player->getHealth() > 0) {
        const auto& p1pos = player->getPosition();
        for (int attempt = 0; attempt < 20; attempt++) {
          float angle = Tyra::Math::randomf(0.0F, 6.2831853F);
          float dist = Tyra::Math::randomf(60.0F, 120.0F);
          Vec4 tryPos(p1pos.x + cosf(angle) * dist, 50.0F, p1pos.z + sinf(angle) * dist);
          if (Player::isPositionValid(tryPos)) {
            player2->resetPositionTo(tryPos);
            placed2 = true;
            break;
          }
        }
      }
      if (!placed2) player2->resetPosition();
      player2->resetWeapons();
    }
  }

  // Shooting: apply N damage rays (with spread for shotgun), render
  // a single tracer + muzzle flash along the center ray.
  if (shootAction.isShooting) {
    const auto& pos = player->getPosition();
    const auto& aimDir = player->getAimDirection();
    float aimAngle = atan2(aimDir.x, aimDir.z);
    const auto& wdata = WEAPON_DATA[player->getCurrentWeapon()];

    // Projectile weapons spawn a projectile; others apply ray damage
    float pelletDistances[5] = {1e9f, 1e9f, 1e9f, 1e9f, 1e9f};
    if (wdata.isProjectile) {
      Vec4 origin(pos.x + aimDir.x * 70.0F, 60.0F, pos.z + aimDir.z * 70.0F);
      Vec4 dir(aimDir.x, 0.0F, aimDir.z, 0.0F);
      spawnProjectile(origin, dir);
    } else {
      bool anyPelletHitEnemy = false;
      float minObstacleDist = 1e9f;
      for (int p = 0; p < wdata.numPellets; p++) {
        float offset = 0.0F;
        if (wdata.numPellets > 1) {
          offset = (((float)p / (float)(wdata.numPellets - 1)) - 0.5F)
                   * wdata.spreadRadians;
        }
        float pelletAngle = aimAngle + offset;
        Vec4 pelletDir(sin(pelletAngle), 0.0F, cos(pelletAngle), 0.0F);
        Vec4 rayOrigin(pos.x, 0.0F, pos.z);
        Tyra::Ray pelletRay(rayOrigin, pelletDir);
        float dist = wdata.isPiercing
            ? enemyManager->applyShotPiercing(pelletRay, wdata.damage)
            : enemyManager->applyShot(pelletRay, wdata.damage);
        if (p < 5) pelletDistances[p] = dist;
        if (enemyManager->lastShotHitEnemy) anyPelletHitEnemy = true;
        else if (dist < minObstacleDist) minObstacleDist = dist;
        // Friendly fire: check P2 along ray (only if P2 alive AND TK on)
        if (tkState == 3 && player2->isJoined && deathTimer2 == 0) {
          const auto& p2p = player2->getPosition();
          // Project (p2p - rayOrigin) onto pelletDir
          float vx = p2p.x - rayOrigin.x, vz = p2p.z - rayOrigin.z;
          float proj = vx * pelletDir.x + vz * pelletDir.z;
          if (proj > 0.0F && proj < dist + 50.0F) {
            // Perpendicular distance from ray
            float px = rayOrigin.x + pelletDir.x * proj;
            float pz = rayOrigin.z + pelletDir.z * proj;
            float perp2 = (px - p2p.x) * (px - p2p.x) + (pz - p2p.z) * (pz - p2p.z);
            if (perp2 < 40.0F * 40.0F && player2->spawnInvulnTimer == 0) {
              player2->takeDamage(wdata.damage);
              engine->audio.adpcm.playPooled(playerHitAdpcm, 8, 3);
            }
          }
        }
      }
      // Ricochet: obstacle-only hit (not enemy, not missed into empty space).
      // Skip for piercing (guncon) per design.
      if (!wdata.isPiercing && !anyPelletHitEnemy && minObstacleDist < 2000.0F) {
        if (Tyra::Math::randomf(0.0F, 1.0F) < 0.3F) engine->audio.adpcm.playPooled(ricochetAdpcm, 19, 2);
      }
    }

    // Visual: one tracer per pellet
    const float barrelOffset = 70.0F;
    for (int t = 0; t < MAX_TRACERS; t++) {
      if (t < wdata.numPellets && !wdata.isProjectile) {
        // Calculate this pellet's angle and distance again
        float offset = 0.0F;
        if (wdata.numPellets > 1) {
          offset = (((float)t / (float)(wdata.numPellets - 1)) - 0.5F)
                   * wdata.spreadRadians;
        }
        float pelletAngle = aimAngle + offset;
        Vec4 pelletDir(sin(pelletAngle), 0.0F, cos(pelletAngle), 0.0F);
        Vec4 rayOrigin(pos.x, 0.0F, pos.z);
        // All pellets start from the barrel tip (center aim), only angle differs
        float sx = pos.x + aimDir.x * barrelOffset;
        float sz = pos.z + aimDir.z * barrelOffset;
        if (!wdata.isPiercing) {
          tracerMeshes[t]->setPosition(Vec4(sx, 60.0F, sz));
          tracerMeshes[t]->rotation.identity();
          tracerMeshes[t]->rotation.rotateY(pelletAngle - 1.5708F);
          tracerMeshes[t]->scale.identity();
          float tracerLength = pelletDistances[t] - barrelOffset;
          if (tracerLength > 800.0F || tracerLength < 0.0F) tracerLength = 800.0F;
          tracerMeshes[t]->scale.scaleX(tracerLength);
          tracerMeshes[t]->scale.scaleZ(80.0F);
        } else {
          tracerMeshes[t]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
        }
      } else {
        tracerMeshes[t]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
      }
    }
    // Piercing weapon: use blue tracer
    if (wdata.isPiercing) {
      float sx = pos.x + aimDir.x * barrelOffset;
      float sz = pos.z + aimDir.z * barrelOffset;
      blueTracerMesh->setPosition(Vec4(sx, 60.0F, sz));
      blueTracerMesh->rotation.identity();
      blueTracerMesh->rotation.rotateY(aimAngle - 1.5708F);
      blueTracerMesh->scale.identity();
      float tl = pelletDistances[0] - barrelOffset;
      if (tl > 800.0F || tl < 0.0F) tl = 800.0F;
      blueTracerMesh->scale.scaleX(tl);
      blueTracerMesh->scale.scaleZ(160.0F);  // wider than regular tracer
    } else {
      blueTracerMesh->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    }
    tracerTimer = 3;

    float startX = pos.x + aimDir.x * barrelOffset;
    float startZ = pos.z + aimDir.z * barrelOffset;
    int wt = player->getCurrentWeapon();
    if (muzzleFlashMeshes[wt] != nullptr) {
      muzzleFlashMeshes[wt]->setPosition(Vec4(startX, 60.0F, startZ));
      muzzleFlashMeshes[wt]->rotation.identity();
      muzzleFlashMeshes[wt]->rotation.rotateY(aimAngle - 1.5708F);
      muzzleFlashMeshes[wt]->scale.identity();
      muzzleFlashMeshes[wt]->scale.scaleX(120.0F);
      muzzleFlashMeshes[wt]->scale.scaleZ(80.0F);
    }
    // Rumble on P1 fire (skip for guncon — light gun doesn't rumble)
    if (player->getCurrentWeapon() != WEAPON_GUNCON) {
      rumbleSmallP1 = 1;
      rumbleBigP1 = 120;
      rumbleTimerP1 = 8;
    }
  }

  // P2 shooting with its own tracer/muzzle pool
  if (shootAction2.isShooting) {
    const auto& pos = player2->getPosition();
    const auto& aimDir = player2->getAimDirection();
    float aimAngle = atan2(aimDir.x, aimDir.z);
    const auto& wdata = WEAPON_DATA[player2->getCurrentWeapon()];
    float pelletDistances2[5] = {1e9f, 1e9f, 1e9f, 1e9f, 1e9f};
    if (wdata.isProjectile) {
      Vec4 origin(pos.x + aimDir.x * 70.0F, 60.0F, pos.z + aimDir.z * 70.0F);
      Vec4 dir(aimDir.x, 0.0F, aimDir.z, 0.0F);
      spawnProjectile(origin, dir);
    } else {
      bool anyPelletHitEnemy = false;
      float minObstacleDist = 1e9f;
      for (int p = 0; p < wdata.numPellets; p++) {
        float offset = 0.0F;
        if (wdata.numPellets > 1) {
          offset = (((float)p / (float)(wdata.numPellets - 1)) - 0.5F) * wdata.spreadRadians;
        }
        float pelletAngle = aimAngle + offset;
        Vec4 pelletDir(sin(pelletAngle), 0.0F, cos(pelletAngle), 0.0F);
        Vec4 rayOrigin(pos.x, 0.0F, pos.z);
        Tyra::Ray pelletRay(rayOrigin, pelletDir);
        float dist = wdata.isPiercing
            ? enemyManager->applyShotPiercing(pelletRay, wdata.damage)
            : enemyManager->applyShot(pelletRay, wdata.damage);
        if (p < 5) pelletDistances2[p] = dist;
        if (enemyManager->lastShotHitEnemy) anyPelletHitEnemy = true;
        else if (dist < minObstacleDist) minObstacleDist = dist;
        // Friendly fire: check P1 along ray
        if (deathTimer == 0) {
          const auto& p1p = player->getPosition();
          float vx = p1p.x - rayOrigin.x, vz = p1p.z - rayOrigin.z;
          float proj = vx * pelletDir.x + vz * pelletDir.z;
          if (proj > 0.0F && proj < dist + 50.0F) {
            float px = rayOrigin.x + pelletDir.x * proj;
            float pz = rayOrigin.z + pelletDir.z * proj;
            float perp2 = (px - p1p.x) * (px - p1p.x) + (pz - p1p.z) * (pz - p1p.z);
            if (tkState == 3 && perp2 < 40.0F * 40.0F && player->spawnInvulnTimer == 0) {
              player->takeDamage(wdata.damage);
              engine->audio.adpcm.playPooled(playerHitAdpcm, 8, 3);
            }
          }
        }
      }
      if (!wdata.isPiercing && !anyPelletHitEnemy && minObstacleDist < 2000.0F) {
        if (Tyra::Math::randomf(0.0F, 1.0F) < 0.3F) engine->audio.adpcm.playPooled(ricochetAdpcm, 19, 2);
      }
    }
    const float barrelOffset = 70.0F;
    for (int t = 0; t < MAX_TRACERS; t++) {
      if (t < wdata.numPellets && !wdata.isProjectile) {
        float offset = 0.0F;
        if (wdata.numPellets > 1) {
          offset = (((float)t / (float)(wdata.numPellets - 1)) - 0.5F) * wdata.spreadRadians;
        }
        float pelletAngle = aimAngle + offset;
        float sx = pos.x + aimDir.x * barrelOffset;
        float sz = pos.z + aimDir.z * barrelOffset;
        if (!wdata.isPiercing) {
          tracerMeshes2[t]->setPosition(Vec4(sx, 60.0F, sz));
          tracerMeshes2[t]->rotation.identity();
          tracerMeshes2[t]->rotation.rotateY(pelletAngle - 1.5708F);
          tracerMeshes2[t]->scale.identity();
          float tl = pelletDistances2[t] - barrelOffset;
          if (tl > 800.0F || tl < 0.0F) tl = 800.0F;
          tracerMeshes2[t]->scale.scaleX(tl);
          tracerMeshes2[t]->scale.scaleZ(80.0F);
        } else {
          tracerMeshes2[t]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
        }
      } else {
        tracerMeshes2[t]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
      }
    }
    if (wdata.isPiercing) {
      float sx = pos.x + aimDir.x * barrelOffset;
      float sz = pos.z + aimDir.z * barrelOffset;
      blueTracerMesh2->setPosition(Vec4(sx, 60.0F, sz));
      blueTracerMesh2->rotation.identity();
      blueTracerMesh2->rotation.rotateY(aimAngle - 1.5708F);
      blueTracerMesh2->scale.identity();
      float tl = pelletDistances2[0] - barrelOffset;
      if (tl > 800.0F || tl < 0.0F) tl = 800.0F;
      blueTracerMesh2->scale.scaleX(tl);
      blueTracerMesh2->scale.scaleZ(160.0F);
    } else {
      blueTracerMesh2->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    }
    tracerTimer2 = 3;
    float startX = pos.x + aimDir.x * barrelOffset;
    float startZ = pos.z + aimDir.z * barrelOffset;
    int wt = player2->getCurrentWeapon();
    if (muzzleFlashMeshes2[wt] != nullptr) {
      muzzleFlashMeshes2[wt]->setPosition(Vec4(startX, 60.0F, startZ));
      muzzleFlashMeshes2[wt]->rotation.identity();
      muzzleFlashMeshes2[wt]->rotation.rotateY(aimAngle - 1.5708F);
      muzzleFlashMeshes2[wt]->scale.identity();
      muzzleFlashMeshes2[wt]->scale.scaleX(120.0F);
      muzzleFlashMeshes2[wt]->scale.scaleZ(80.0F);
    }
    // Rumble on P2 fire (skip for guncon)
    if (player2->getCurrentWeapon() != WEAPON_GUNCON) {
      rumbleSmallP2 = 1;
      rumbleBigP2 = 120;
      rumbleTimerP2 = 8;
    }
  }
  if (tracerTimer2 > 0) {
    tracerTimer2--;
  } else {
    for (int t = 0; t < MAX_TRACERS; t++) {
      tracerMeshes2[t]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    }
    blueTracerMesh2->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    for (int i = 0; i < WEAPON_COUNT; i++) {
      if (muzzleFlashMeshes2[i]) muzzleFlashMeshes2[i]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    }
  }
  if (tracerTimer > 0) {
    tracerTimer--;
  } else {
    for (int t = 0; t < MAX_TRACERS; t++) {
      tracerMeshes[t]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    }
    blueTracerMesh->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    for (int i = 0; i < WEAPON_COUNT; i++) {
      if (muzzleFlashMeshes[i]) muzzleFlashMeshes[i]->setPosition(Vec4(0.0F, -5000.0F, 0.0F));
    }
  }

  hud->setWeapon(player->getCurrentWeapon());
  hud->setAmmo(deathTimer > 0 ? 0 : (player->isReloading() ? 0 : player->getAmmo()));
  if (player2->isJoined) {
    hud2->setWeapon(player2->getCurrentWeapon());
    hud2->setAmmo(deathTimer2 > 0 ? 0 : (player2->isReloading() ? 0 : player2->getAmmo()));
  }

  updateProjectiles();
  updateExplosions();
  updateSparks();

  engine->renderer.beginFrame(camera.getCameraInfo());
  {
    renderer.clear();
    {
      renderer.add(lavaPair);
      renderer.add(terrain->pair);
      renderer.add(enemyManager->getPentagramPairs());
      renderer.add(enemyManager->getUpperBodyPairs());
      renderer.add(enemyManager->getBloodPairs());
      renderer.add(pickupManager->getActivePairs());
      for (int t = 0; t < MAX_TRACERS; t++) renderer.add(tracerPairs[t]);
      renderer.add(blueTracerPair);
      for (int i = 0; i < WEAPON_COUNT; i++) {
        if (muzzleFlashPairs[i]) renderer.add(muzzleFlashPairs[i]);
      }
      if (player2->isJoined) {
        for (int t = 0; t < MAX_TRACERS; t++) renderer.add(tracerPairs2[t]);
        renderer.add(blueTracerPair2);
        for (int i = 0; i < WEAPON_COUNT; i++) {
          if (muzzleFlashPairs2[i]) renderer.add(muzzleFlashPairs2[i]);
        }
      }
      for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].alive) renderer.add(projectiles[i].pair);
      }
      for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (explosionLifetimes[i] > 0) renderer.add(explosionPairs[i]);
      }
      for (int i = 0; i < MAX_SPARKS; i++) {
        if (sparksLifetimes[i] > 0) renderer.add(sparksPairs[i]);
      }
      if (deathTimer == 0) {
        renderer.add(player->getCurrentModelPair());
        renderer.add(player->legsPair);
      }
      if (player2->isJoined && deathTimer2 == 0) {
        renderer.add(player2->getCurrentModelPair());
        renderer.add(player2->legsPair);
      }
      if (playerBloodTimer > 0) renderer.add(playerBloodPair);
      renderer.add(enemyManager->getPairs());
      renderer.add(hud->healthBarSprite.get());
      renderer.add(hud->healthSprite.get());
      renderer.add(hud->gunBgSprite.get());
      renderer.add(hud->gunIconSprite.get());
      for (auto& bullet : hud->bulletSprites) {
        renderer.add(bullet.get());
      }
      if (player2->isJoined) {
        renderer.add(hud2->healthBarSprite.get());
        renderer.add(hud2->healthSprite.get());
        renderer.add(hud2->gunBgSprite.get());
        renderer.add(hud2->gunIconSprite.get());
        for (auto& bullet : hud2->bulletSprites) {
          renderer.add(bullet.get());
        }
      }
    }
    renderer.render();
  }
  if (tkState != 0) engine->renderer.renderer2D.render(tkSprite);
  if (deathTimer > 0 && (!player2->isJoined || deathTimer2 > 0)) {
    engine->renderer.renderer2D.render(respawnSprite);
  }
  engine->renderer.endFrame();
}

void GameState::spawnProjectile(const Tyra::Vec4& origin, const Tyra::Vec4& dir) {
  // Find a dead slot
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (!projectiles[i].alive) {
      projectiles[i].alive = true;
      projectiles[i].position = origin;
      // Grenade travels at 15 units/frame in aim direction
      const float speed = 15.0F;
      projectiles[i].velocity = Vec4(dir.x * speed, 0.0F, dir.z * speed, 0.0F);
      projectiles[i].lifetime = 120;  // 2 seconds max
      projectiles[i].mesh->setPosition(origin);
      return;
    }
  }
}

void GameState::updateProjectiles() {
  const auto& wdata = WEAPON_DATA[WEAPON_GRENADE_LAUNCHER];

  for (int i = 0; i < MAX_PROJECTILES; i++) {
    auto& p = projectiles[i];
    if (!p.alive) continue;

    p.lifetime--;
    if (p.lifetime <= 0) {
      // Detonate where it stopped
      enemyManager->applyAoeDamage(p.position, wdata.aoeRadius, wdata.damage);
      // Both players take damage if within AoE (50%)
      {
        const auto& ppos = player->getPosition();
        float dx = ppos.x - p.position.x;
        float dz = ppos.z - p.position.z;
        if (dx*dx + dz*dz <= wdata.aoeRadius * wdata.aoeRadius) {
          player->takeDamage(wdata.damage / 2);
        }
        if (player2->isJoined && deathTimer2 == 0) {
          const auto& p2pos = player2->getPosition();
          float dx2 = p2pos.x - p.position.x;
          float dz2 = p2pos.z - p.position.z;
          if (dx2*dx2 + dz2*dz2 <= wdata.aoeRadius * wdata.aoeRadius) {
            player2->takeDamage(wdata.damage / 2);
          }
        }
      }
      spawnExplosion(p.position);
      // Play player hit AFTER explosion to avoid channel step-on
      {
        const auto& ppos2 = player->getPosition();
        float dx2 = ppos2.x - p.position.x;
        float dz2 = ppos2.z - p.position.z;
        if (dx2*dx2 + dz2*dz2 <= wdata.aoeRadius * wdata.aoeRadius && player->spawnInvulnTimer == 0) {
          engine->audio.adpcm.playPooled(playerHitAdpcm, 8, 3);
        }
      }
      p.hide();
      continue;
    }

    // Move
    p.position.x += p.velocity.x;
    p.position.z += p.velocity.z;
    p.mesh->setPosition(p.position);

    // Check obstacle collision (point-in-box)
    bool hitObstacle = false;
    for (int j = 0; j < NUM_OBSTACLES; j++) {
      const auto& box = OBSTACLES[j];
      if (!box.blocksProjectiles) continue;
      if (p.position.x >= box.minX && p.position.x <= box.maxX &&
          p.position.z >= box.minZ && p.position.z <= box.maxZ) {
        hitObstacle = true;
        break;
      }
    }

    // Check enemy collision (proximity to enemy center)
    bool hitEnemy = false;
    for (auto* epair : enemyManager->getPairs()) {
      auto* epos = epair->mesh->getPosition();
      float dx = epos->x - p.position.x;
      float dz = epos->z - p.position.z;
      if (dx*dx + dz*dz < 60.0F * 60.0F) {
        hitEnemy = true;
        break;
      }
    }

    // Check player collision (either player)
    bool hitPlayer = false;
    if (deathTimer == 0) {
      const auto& ppos = player->getPosition();
      float dx = ppos.x - p.position.x;
      float dz = ppos.z - p.position.z;
      if (dx*dx + dz*dz < 60.0F * 60.0F) hitPlayer = true;
    }
    if (!hitPlayer && player2->isJoined && deathTimer2 == 0) {
      const auto& p2pos = player2->getPosition();
      float dx = p2pos.x - p.position.x;
      float dz = p2pos.z - p.position.z;
      if (dx*dx + dz*dz < 60.0F * 60.0F) hitPlayer = true;
    }

    if (hitObstacle || hitEnemy || hitPlayer) {
      enemyManager->applyAoeDamage(p.position, wdata.aoeRadius, wdata.damage);
      // Both players take damage if within AoE (50%)
      {
        const auto& ppos = player->getPosition();
        float dx = ppos.x - p.position.x;
        float dz = ppos.z - p.position.z;
        if (dx*dx + dz*dz <= wdata.aoeRadius * wdata.aoeRadius) {
          player->takeDamage(wdata.damage / 2);
        }
        if (player2->isJoined && deathTimer2 == 0) {
          const auto& p2pos = player2->getPosition();
          float dx2 = p2pos.x - p.position.x;
          float dz2 = p2pos.z - p.position.z;
          if (dx2*dx2 + dz2*dz2 <= wdata.aoeRadius * wdata.aoeRadius) {
            player2->takeDamage(wdata.damage / 2);
          }
        }
      }
      spawnExplosion(p.position);
      // Play player hit AFTER explosion to avoid channel step-on
      {
        const auto& ppos2 = player->getPosition();
        float dx2 = ppos2.x - p.position.x;
        float dz2 = ppos2.z - p.position.z;
        if (dx2*dx2 + dz2*dz2 <= wdata.aoeRadius * wdata.aoeRadius && player->spawnInvulnTimer == 0) {
          engine->audio.adpcm.playPooled(playerHitAdpcm, 8, 3);
        }
      }
      p.hide();
    }
  }
}

void GameState::spawnExplosion(const Tyra::Vec4& position, float scaleMul) {
  for (int i = 0; i < MAX_EXPLOSIONS; i++) {
    if (explosionLifetimes[i] <= 0) {
      explosionLifetimes[i] = 20;
      explosionPositions[i] = position;
      explosionScaleMuls[i] = scaleMul;
      explosionRotations[i] = Tyra::Math::randomf(0.0F, 6.2831853F);
      explosionMeshes[i]->setPosition(position);
      explosionMeshes[i]->rotation.identity();
      explosionMeshes[i]->rotation.rotateY(explosionRotations[i]);
      explosionMeshes[i]->scale.identity();
      explosionMeshes[i]->scale.scale(30.0F * scaleMul);
      engine->audio.adpcm.tryPlay(glImpactAdpcm, 21);
      rumbleBigP1 = 255; rumbleSmallP1 = 0; rumbleTimerP1 = 15;
      if (player2 && player2->isJoined) {
        rumbleBigP2 = 255; rumbleSmallP2 = 0; rumbleTimerP2 = 15;
      }
      return;
    }
  }
}

void GameState::updateExplosions() {
  for (int i = 0; i < MAX_EXPLOSIONS; i++) {
    if (explosionLifetimes[i] <= 0) continue;

    int elapsed = 20 - explosionLifetimes[i];
    float s = (30.0F + (float)elapsed * 6.0F) * explosionScaleMuls[i];
    explosionMeshes[i]->setPosition(explosionPositions[i]);
    explosionMeshes[i]->rotation.identity();
    explosionMeshes[i]->rotation.rotateY(explosionRotations[i]);
    explosionMeshes[i]->scale.identity();
    explosionMeshes[i]->scale.scale(s);

    explosionLifetimes[i]--;
    if (explosionLifetimes[i] <= 0) {
      explosionMeshes[i]->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
    }
  }
}

void GameState::spawnSparks(const Tyra::Vec4& position) {
  for (int i = 0; i < MAX_SPARKS; i++) {
    if (sparksLifetimes[i] <= 0) {
      sparksLifetimes[i] = 20;
      sparksPositions[i] = position;
      sparksRotations[i] = Tyra::Math::randomf(0.0F, 6.2831853F);
      sparksMeshes[i]->setPosition(position);
      sparksMeshes[i]->rotation.identity();
      sparksMeshes[i]->rotation.rotateY(sparksRotations[i]);
      sparksMeshes[i]->scale.identity();
      sparksMeshes[i]->scale.scale(37.5F);
      engine->audio.adpcm.tryPlay(ophanimDeathAdpcm, 16);
      return;
    }
  }
}

void GameState::updateSparks() {
  for (int i = 0; i < MAX_SPARKS; i++) {
    if (sparksLifetimes[i] <= 0) continue;
    int elapsed = 20 - sparksLifetimes[i];
    float s = 37.5F + (float)elapsed * 7.5F;
    sparksMeshes[i]->setPosition(sparksPositions[i]);
    sparksMeshes[i]->rotation.identity();
    sparksMeshes[i]->rotation.rotateY(sparksRotations[i]);
    sparksMeshes[i]->scale.identity();
    sparksMeshes[i]->scale.scale(s);
    sparksLifetimes[i]--;
    if (sparksLifetimes[i] <= 0) {
      sparksMeshes[i]->setPosition(Vec4(0.0F, -9999.0F, 0.0F));
    }
  }
}

}  // namespace Demo
