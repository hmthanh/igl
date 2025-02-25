/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <igl/Buffer.h>
#include <igl/CommandEncoder.h>
#include <igl/Common.h>
#include <igl/Framebuffer.h>
#include <igl/RenderCommandEncoder.h>
#include <igl/vulkan/CommandBuffer.h>
#include <igl/vulkan/RenderPipelineState.h>
#include <igl/vulkan/ResourcesBinder.h>
#include <igl/vulkan/VulkanImmediateCommands.h>

namespace igl {
namespace vulkan {

/// @brief This class implements the igl::IRenderCommandEncoder interface for Vulkan
class RenderCommandEncoder : public IRenderCommandEncoder {
 public:
  static std::unique_ptr<RenderCommandEncoder> create(
      const std::shared_ptr<CommandBuffer>& commandBuffer,
      VulkanContext& ctx,
      const RenderPassDesc& renderPass,
      const std::shared_ptr<IFramebuffer>& framebuffer,
      const Dependencies& dependencies,
      Result* outResult);

  ~RenderCommandEncoder() override {
    IGL_ASSERT(!isEncoding_); // did you forget to call endEncoding()?
    endEncoding();
  }

  /// @brief Ends encoding for render commands and transitions the layouts of all images bound to
  /// this encoder back to `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`. Also transitions all
  /// dependent textures to `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL`
  void endEncoding() override;

  void pushDebugGroupLabel(const char* label, const igl::Color& color) const override;
  void insertDebugEventLabel(const char* label, const igl::Color& color) const override;
  void popDebugGroupLabel() const override;

  /// @brief Sets the viewport size specified in `viewport`. This function flips the viewport in the
  /// y-direction but retains the same winding as in OpenGL
  void bindViewport(const Viewport& viewport) override;
  void bindScissorRect(const ScissorRect& rect) override;

  void bindRenderPipelineState(const std::shared_ptr<IRenderPipelineState>& pipelineState) override;
  void bindDepthStencilState(const std::shared_ptr<IDepthStencilState>& depthStencilState) override;

  void bindBuffer(int index,
                  uint8_t target,
                  const std::shared_ptr<IBuffer>& buffer,
                  size_t bufferOffset) override;

  /// @brief Not implemented
  void bindBytes(size_t index, uint8_t target, const void* data, size_t length) override;

  /// @brief Binds push constants pointed by `data` with `length` bytes starting at `offset`.
  /// `length` must be a multiple of 4.
  void bindPushConstants(const void* data, size_t length, size_t offset) override;
  void bindSamplerState(size_t index, uint8_t target, ISamplerState* samplerState) override;

  void bindTexture(size_t index, uint8_t target, ITexture* texture) override;

  /// @brief This is only for backends that MUST use single uniforms in some situations. Do not
  /// implement!
  void bindUniform(const UniformDesc& uniformDesc, const void* data) override;

  void draw(PrimitiveType primitiveType, size_t vertexStart, size_t vertexCount) override;
  void drawIndexed(PrimitiveType primitiveType,
                   size_t indexCount,
                   IndexFormat indexFormat,
                   IBuffer& indexBuffer,
                   size_t indexBufferOffset) override;
  void drawIndexedIndirect(PrimitiveType primitiveType,
                           IndexFormat indexFormat,
                           IBuffer& indexBuffer,
                           IBuffer& indirectBuffer,
                           size_t indirectBufferOffset) override;
  void multiDrawIndirect(PrimitiveType primitiveType,
                         IBuffer& indirectBuffer,
                         size_t indirectBufferOffset,
                         uint32_t drawCount,
                         uint32_t stride = 0) override;
  void multiDrawIndexedIndirect(PrimitiveType primitiveType,
                                IndexFormat indexFormat,
                                IBuffer& indexBuffer,
                                IBuffer& indirectBuffer,
                                size_t indirectBufferOffset,
                                uint32_t drawCount,
                                uint32_t stride = 0) override;

  void setStencilReferenceValue(uint32_t value) override;
  void setStencilReferenceValues(uint32_t frontValue, uint32_t backValue) override;
  void setBlendColor(Color color) override;
  void setDepthBias(float depthBias, float slopeScale, float clamp) override;

  VkCommandBuffer getVkCommandBuffer() const {
    return cmdBuffer_;
  }

  igl::vulkan::ResourcesBinder& binder() {
    return binder_;
  }

  /// @brief Enables or disables the draw call count. If enabled, it will increment the draw call,
  /// otherwise it won't. This is used to disable the draw call count when we are doing auxiliary
  /// draw calls such as shader debugging.
  bool setDrawCallCountEnabled(bool value);

 private:
  void bindPipeline();

  /// @brief Ensures that the vertex buffers are bound by performing checks. If the function doesn't
  /// assert at some point, the vertex buffer(s) is bound correctly.
  void ensureVertexBuffers();

  /// @brief Ensures that all shader bindings are bound by checking the SPIR-V reflection. If the
  /// function doesn't assert at some point, the shader bindings are correct.
  void ensureShaderModule(IShaderModule* sm);

 private:
  VulkanContext& ctx_;
  VkCommandBuffer cmdBuffer_ = VK_NULL_HANDLE;
  bool isEncoding_ = false;
  bool hasDepthAttachment_ = false;
  std::shared_ptr<IFramebuffer> framebuffer_;

  igl::vulkan::ResourcesBinder binder_;

  std::shared_ptr<igl::IRenderPipelineState> currentPipeline_ = nullptr;
  RenderPipelineDynamicState dynamicState_;

  /* Used to increment the draw call count. Should either be 0 or 1
   *  0: When draw call count is disabled during auxiliary draw calls (shader debugging)
   *  1: All other times */
  uint32_t drawCallCountEnabled_ = 1u;

  bool isVertexBufferBound_[IGL_VERTEX_BUFFER_MAX] = {};

 private:
  RenderCommandEncoder(const std::shared_ptr<CommandBuffer>& commandBuffer, VulkanContext& ctx);

  void initialize(const RenderPassDesc& renderPass,
                  const std::shared_ptr<IFramebuffer>& framebuffer,
                  const Dependencies& dependencies,
                  Result* outResult);

  Dependencies dependencies_ = {};
};

} // namespace vulkan
} // namespace igl
