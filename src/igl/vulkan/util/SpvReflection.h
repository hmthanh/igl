/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <igl/Texture.h>
#include <vector>

namespace igl::vulkan::util {

constexpr uint32_t kNoBindingLocation = 0xffffffff;
constexpr uint32_t kNoDescriptorSet = 0xffffffff;

struct TextureDescription {
  uint32_t bindingLocation = kNoBindingLocation;
  uint32_t descriptorSet = kNoDescriptorSet;
  TextureType type = TextureType::Invalid;
};

struct BufferDescription {
  uint32_t bindingLocation = kNoBindingLocation;
  uint32_t descriptorSet = kNoDescriptorSet;
};

struct SpvModuleInfo {
  std::vector<BufferDescription> uniformBuffers;
  std::vector<BufferDescription> storageBuffers;
  std::vector<TextureDescription> textures;
};

SpvModuleInfo getReflectionData(const uint32_t* spirv, size_t numBytes);

} // namespace igl::vulkan::util
