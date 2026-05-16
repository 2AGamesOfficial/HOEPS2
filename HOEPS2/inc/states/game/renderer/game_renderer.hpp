/*
# _____        ____   ___
#   |     \/   ____| |___|
#   |     |   |   \  |   |
#-----------------------------------------------------------------------
# Copyright 2022, tyra - https://github.com/h4570/tyra
# Licensed under Apache License 2.0
# Sandro Sobczyński <sandro.sobczynski@gmail.com>
*/

#pragma once

#include <tyra>
#include "./renderer_static_pair.hpp"
#include "./renderer_dynamic_pair.hpp"

using Tyra::CoreBBox;
using Tyra::DynamicPipeline;
using Tyra::Renderer;
using Tyra::Sprite;
using Tyra::StaticPipeline;
using Tyra::Vec4;

struct TracerLine { Vec4 from; Vec4 to; };

namespace Demo {

class GameRenderer {
 public:
  GameRenderer(Renderer* renderer);
  ~GameRenderer();

  void add(std::vector<RendererStaticPair*> staticPairs);
  void add(std::vector<RendererDynamicPair*> dynamicPairs);
  void add(RendererStaticPair* staticPair);
  void add(RendererDynamicPair* dynamicPair);
  void add(Sprite* sprite);
  void add(const CoreBBox& bbox);
  void addLine(const Vec4& from, const Vec4& to);

  Tyra::Info* dbgInfo = nullptr;
  void clear();
  void render();

 private:
  Renderer* renderer;

  StaticPipeline stpip;
  DynamicPipeline dypip;

  std::vector<RendererStaticPair*> staticPairs;
  std::vector<RendererDynamicPair*> dynamicPairs;
  std::vector<Sprite*> sprites;
  std::vector<CoreBBox> bboxes;
  std::vector<TracerLine> lines;
};

}  // namespace Demo
