#include "states/intro/states/intro_ps2dev_state.hpp"
#include "states/intro/intro_state.hpp"

using Tyra::FileUtils;
using Tyra::PngLoader;
using Tyra::Threading;

namespace Demo {

IntroPs2DevState::IntroPs2DevState(Engine* t_engine) : State(t_engine) {
  state = STATE_PS2DEV;
  _wantFinish = false;
  initialized = false;
  initialDelayElapsed = false;
  frameSkipper = 0;
  fadeinActivated = true;
  fadeoutActivated = false;
}

IntroPs2DevState::~IntroPs2DevState() {}

void IntroPs2DevState::onStart() {
  TYRA_LOG("Intro - studio. RAM: ", engine->info.getAvailableRAM(), "MB");

  const auto& settings = engine->renderer.core.getSettings();

  initialDelayTimer.prime();

  const char* logoPaths[3] = {"intro/blender.png", "intro/gimp.png", "intro/audacity.png"};
  const float logoSize = 128.0F;
  const float totalWidth = logoSize * 3.0F + 32.0F;
  const float startX = settings.getWidth() / 2.0F - totalWidth / 2.0F;
  const float y = settings.getHeight() / 2.0F - logoSize / 2.0F;
  for (int i = 0; i < 3; i++) {
    sprite[i] = new Sprite;
    sprite[i]->size.set(logoSize, logoSize);
    sprite[i]->position.set(startX + i * (logoSize + 16.0F), y);
    sprite[i]->color.a = 0;
    Threading::switchThread();
    texture[i] = engine->renderer.core.texture.repository.add(
        FileUtils::fromCwd(logoPaths[i]));
    texture[i]->addLink(sprite[i]->id);
  }

  initialized = true;
}

IntroStateType IntroPs2DevState::onFinish() {
  if (!initialized) return STATE_TYRA;

  for (int i = 0; i < 3; i++) delete sprite[i];
  for (int i = 0; i < 3; i++) engine->renderer.core.texture.repository.free(texture[i]->id);

  initialized = false;
  return STATE_TYRA;
}

void IntroPs2DevState::update() {
  if (IntroState::skipToPressKey) { _wantFinish = true; return; }
  engine->renderer.beginFrame();

  if (initialDelayTimer.getTimeDelta() >= 60000) initialDelayElapsed = true;

  if (engine->pad.getClicked().Cross) {
    _wantFinish = true;
  }

  frameSkipper++;
  if (frameSkipper > 3) {
    frameSkipper = 0;
  }

  if (fadeinActivated && initialDelayElapsed && frameSkipper == 0) {
    if (sprite[0]->color.a < 128.0F)
      for (int i = 0; i < 3; i++) sprite[i]->color.a += 2.0F;
    else {
      fadeoutActivated = true;
      fadeinActivated = false;
    }
  }

  Threading::switchThread();

  if (fadeoutActivated && frameSkipper == 0) {
    if (sprite[0]->color.a > 0)
      for (int i = 0; i < 3; i++) sprite[i]->color.a -= 4;
    else
      _wantFinish = true;
  }

  for (int i = 0; i < 3; i++) engine->renderer.renderer2D.render(sprite[i]);
  engine->renderer.endFrame();
}

}  // namespace Demo
