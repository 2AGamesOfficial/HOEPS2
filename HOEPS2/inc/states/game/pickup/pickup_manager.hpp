#pragma once
#include <tyra>
#include <vector>
#include "./pickup.hpp"
#include "states/game/player/player.hpp"
#include "./pickup_type.hpp"

using Tyra::Engine;
using Tyra::Vec4;

namespace Demo {

class PickupManager {
 public:
  PickupManager(Engine* engine);
  ~PickupManager();

  // Spawn a pickup at a world position. Call during GameState::onStart().
  void spawn(PickupType type, const Vec4& position);

  // Call every frame: spins, bobs, checks player proximity
  // Returns the type of a picked-up item, or -1 if none picked up this frame.
  int update(const Vec4& playerPosition, const Player* player);

  // Reactivate all pickups (called on player death/reset)
  void init();
  void clearAll();

  // Renderer access
  std::vector<RendererStaticPair*> getActivePairs() const;

 private:
  Engine* engine;
  audsrv_adpcm_t* pickupSound;
  std::vector<Pickup*> pickups;  // all spawned (active + inactive)
  static constexpr float PICKUP_RADIUS = 60.0F;
};

}  // namespace Demo
