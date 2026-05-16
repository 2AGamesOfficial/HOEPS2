/*
# _____        ____   ___
#   |     \/   ____| |___|
#   |     |   |   \  |   |
#-----------------------------------------------------------------------
# Copyright 2022, tyra - https://github.com/h4570/tyra
# Licensed under Apache License 2.0
# Sandro Sobczyński <sandro.sobczynski@gmail.com>
*/

#include "states/intro/intro_state.hpp"

#include "states/intro/states/intro_disclaimer_state.hpp"
#include "states/intro/states/intro_ps2dev_state.hpp"
#include "states/intro/states/intro_tyra_state.hpp"
#include "states/intro/states/intro_press_key_state.hpp"
#include "states/intro/states/intro_backstory_state.hpp"

using Tyra::FileUtils;

namespace Demo {

bool IntroState::skipToPressKey = false;

IntroState::IntroState(Engine* t_engine)
    : State(t_engine),
      stateManager(skipToPressKey ? STATE_PRESS_KEY : STATE_DISCLAIMER, STATE_INTRO_END) {
  state = STATE_INTRO;
  _wantFinish = false;
  initialized = false;
}

IntroState::~IntroState() {}

void IntroState::onStart() {
  TYRA_LOG("Intro. RAM: ", engine->info.getAvailableRAM(), "MB");
  stateManager.setCurrentState(skipToPressKey ? STATE_PRESS_KEY : STATE_DISCLAIMER);

  stateManager.add(new IntroDisclaimerState(engine));
  stateManager.add(new IntroPs2DevState(engine));
  stateManager.add(new IntroTyraState(engine));
  stateManager.add(new IntroPressKeyState(engine));
  stateManager.add(new IntroBackstoryState(engine));

  initialized = true;
}

GlobalStateType IntroState::onFinish() {
  _wantFinish = false;
  if (!initialized) return STATE_LOADING;

  stateManager.freeAll();

  initialized = false;
  return STATE_LOADING;
}

void IntroState::update() {
  stateManager.update();

  if (stateManager.finished()) {
    _wantFinish = true;
  }
}

}  // namespace Demo
