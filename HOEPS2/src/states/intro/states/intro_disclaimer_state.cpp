#include "states/intro/states/intro_disclaimer_state.hpp"
#include "states/intro/intro_state.hpp"

using Tyra::FileUtils;
using Tyra::Threading;
using Tyra::SpriteMode;

namespace Demo {

IntroDisclaimerState::IntroDisclaimerState(Engine* t_engine) : State(t_engine) {
  state = STATE_DISCLAIMER;
  _wantFinish = false;
  initialized = false;
  pressKeyAlphaToggle = false;
  pressKeyAlphaDelayer = 0;
  bgSprite = nullptr;
  bgTexture = nullptr;
  textSprite = nullptr;
  textTexture = nullptr;
  pressKeySprite = nullptr;
  pressKeyTexture = nullptr;
}

IntroDisclaimerState::~IntroDisclaimerState() {}

void IntroDisclaimerState::onStart() {
  TYRA_LOG("Intro - disclaimer. RAM: ", engine->info.getAvailableRAM(), "MB");

  const auto& settings = engine->renderer.core.getSettings();
  const float screenW = (float)settings.getWidth();
  const float bgSize = 336.0F;
  bgSprite = new Sprite;
  bgSprite->mode = SpriteMode::MODE_STRETCH;
  bgSprite->size.set(bgSize, bgSize);
  bgSprite->position.set((screenW - bgSize) / 2.0F, 0.0F);
  bgTexture = engine->renderer.core.texture.repository.add(
      FileUtils::fromCwd("intro/filler.png"));
  bgTexture->addLink(bgSprite->id);

  textSprite = new Sprite;
  textSprite->mode = SpriteMode::MODE_STRETCH;
  const float textSize = 270.0F;
  textSprite->size.set(textSize, textSize);
  textSprite->position.set((screenW - textSize) / 2.0F, (bgSize - textSize) / 2.0F);
  textTexture = engine->renderer.core.texture.repository.add(
      FileUtils::fromCwd("intro/disclaimer.png"));
  textTexture->addLink(textSprite->id);

  pressKeySprite = new Sprite;
  pressKeySprite->size.set(256.0F, 64.0F);
  pressKeySprite->scale = 0.5F;
  pressKeySprite->position.set(192.0F, 410.0F);
  pressKeyTexture = engine->renderer.core.texture.repository.add(
      FileUtils::fromCwd("intro/press-key.png"));
  pressKeyTexture->addLink(pressKeySprite->id);

  initialized = true;
}

IntroStateType IntroDisclaimerState::onFinish() {
  if (!initialized) return STATE_PS2DEV;

  engine->renderer.core.texture.repository.free(bgTexture->id);
  engine->renderer.core.texture.repository.free(textTexture->id);
  engine->renderer.core.texture.repository.free(pressKeyTexture->id);
  delete bgSprite;
  delete textSprite;
  delete pressKeySprite;

  initialized = false;
  return STATE_PS2DEV;
}

void IntroDisclaimerState::update() {
  if (IntroState::skipToPressKey) { _wantFinish = true; return; }
  engine->renderer.beginFrame();

  if (engine->pad.getClicked().Cross) {
    _wantFinish = true;
  }

  engine->renderer.renderer2D.render(bgSprite);
  engine->renderer.renderer2D.render(textSprite);

  Threading::switchThread();
  if (pressKeyAlphaDelayer++ > 50) {
    pressKeyAlphaToggle = !pressKeyAlphaToggle;
    pressKeyAlphaDelayer = 0;
  }
  if (pressKeyAlphaToggle) {
    engine->renderer.renderer2D.render(pressKeySprite);
  }

  engine->renderer.endFrame();
}

}  // namespace Demo
