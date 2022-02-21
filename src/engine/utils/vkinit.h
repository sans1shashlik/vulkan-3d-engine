//
// Created by sans1shashlik on 2/10/22.
//

#ifndef VULKAN_TEST_VKINIT_H
#define VULKAN_TEST_VKINIT_H

#include "utils.h"
#include <iostream>


namespace Game::Engine::Utils
{
    inline VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageFunc( VkDebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT              messageTypes,
                                                            VkDebugUtilsMessengerCallbackDataEXT const * pCallbackData,
                                                            void * /*pUserData*/ )
    {
        std::string message;

        message += vk::to_string( static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>( messageSeverity ) ) +
                   vk::to_string( static_cast<vk::DebugUtilsMessageTypeFlagsEXT>( messageTypes ) ) + ": "
                   + pCallbackData->pMessage;
        /*if ( 0 < pCallbackData->queueLabelCount )
        {
            message += std::string( "\t" ) + "Queue Labels:\n";
            for ( uint8_t i = 0; i < pCallbackData->queueLabelCount; i++ )
            {
                message += std::string( "\t\t" ) + "labelName = <" + pCallbackData->pQueueLabels[i].pLabelName + ">\n";
            }
        }
        if ( 0 < pCallbackData->cmdBufLabelCount )
        {
            message += std::string( "\t" ) + "CommandBuffer Labels:\n";
            for ( uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++ )
            {
                message += std::string( "\t\t" ) + "labelName = <" + pCallbackData->pCmdBufLabels[i].pLabelName + ">\n";
            }
        }
        if ( 0 < pCallbackData->objectCount )
        {
            for ( uint8_t i = 0; i < pCallbackData->objectCount; i++ )
            {
                message += std::string( "\t" ) + "Object " + std::to_string( i ) + "\n";
                message += std::string( "\t\t" ) + "objectType   = " +
                           vk::to_string( static_cast<vk::ObjectType>( pCallbackData->pObjects[i].objectType ) ) + "\n";
                message +=
                        std::string( "\t\t" ) + "objectHandle = " + std::to_string( pCallbackData->pObjects[i].objectHandle ) + "\n";
                if ( pCallbackData->pObjects[i].pObjectName )
                {
                    message += std::string( "\t\t" ) + "objectName   = <" + pCallbackData->pObjects[i].pObjectName + ">\n";
                }
            }
        }*/

        std::cout << message << std::endl;

        return false;
    }

    inline vk::DebugUtilsMessengerCreateInfoEXT createDebugMessenger()
    {
        return { {},
                 vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                 vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
                 &debugMessageFunc
        };
    }

    inline bool isSuitablePhysicalDevice(const vk::raii::PhysicalDevice& physicalDevice) {
        auto requiredExtensions = std::vector<std::string>{Game::Config::RequiredExtensions.begin(), Game::Config::RequiredExtensions.end()};
        for(const auto& extension : physicalDevice.enumerateDeviceExtensionProperties()) {
            const auto it = std::remove(requiredExtensions.begin(), requiredExtensions.end(), extension.extensionName);
            requiredExtensions.erase(it, requiredExtensions.end());
        }
        return requiredExtensions.empty();
    }

    inline vk::raii::PhysicalDevice findPhysicalDevice(std::vector<vk::raii::PhysicalDevice> devices) {
        fmt::print("Looking for suitable physical devices...\n");
        for(auto& device : devices) {
            if(Utils::isSuitablePhysicalDevice(device)) {
                fmt::print("Chosen physical device: {}\n", device.getProperties().deviceName);
                return std::move(device);
            }
        }
        throw std::runtime_error("No suitable GPUs!");
    }

