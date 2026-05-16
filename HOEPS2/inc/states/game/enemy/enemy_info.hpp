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
#include <audsrv.h>
using Tyra::DynamicMesh;
using Tyra::StaticMesh;
using Tyra::Texture;
using Tyra::Vec4;
namespace Demo {
enum EnemyType { ENEMY_OPHANIM, ENEMY_DEMON };
struct EnemyInfo {
  u8 adpcmChannel;
  audsrv_adpcm_t* adpcmPunch;
  audsrv_adpcm_t* adpcmDeath;
  audsrv_adpcm_t* adpcmHit;
  audsrv_adpcm_t* adpcmDemonDeath;
  DynamicMesh* motherMesh;
  Texture* bodyTexture;
  Texture* clothTexture;
  StaticMesh* upperMotherMesh;
  Texture* upperTex1;
  Texture* upperTex2;
  DynamicMesh* legsMotherMesh;
  Texture* legsTex;
  StaticMesh* pentagramMotherMesh;
  Texture* pentagramTex;
  StaticMesh* bloodMotherMesh;
  Texture* bloodTex;
  Vec4 terrainLeftUp;
  Vec4 terrainRightDown;
};
}  // namespace Demo
