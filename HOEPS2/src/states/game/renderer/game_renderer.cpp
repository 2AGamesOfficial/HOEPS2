/*
# _____        ____   ___
#   |     \/   ____| |___|
#   |     |   |   \  |   |
#-----------------------------------------------------------------------
# Copyright 2022, tyra - https://github.com/h4570/tyra
# Licensed under Apache License 2.0
# Sandro Sobczyński <sandro.sobczynski@gmail.com>
*/

#include "states/game/renderer/game_renderer.hpp"

using Tyra::Threading;
using Tyra::Vec4;

namespace Demo {

GameRenderer::GameRenderer(Renderer* t_renderer) {
  renderer = t_renderer;

  stpip.setRenderer(&renderer->core);
  dypip.setRenderer(&renderer->core);
}

GameRenderer::~GameRenderer() {}

void GameRenderer::add(std::vector<RendererStaticPair*> t_staticPairs) {
  staticPairs.insert(staticPairs.end(), t_staticPairs.begin(),
                     t_staticPairs.end());
}

void GameRenderer::add(std::vector<RendererDynamicPair*> t_dynamicPairs) {
  dynamicPairs.insert(dynamicPairs.end(), t_dynamicPairs.begin(),
                      t_dynamicPairs.end());
}

void GameRenderer::add(Sprite* sprite) { sprites.push_back(sprite); }

void GameRenderer::add(const CoreBBox& bbox) { bboxes.push_back(bbox); }
void GameRenderer::addLine(const Vec4& from, const Vec4& to) {
  lines.push_back({from, to});
}

void GameRenderer::add(RendererStaticPair* staticPair) {
  staticPairs.push_back(staticPair);
}

void GameRenderer::add(RendererDynamicPair* dynamicPair) {
  dynamicPairs.push_back(dynamicPair);
}

void GameRenderer::clear() {
  staticPairs.clear();
  dynamicPairs.clear();
  sprites.clear();
  bboxes.clear();
  lines.clear();
}

void GameRenderer::render() {
  if (staticPairs.size()) {
    renderer->renderer3D.usePipeline(&stpip);
    for (auto& pair : staticPairs) {
      stpip.render(pair->mesh, pair->options);
    }
  }

  Threading::switchThread();

  if (dynamicPairs.size()) {
    renderer->renderer3D.usePipeline(&dypip);
    for (auto& pair : dynamicPairs) {
      dypip.render(pair->mesh, pair->options);
    }
  }

  Threading::switchThread();

  for (auto& bbox : bboxes) {
    renderer->renderer3D.utility.drawBBox(bbox);
  }
  for (auto& line : lines) {
    renderer->renderer3D.utility.drawLine(line.from, line.to, Tyra::Color(255.0F, 255.0F, 255.0F));
  }

  for (auto& sprite : sprites) {
    renderer->renderer2D.render(sprite);
  }
}

}  // namespace Demo
