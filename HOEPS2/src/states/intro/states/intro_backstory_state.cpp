#include "states/intro/states/intro_backstory_state.hpp"

using Tyra::FileUtils;
using Tyra::Threading;
using Tyra::SpriteMode;

namespace Demo {

IntroBackstoryState::IntroBackstoryState(Engine* t_engine) : State(t_engine) {
  state = STATE_BACKSTORY;
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

IntroBackstoryState::~IntroBackstoryState() {}

void IntroBackstoryState::onStart() {
  TYRA_LOG("Intro - backstory. RAM: ", engine->info.getAvailableRAM(), "MB");

  // Black fullscreen background (reuse filler.png stretched)
  bgSprite = new Sprite;
  bgSprite->mode = SpriteMode::MODE_STRETCH;
  bgSprite->size.set(512.0F, 448.0F);
  bgSprite->position.set(0.0F, 0.0F);
  bgTexture = engine->renderer.core.texture.repository.add(
      FileUtils::fromCwd("intro/filler.png"));
  bgTexture->addLink(bgSprite->id);

  // Text overlay (backstory.png, 512x448 with transparency)
  textSprite = new Sprite;
  textSprite->mode = SpriteMode::MODE_STRETCH;
  textSprite->size.set(460.0F, 403.0F);
  textSprite->position.set(26.0F, 22.0F);  // centered
  textTexture = engine->renderer.core.texture.repository.add(
      FileUtils::fromCwd("intro/backstory.png"));
  textTexture->addLink(textSprite->id);

  // Press-X prompt (reuse existing press-key.png)
  pressKeySprite = new Sprite;
  pressKeySprite->size.set(256.0F, 64.0F);
  pressKeySprite->scale = 0.5F;
  pressKeySprite->position.set(192.0F, 410.0F);
  pressKeyTexture = engine->renderer.core.texture.repository.add(
      FileUtils::fromCwd("intro/press-key.png"));
  pressKeyTexture->addLink(pressKeySprite->id);

  initialized = true;
}

IntroStateType IntroBackstoryState::onFinish() {
  if (!initialized) return STATE_INTRO_END;

  engine->renderer.core.texture.repository.free(bgTexture->id);
  engine->renderer.core.texture.repository.free(textTexture->id);
  engine->renderer.core.texture.repository.free(pressKeyTexture->id);
  delete bgSprite;
  delete textSprite;
  delete pressKeySprite;

  initialized = false;
  return STATE_INTRO_END;
}

void IntroBackstoryState::update() {
  engine->renderer.beginFrame();

  if (engine->pad.getClicked().Cross) {
    _wantFinish = true;
  }

  engine->renderer.renderer2D.render(bgSprite);
  engine->renderer.renderer2D.render(textSprite);

  Threading::switchThread();

  // Blinking press-X
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
