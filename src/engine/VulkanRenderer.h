//
// Created by sans1shashlik on 1/28/22.
//

#ifndef VULKAN_TEST_VULKANRENDERER_H
#define VULKAN_TEST_VULKANRENDERER_H

#include <vector>
#include <concepts>
#include <chrono>
#include <thread>
#include "utils/utils.h"
#include "utils/vkinit.h"
#include "Camera.h"

namespace Game::Engine {
    class VulkanRenderer {
    public:
        explicit VulkanRenderer();
        [[nodiscard]] GLFWwindow* getWindow() const;

        Utils::PipelineData createPipeline(std::array<Game::Engine::Utils::Geometry::VertexPC,36>, const vk::raii::DescriptorSetLayout&);

        vk::raii::Device& getDevice() { return device_; }
        vk::raii::PhysicalDevice& getPhysicalDevice() { return physicalDevice_; }
        vk::Extent2D& getSurfaceExtent() { return surfaceData_.extent; }

        void beginFrame();
        void presentFrame();

        Engine::Camera camera{glm::vec3{1.0f, 0.0f, -4.0f}};
    private:
        //void initVulkan();

        uint32_t graphicsQueueFamilyIndex_ = 0;
        uint32_t presentQueueFamilyIndex_ = 0;
        uint32_t imageIndex_ = 0;

        vk::raii::Context context_;
        vk::raii::Instance instance_;
        vk::raii::DebugUtilsMessengerEXT debugMessenger_;
        Utils::SurfaceData surfaceData_;
        vk::raii::PhysicalDevice physicalDevice_;
        vk::raii::Device device_;
        vk::raii::CommandPool commandPool_;
        vk::raii::CommandBuffers commandBuffers_;
        vk::raii::Queue graphicsQueue_;
        vk::raii::Queue presentQueue_;
        Utils::SwapChainData swapChainData_;
        Utils::DepthImage depthImageData_;
        vk::raii::RenderPass renderPass_;
        vk::raii::ShaderModule vertexShaderModule_;
        vk::raii::ShaderModule fragmentShaderModule_;
        std::vector<vk::raii::Framebuffer> frameBuffers_;

        template<class F> void drawCallback(F&& renderCallback,
                                            const Utils::PipelineData& graphicsPipelineData,
                                            const Utils::DescriptorData& descriptorData,
                                            const vk::raii::Buffer& vertexBuffer,
                                            const vk::raii::CommandBuffer& commandBuffer)
        {
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipelineData.pipeline);
            commandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics, *graphicsPipelineData.pipelineLayout, 0, {*descriptorData.descriptorSet}, nullptr );
            commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
            commandBuffer.setViewport(0,
                                      vk::Viewport{0.0f, 0.0f, static_cast<float>(surfaceData_.extent.width), static_cast<float>(surfaceData_.extent.height), 0.0f, 1.0f }
            );
            commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0,0), surfaceData_.extent));
            std::invoke(renderCallback, commandBuffer);
        }

    public:
        template<class F> void draw(const Utils::PipelineData& graphicsPipelineData,
                                    const vk::raii::Buffer& vertexBuffer,
                                    const Utils::DescriptorData& descriptorData,
                                    F&& renderCallback)
        {
            drawCallback(renderCallback, graphicsPipelineData, descriptorData, vertexBuffer, commandBuffers_.front());
        }

    };
}

#endif //VULKAN_TEST_VULKANRENDERER_H
