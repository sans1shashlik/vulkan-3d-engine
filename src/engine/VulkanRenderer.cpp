//
// Created by sans1shashlik on 1/28/22.
//

#include "VulkanRenderer.h"
#include "utils/geometry.h"
#include "config.h"

#include <fmt/core.h>

namespace Game::Engine {
    VulkanRenderer::VulkanRenderer()
    : context_(),
      instance_(Utils::createInstance(context_)),
      debugMessenger_(instance_, Utils::createDebugMessenger()),
      surfaceData_(instance_, Config::AppName, vk::Extent2D(Config::WindowWidth, Config::WindowHeight)),
      physicalDevice_(Utils::findPhysicalDevice(instance_.enumeratePhysicalDevices())),
      device_(Utils::createDevice(physicalDevice_, surfaceData_.surface, presentQueueFamilyIndex_, graphicsQueueFamilyIndex_)),
      commandPool_(device_, {vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsQueueFamilyIndex_}),
      commandBuffers_(device_, {*commandPool_, vk::CommandBufferLevel::ePrimary, 1}),
      graphicsQueue_(device_, graphicsQueueFamilyIndex_, 0),
      presentQueue_(device_, presentQueueFamilyIndex_, 0),
      swapChainData_(physicalDevice_, surfaceData_.surface, device_, graphicsQueueFamilyIndex_, presentQueueFamilyIndex_),
      depthImageData_(physicalDevice_, device_, surfaceData_.extent),
      renderPass_(Utils::createRenderPass(physicalDevice_, device_, surfaceData_.surface)),
      frameBuffers_(Utils::createFrameBuffers(device_, depthImageData_.depthImageView, swapChainData_.imageViews, renderPass_, surfaceData_.extent)),
      vertexShaderModule_(Utils::createShaderModule(device_, vk::ShaderStageFlagBits::eVertex, Utils::vertexShaderText)),
      fragmentShaderModule_(Utils::createShaderModule(device_, vk::ShaderStageFlagBits::eFragment, Utils::fragmentShaderText))
    {}

    /*void VulkanRenderer::initVulkan() {
        //debug something validation layers
    }*/
    /*void VulkanRenderer::destroy() {
        glfwDestroyWindow(window_);
    }*/

    Utils::PipelineData VulkanRenderer::createPipeline(std::array<Game::Engine::Utils::Geometry::VertexPC,36> geometry, const vk::raii::DescriptorSetLayout& descLayout) {
        return {
                device_,
                descLayout,
                vertexShaderModule_,
                nullptr,
                fragmentShaderModule_,
                nullptr,
                static_cast<uint32_t>(sizeof(geometry.at(0))),
                {{vk::Format::eR32G32B32A32Sfloat, 0}, {vk::Format::eR32G32B32A32Sfloat, 16}},
                vk::FrontFace::eClockwise,
                true,
                renderPass_
        };
    }

    GLFWwindow* VulkanRenderer::getWindow() const {
        return surfaceData_.window.handle;
    }

    void VulkanRenderer::beginFrame() {
        const auto imageSemaphore = vk::raii::Semaphore{device_, vk::SemaphoreCreateInfo{}};;
        while(true) {
            auto[result, imageIndex] = swapChainData_.swapChain.acquireNextImage(Config::Timeout, *imageSemaphore);

            if(result == vk::Result::eTimeout)
                continue;
            if (result != vk::Result::eSuccess)
                throw std::runtime_error("Failed to acquire next image from swapchain");

            assert(imageIndex < swapChainData_.imageViews.size());
            imageIndex_ = imageIndex;
            break;
        }

        const auto clearValues = std::array<vk::ClearValue,2> {
                vk::ClearColorValue{ std::array{0.0f, 0.0f, 0.0f, 0.0f} },
                vk::ClearDepthStencilValue{ 1.0f, 0 }
        };

        const auto& commandBuffer = commandBuffers_.front();

        commandBuffer.begin({});
        commandBuffer.beginRenderPass(
                {*renderPass_, *frameBuffers_.at(imageIndex_), vk::Rect2D({0,0}, surfaceData_.extent), clearValues},
                vk::SubpassContents::eInline
        );
    }

    void VulkanRenderer::presentFrame() {
        const auto& commandBuffer = commandBuffers_.front();

        commandBuffer.endRenderPass();
        commandBuffer.end();

        const auto fence = vk::raii::Fence{device_, vk::FenceCreateInfo{}};
        graphicsQueue_.submit(vk::SubmitInfo{nullptr, nullptr, *commandBuffer}, *fence);
        while(vk::Result::eTimeout == device_.waitForFences({*fence}, VK_TRUE, Config::Timeout));

        const auto result = presentQueue_.presentKHR({ nullptr, *swapChainData_.swapChain, imageIndex_ });
        if(result == vk::Result::eSuboptimalKHR) {
            fmt::print("Suboptimal present result");
        } else if(result != vk::Result::eSuccess) {
            throw std::runtime_error("Unexpected present result!");
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(16));
        device_.waitIdle();
    }
}