#pragma once

#include "state/state.hpp"
#include "../intro_state_type.hpp"
#include <tyra>

using Tyra::Sprite;
using Tyra::Texture;

namespace Demo {

class IntroDisclaimerState : public State<IntroStateType> {
 public:
  IntroDisclaimerState(Engine* t_engine);
  ~IntroDisclaimerState();

  const IntroStateType& getState() const { return state; }
  const bool& wantFinish() const { return _wantFinish; };

  void onStart();
  IntroStateType onFinish();
  void update();

 private:
  IntroStateType state;
  bool _wantFinish;
  bool initialized;

  bool pressKeyAlphaToggle;
  u8 pressKeyAlphaDelayer;

  Sprite* bgSprite;
  Texture* bgTexture;

  Sprite* textSprite;
  Texture* textTexture;

  Sprite* pressKeySprite;
  Texture* pressKeyTexture;
};

}  // namespace Demo
