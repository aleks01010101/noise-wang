#pragma once

#include "utility/Types.hpp"

#include <vector>

void generateWeights(u32 count, std::vector<f32>& outWeights);
f32 bilerp(f32 tl, f32 tr, f32 bl, f32 br, f32 yWeight, f32 invYWeight, f32 xWeight);