    inline vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, vk::ShaderStageFlagBits stage, const std::string& text) {
        glslang::InitializeProcess();
        auto shaderSpv = std::vector<unsigned int>{};
        if(!GLSLtoSPV(stage, text, shaderSpv))
            throw std::runtime_error("Could not convert GLSL shader to SPV!");
        glslang::FinalizeProcess();
        return { device, {{}, shaderSpv } };
    };

    inline vk::raii::Instance createInstance(const vk::raii::Context& context) {
        fmt::print("Creating instance...\n");

        constexpr auto appInfo = vk::ApplicationInfo {
                Config::AppName,
                Config::AppVersion,
                Config::EngineName,
                Config::EngineVersion,
                VK_API_VERSION_1_3
        };

        uint32_t glfwExtensionCount = 0;
        const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        if(glfwExtensionCount == 0 || glfwExtensions == nullptr)
            throw std::runtime_error("No available instance extensions!");

        /*IF debug*/
        std::vector<char const*> extensions{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
        for(auto i = 0u; i < glfwExtensionCount; i++) {
            extensions.push_back(glfwExtensions[i]);
        }
        std::for_each(extensions.begin(), extensions.end(), [](const auto& e){
            std::cout << "EXTENSION: " << e << '\n';
        });
        std::vector<char const *> instanceLayerNames;
        instanceLayerNames.push_back( "VK_LAYER_KHRONOS_validation" );

        const auto instanceCreateInfo = vk::InstanceCreateInfo { {}, &appInfo, instanceLayerNames, extensions };
        /*ELSE*/
        //const auto instanceCreateInfo = vk::InstanceCreateInfo { {}, &appInfo, {}, {}, glfwExtensionCount, glfwExtensions };

        return { context, instanceCreateInfo };
    }


    inline vk::raii::Device createDevice(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface, uint32_t& presentIndex, uint32_t& graphicsIndex) {
        fmt::print("Creating logical device..\n");

        const auto [p, g] = findQueueFamilyIndex(physicalDevice, surface);
        presentIndex = p;
        graphicsIndex = g;

        const auto queuePriority = 0.0f;
        const auto deviceQueueCreateInfo = vk::DeviceQueueCreateInfo{ {}, graphicsIndex, 1, &queuePriority };
        const auto deviceCreateInfo = vk::DeviceCreateInfo{ {}, deviceQueueCreateInfo, {}, Config::RequiredExtensions };

        return {physicalDevice, deviceCreateInfo};
    }

    inline vk::raii::RenderPass createRenderPass(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::Device& device, const vk::raii::SurfaceKHR& surface) {
        fmt::print("Creating renderpass...\n");
        const auto colorFormat = Utils::pickSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface)).format;
        const auto attachmentDescriptions = std::array<vk::AttachmentDescription,2> {
                vk::AttachmentDescription{
                        {}, colorFormat,
                        vk::SampleCountFlagBits::e1,
                        vk::AttachmentLoadOp::eClear,
                        vk::AttachmentStoreOp::eStore,
                        vk::AttachmentLoadOp::eDontCare,
                        vk::AttachmentStoreOp::eDontCare,
                        vk::ImageLayout::eUndefined,
                        vk::ImageLayout::ePresentSrcKHR
                },
                vk::AttachmentDescription{
                        {}, vk::Format::eD16Unorm,
                        vk::SampleCountFlagBits::e1,
                        vk::AttachmentLoadOp::eClear,
                        vk::AttachmentStoreOp::eDontCare,
                        vk::AttachmentLoadOp::eDontCare,
                        vk::AttachmentStoreOp::eDontCare,
                        vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eDepthStencilAttachmentOptimal
                }
        };

        const auto colorReference = vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal};
        const auto depthReference = vk::AttachmentReference{1, vk::ImageLayout::eDepthStencilAttachmentOptimal};
        const auto subPass = vk::SubpassDescription {{}, vk::PipelineBindPoint::eGraphics,{}, colorReference,{}, &depthReference };
        const auto createInfo = vk::RenderPassCreateInfo{ {}, attachmentDescriptions, subPass };
        return { device,  createInfo };
    }

    inline std::vector<vk::raii::Framebuffer> createFrameBuffers(const vk::raii::Device& device,
                                                                 const vk::raii::ImageView& depthImageView,
                                                                 const std::vector<vk::raii::ImageView>& imageViews,
                                                                 const vk::raii::RenderPass& renderPass,
                                                                 const vk::Extent2D& extent
    )
    {
        fmt::print("Creating framebuffers...\n");
        auto frameBuffers = std::vector<vk::raii::Framebuffer>{};

        std::array<vk::ImageView, 2> attachments;
        attachments.at(1) = *depthImageView;

        frameBuffers.reserve(imageViews.size());
        for( const auto& view : imageViews ) {
            attachments.at(0) = *view;
            const auto createInfo = vk::FramebufferCreateInfo{
                    {},
                    *renderPass,
                    attachments,
                    extent.width,
                    extent.height,
                    1
            };
            frameBuffers.emplace_back(device, createInfo);
        }

        return frameBuffers;
    }
}

#endif //VULKAN_TEST_VKINIT_H
