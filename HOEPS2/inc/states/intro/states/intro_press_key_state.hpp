#pragma once

#include "state/state.hpp"
#include "../intro_state_type.hpp"
#include <tyra>

using Tyra::Sprite;
using Tyra::Texture;
using Tyra::Timer;
using Tyra::Vec2;

namespace Demo {

class IntroPressKeyState : public State<IntroStateType> {
 public:
  IntroPressKeyState(Engine* t_engine);
  ~IntroPressKeyState();

  const IntroStateType& getState() const { return state; }

  const bool& wantFinish() const { return _wantFinish; }

  void onStart();
  IntroStateType onFinish();
  void update();

 private:
  void renderFiller();
  void updateMap();

  IntroStateType state;
  bool _wantFinish;
  bool initialized;
  bool mapFadeIn;
  bool fillerFadeIn;
  bool showPressKey;
  bool pressKeyAlphaToggle;
  u8 pressKeyAlphaDelayer;

  float fillersOffset;
  constexpr static float textureWidthHeight = 512.0F;
  const static u8 mapRows = 3;
  const static u8 mapCols = 3;
  const static u8 totalFrames = 24;

  Vec2 mapPosition;
  u8 mapDirection;
  u8 currentFrame;
  u8 frameDelay;

  Sprite* bgSprite;
  Texture* animTextures[24];

  Texture* pressKeyTexture;
  Sprite* pressKeySprite;
  Texture* gradientTexture;
  Sprite* gradientSprite;

  Texture* mapTextures[mapRows][mapCols];
  Sprite* mapSprites[mapRows][mapCols];
  audsrv_adpcm_t* introMusicAdpcm = nullptr;
};

}  // namespace Demo
