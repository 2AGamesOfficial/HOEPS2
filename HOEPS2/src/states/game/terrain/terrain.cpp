/*
# _____        ____   ___
#   |     \/   ____| |___|
#   |     |   |   \  |   |
#-----------------------------------------------------------------------
# Copyright 2022, tyra - https://github.com/h4570/tyra
# Licensed under Apache License 2.0
# Sandro Sobczyński <sandro.sobczynski@gmail.com>
*/

#include "states/game/terrain/terrain.hpp"

using Tyra::FileUtils;
using Tyra::ObjLoader;
using Tyra::ObjLoaderOptions;

namespace Demo {
Terrain::Terrain(TextureRepository* repo)
    : heightmap(0.0F,
                10.0F,
                Vec4(-1500.0F, 0.0F, -1500.0F, 1.0F),
                Vec4(1500.0F, 0.0F, 1500.0F, 1.0F)
      ) {
  ObjLoader loader;

  ObjLoaderOptions objOptions;
  objOptions.flipUVs = true;
  objOptions.scale = 160.0F;

  auto data = loader.load(FileUtils::fromCwd("game/models/rockplatform/RockPlatform.obj"),
                          objOptions);
  data->loadNormals = false;
  mesh = new StaticMesh(data.get());

  repo->addByMesh(mesh, FileUtils::fromCwd("game/models/rockplatform/"), "png");

  allocateOptions();

  pair = new RendererStaticPair{mesh, options};
}

float Terrain::getHeightOffset(const Vec4& playerPosition) {
  return heightmap.getHeightOffset(playerPosition);
}

Terrain::~Terrain() {
  delete mesh;
  delete options;
  delete pair;
}

void Terrain::allocateOptions() {
  options = new StaPipOptions();
  options->frustumCulling = Tyra::PipelineFrustumCulling_None;
  options->shadingType = Tyra::TyraShadingGouraud;
  options->blendingEnabled = true;
  options->antiAliasingEnabled = false;
}

}  // namespace Demo
