#include "states/intro/states/intro_press_key_state.hpp"
#include "states/intro/intro_state.hpp"
#include <string>

using Tyra::FileUtils;
using Tyra::PngLoader;
using Tyra::SpriteMode;
using Tyra::Threading;

namespace Demo {

IntroPressKeyState::IntroPressKeyState(Engine* t_engine)
    : State(t_engine), mapPosition(0.0F, 0.0F) {
  state = STATE_PRESS_KEY;
  _wantFinish = false;
  mapDirection = 0;
  mapFadeIn = true;
  fillerFadeIn = false;
  showPressKey = false;
  pressKeyAlphaToggle = false;
  pressKeyAlphaDelayer = 0;
  fillersOffset = -512.0F;
  currentFrame = 0;
  frameDelay = 0;
}

IntroPressKeyState::~IntroPressKeyState() {}

void IntroPressKeyState::onStart() {
  IntroState::skipToPressKey = false;
  TYRA_LOG("Intro - presskey. RAM: ", engine->info.getAvailableRAM(), "MB");

  engine->audio.song.load(FileUtils::fromCwd("intro/intro.wav"));
  engine->audio.song.inLoop = true;
  engine->audio.song.play();
  engine->audio.song.setVolume(100);

  const auto& settings = engine->renderer.core.getSettings();

  bgSprite = new Sprite;
  bgSprite->mode = SpriteMode::MODE_STRETCH;
  // Smaller than full screen to make room for press-x
  const float bgW = 400.0F;
  const float bgH = 350.0F;
  bgSprite->size.set(bgW, bgH);
  bgSprite->position.set((settings.getWidth() - bgW) / 2.0F, 30.0F);

  for (u8 i = 0; i < totalFrames; i++) {
    int n = i + 1;
    std::string path = "intro/anim/frame_" + std::string(n < 10 ? "0" : "") + std::to_string(n) + ".png";
    animTextures[i] = engine->renderer.core.texture.repository.add(
        FileUtils::fromCwd(path));
  }
  animTextures[0]->addLink(bgSprite->id);

  pressKeySprite = new Sprite;
  pressKeySprite->size.set(256.0F, 64.0F);
  pressKeySprite->scale = 0.5F;
  pressKeySprite->position.set(192.0F, 398.0F);

  pressKeyTexture = engine->renderer.core.texture.repository.add(
      FileUtils::fromCwd("intro/press-key.png"));
  pressKeyTexture->addLink(pressKeySprite->id);

  gradientSprite = new Sprite;
  gradientSprite->mode = SpriteMode::MODE_STRETCH;
  gradientSprite->size.set((float)settings.getWidth(), (float)settings.getHeight() / 2.0F);
  gradientSprite->position.set(0.0F, (float)settings.getHeight() / 2.0F);
  gradientTexture = engine->renderer.core.texture.repository.add(
      FileUtils::fromCwd("intro/gradient.png"));
  gradientTexture->addLink(gradientSprite->id);

  initialized = true;
  showPressKey = true;
}

IntroStateType IntroPressKeyState::onFinish() {
  if (!initialized) return STATE_BACKSTORY;

  for (u8 i = 0; i < totalFrames; i++) {
    engine->renderer.core.texture.repository.free(animTextures[i]->id);
  }
  delete bgSprite;

  engine->renderer.core.texture.repository.free(pressKeyTexture->id);
  delete pressKeySprite;
  engine->renderer.core.texture.repository.free(gradientTexture->id);
  delete gradientSprite;

  initialized = false;
  return STATE_BACKSTORY;
}

void IntroPressKeyState::update() {
  engine->renderer.beginFrame();

  if (engine->pad.getClicked().Cross) {
    _wantFinish = true;
  }

  frameDelay++;
  if (frameDelay > 6) {
    frameDelay = 0;
    animTextures[currentFrame]->removeLinkById(bgSprite->id);
    currentFrame = (currentFrame + 1) % totalFrames;
    animTextures[currentFrame]->addLink(bgSprite->id);
  }

  engine->renderer.renderer2D.render(bgSprite);
  engine->renderer.renderer2D.render(gradientSprite);

  Threading::switchThread();

  if (showPressKey) {
    if (pressKeyAlphaDelayer++ > 50) {
      pressKeyAlphaToggle = !pressKeyAlphaToggle;
      pressKeyAlphaDelayer = 0;
    }

    if (pressKeyAlphaToggle) {
      engine->renderer.renderer2D.render(pressKeySprite);
    }
  }

  engine->renderer.endFrame();
}

void IntroPressKeyState::renderFiller() {}

void IntroPressKeyState::updateMap() {}

}  // namespace Demo
