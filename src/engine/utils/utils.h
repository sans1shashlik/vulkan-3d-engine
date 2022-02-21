//
// Created by sans1shashlik on 2/3/22.
//

#ifndef VULKAN_TEST_UTILS_H
#define VULKAN_TEST_UTILS_H

#include "../includes.h"
#include "geometry.h"
#include "../config.h"
#include "shader.h"

#include <vector>
#include <iostream>

#include <glslang/SPIRV/GlslangToSpv.h>
#include <fmt/core.h>

namespace Game::Engine::Utils {

    inline EShLanguage translateShaderStage( vk::ShaderStageFlagBits stage ) {
        switch (stage) {
            case vk::ShaderStageFlagBits::eVertex:
                return EShLangVertex;
            case vk::ShaderStageFlagBits::eTessellationControl:
                return EShLangTessControl;
            case vk::ShaderStageFlagBits::eTessellationEvaluation:
                return EShLangTessEvaluation;
            case vk::ShaderStageFlagBits::eGeometry:
                return EShLangGeometry;
            case vk::ShaderStageFlagBits::eFragment:
                return EShLangFragment;
            case vk::ShaderStageFlagBits::eCompute:
                return EShLangCompute;
            case vk::ShaderStageFlagBits::eRaygenNV:
                return EShLangRayGenNV;
            case vk::ShaderStageFlagBits::eAnyHitNV:
                return EShLangAnyHitNV;
            case vk::ShaderStageFlagBits::eClosestHitNV:
                return EShLangClosestHitNV;
            case vk::ShaderStageFlagBits::eMissNV:
                return EShLangMissNV;
            case vk::ShaderStageFlagBits::eIntersectionNV:
                return EShLangIntersectNV;
            case vk::ShaderStageFlagBits::eCallableNV:
                return EShLangCallableNV;
            case vk::ShaderStageFlagBits::eTaskNV:
                return EShLangTaskNV;
            case vk::ShaderStageFlagBits::eMeshNV:
                return EShLangMeshNV;
            default:
                assert(false && "Unknown shader stage");
                return EShLangVertex;
        }
    }

    inline bool GLSLtoSPV(const vk::ShaderStageFlagBits& type, const std::string& glslShader, std::vector<unsigned int>& spvShader) {
        EShLanguage stage = translateShaderStage(type);
        const auto shaderStrings = std::array { glslShader.data() };
        auto shader = glslang::TShader{ stage };
        shader.setStrings(shaderStrings.data(), 1);

        const auto msg = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
        if(!shader.parse(&DefaultTBuiltInResource, 100, false, msg)) {
            fmt::print(shader.getInfoLog());
            fmt::print(shader.getInfoDebugLog());
            return false;
        }

        auto program = glslang::TProgram{};
        program.addShader(&shader);
        if(!program.link(msg)) {
            fmt::print(shader.getInfoLog());
            fmt::print(shader.getInfoDebugLog());
            return false;
        }

        glslang::GlslangToSpv( *program.getIntermediate(stage), spvShader);
        return true;
    }

    inline std::pair<uint32_t, uint32_t> findQueueFamilyIndex(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface) {
        const auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
        const auto graphicsQueueFamilyProperty = std::find_if(
                queueFamilyProperties.begin(),
                queueFamilyProperties.end(),
                [](const auto& qfp){
                    return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
                });

        auto graphicsQueueIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
        //check if current index supports graphics and present
        auto presentQueueIndex= physicalDevice.getSurfaceSupportKHR(graphicsQueueIndex, *surface)
                                ? graphicsQueueIndex : queueFamilyProperties.size();
        //doesn't support , look for a good one
        if(presentQueueIndex == queueFamilyProperties.size()) {
            for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
                if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                    physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface))
                {
                    graphicsQueueIndex = static_cast<uint32_t>(i);
                    presentQueueIndex = graphicsQueueIndex;
                    break;
                }
            }
            if(presentQueueIndex == queueFamilyProperties.size()) {
                //none supports both, look for just present support
                for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
                    if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface))
                    {
                        presentQueueIndex = static_cast<uint32_t>(i);
                        break;
                    }
                }
            }
        }
        if((presentQueueIndex == queueFamilyProperties.size()) || (graphicsQueueIndex == queueFamilyProperties.size()))
        {
            throw std::runtime_error("Couldn't find graphics or present support!");
        }

        return {presentQueueIndex, graphicsQueueIndex};
    }

    inline uint32_t findMemoryType(const vk::PhysicalDeviceMemoryProperties& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags propertyMask) {
        auto typeIndex = uint32_t(~0);
        for(auto i = 0u; i < memoryProperties.memoryTypeCount; i++) {
            if( (typeBits & 1) &&
                ((memoryProperties.memoryTypes.at(i).propertyFlags & propertyMask) == propertyMask)
                    ) {
                typeIndex = 1;
                break;
            }
            typeBits >>= 1;
        }
        assert(typeIndex != uint32_t(~0));
        return typeIndex;
    }

    inline vk::SurfaceFormatKHR pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats) {
        assert(!formats.empty());

        auto pickedFormat = formats[0];
        if(formats.size() == 1) {
            if(pickedFormat == vk::Format::eUndefined) {
                pickedFormat.format = vk::Format::eB8G8R8A8Unorm;
                pickedFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
            }
        } else {
            // request several formats, the first found will be used
            vk::ColorSpaceKHR requestedColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
            for (const auto& requestedFormat : { vk::Format::eB8G8R8A8Unorm, vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8Unorm, vk::Format::eR8G8B8Unorm }) {
                const auto it = std::find_if(formats.begin(), formats.end(),
                                             [requestedFormat, requestedColorSpace](vk::SurfaceFormatKHR const& f){
                                                 return (f.format == requestedFormat) && (f.colorSpace == requestedColorSpace);
                                             });
                if (it != formats.end()) {
                    pickedFormat = *it;
                    break;
                }
            }
        }

        assert(pickedFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
        return pickedFormat;
    }

    struct WindowData {
        WindowData(const std::string_view& _windowName, const vk::Extent2D& _extent)
                : extent(_extent),
                  handle(glfwCreateWindow(_extent.width, _extent.height, _windowName.data(), nullptr, nullptr))
        {}

        GLFWwindow* handle;
        vk::Extent2D extent;
    };

    struct SurfaceData {
        SurfaceData(const vk::raii::Instance& _instance, const std::string_view& _windowName, const vk::Extent2D& _extent) :
                extent(_extent), window(_windowName, _extent)
        {
            VkSurfaceKHR rawSurface;
            const auto result = glfwCreateWindowSurface(*_instance, window.handle, nullptr, &rawSurface);
            if(result != VK_SUCCESS)
                throw std::runtime_error("Could not create window surface!");
            surface = vk::raii::SurfaceKHR(_instance, rawSurface);
        }

        vk::Extent2D extent;
        WindowData window;
        vk::raii::SurfaceKHR surface = nullptr;
    };

    struct SwapChainData {
        SwapChainData(const vk::raii::PhysicalDevice& physicalDevice,
                      const vk::raii::SurfaceKHR& surface,
                      const vk::raii::Device& device,
                      uint32_t graphicsIndex, uint32_t presentIndex)
        {
            auto format = Utils::pickSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface)).format;

            const auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
            auto swapChainExtent = vk::Extent2D{};

            if(surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
                swapChainExtent.width =
                        std::clamp(Config::ImageWidth, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
                swapChainExtent.height =
                        std::clamp(Config::ImageWidth, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            }
            else
                swapChainExtent = surfaceCapabilities.currentExtent;

            auto swapChainPresentMode = vk::PresentModeKHR::eFifo;
            for(const auto mode : physicalDevice.getSurfacePresentModesKHR(*surface)) {
                if(mode == vk::PresentModeKHR::eImmediate) {
                    swapChainPresentMode = mode;
                    break;
                }
            }
            const auto preTransform =
                    ( surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity )
                    ? vk::SurfaceTransformFlagBitsKHR::eIdentity
                    : surfaceCapabilities.currentTransform;
            const auto compositeAlpha =
                    ( surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied )
                    ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
                    : ( surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied )
                      ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
                      : ( surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit )
                        ? vk::CompositeAlphaFlagBitsKHR::eInherit
                        : vk::CompositeAlphaFlagBitsKHR::eOpaque;

            auto swapChainCreateInfo = vk::SwapchainCreateInfoKHR{ vk::SwapchainCreateFlagsKHR(),
                                                                   *surface,
                                                                   surfaceCapabilities.minImageCount,
                                                                   format,
                                                                   vk::ColorSpaceKHR::eSrgbNonlinear,
                                                                   swapChainExtent,
                                                                   1,
                                                                   vk::ImageUsageFlagBits::eColorAttachment,
                                                                   vk::SharingMode::eExclusive,
                                                                   {},
                                                                   preTransform,
                                                                   compositeAlpha,
                                                                   swapChainPresentMode,
                                                                   true,
            };


            if ( graphicsIndex != presentIndex )
            {
                const auto queueFamilyIndices = std::array<uint32_t, 2>{ graphicsIndex, presentIndex };
                swapChainCreateInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
                swapChainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>( queueFamilyIndices.size() );
                swapChainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
            }

            swapChain = vk::raii::SwapchainKHR{device, swapChainCreateInfo};

            const auto images = swapChain.getImages();
            imageViews.reserve(images.size());

            auto imageViewInfo = vk::ImageViewCreateInfo{
                    {}, {},
                    vk::ImageViewType::e2D,
                    format,
                    {},
                    { vk::ImageAspectFlagBits::eColor, 0,1,0,1 }
            };
            for(const auto& image : images) {
                imageViewInfo.image = image;
                imageViews.emplace_back(device, imageViewInfo);
            }
            //fmt::print("ImageViews count: {}\n", imageViews.size());
        }

        std::vector<vk::raii::ImageView> imageViews;
        vk::raii::SwapchainKHR swapChain = nullptr;
    };

    struct DepthImage {
        DepthImage(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::Device& device, const vk::Extent2D& extent) {
            fmt::print("Creating depth image...\n");

            const auto formatProperties = physicalDevice.getFormatProperties(depthFormat);

            const auto imageTiling =
                    (formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
                    ? vk::ImageTiling::eLinear
                    : (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
                      ? vk::ImageTiling::eOptimal
                      : throw std::runtime_error("Depth Stencil attachment not supported with D16Unorm depth format.");

            depthImage = vk::raii::Image{device, {
                    {},
                    vk::ImageType::e2D,
                    depthFormat,
                    vk::Extent3D{extent, 1},
                    1,
                    1,
                    vk::SampleCountFlagBits::e1,
                    imageTiling,
                    vk::ImageUsageFlagBits::eDepthStencilAttachment
            }};

            const auto memoryProperties = physicalDevice.getMemoryProperties();
            const auto memoryRequirements = depthImage.getMemoryRequirements();

            const auto typeIndex = Utils::findMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
            const auto memoryAllocInfo = vk::MemoryAllocateInfo{memoryRequirements.size, typeIndex};
            depthMemory = vk::raii::DeviceMemory{device, memoryAllocInfo};
            depthImage.bindMemory(*depthMemory, 0);

            depthImageView = {
                device, {
                    {},
                    *depthImage,
                    vk::ImageViewType::e2D,
                    vk::Format::eD16Unorm,
                    {},
                    { vk::ImageAspectFlagBits::eDepth, 0,1,0,1 }
                }
            };
        }

        vk::Format depthFormat = vk::Format::eD16Unorm;
        vk::raii::DeviceMemory depthMemory = nullptr;
        vk::raii::Image depthImage = nullptr;
        vk::raii::ImageView depthImageView = nullptr;
    };

    struct UniformBufferObject { glm::mat4x4 mvp; glm::mat4x4 transform; glm::mat4x4 proj; };
    struct UniformBufferData {

        UniformBufferData(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::Device& device, const vk::Extent2D& extent, const glm::mat4x4 uniformData) {
            uniformBuffer = vk::raii::Buffer{ device, {{}, sizeof(uniformData), vk::BufferUsageFlagBits::eUniformBuffer }};

            const auto memoryRequirements = uniformBuffer.getMemoryRequirements();
            const auto typeIndex = Utils::findMemoryType(physicalDevice.getMemoryProperties(), memoryRequirements.memoryTypeBits,
                                                         vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            const auto memoryAllocInfo = vk::MemoryAllocateInfo{memoryRequirements.size, typeIndex};
            uniformMemory = vk::raii::DeviceMemory{device, memoryAllocInfo};

            auto data = static_cast<uint8_t *>( uniformMemory.mapMemory( 0, memoryRequirements.size ) );
            memcpy( data, &uniformData, sizeof( uniformData ) );
            uniformMemory.unmapMemory();
            uniformBuffer.bindMemory(*uniformMemory, 0);
        }

        void updateBuffer(const vk::Extent2D& extent, glm::mat4x4 uniformData) {
            auto data = static_cast<uint8_t *>( uniformMemory.mapMemory( 0, uniformBuffer.getMemoryRequirements().size ) );
            memcpy(data, &uniformData, sizeof( uniformData ) );
            uniformMemory.unmapMemory();
        }

        vk::raii::Buffer uniformBuffer = nullptr;
        vk::raii::DeviceMemory uniformMemory = nullptr;
    };

    struct DescriptorData {
        DescriptorData(const vk::raii::Device& device, const vk::DescriptorType type, const vk::raii::Buffer& uniformBuffer) {
            fmt::print("Creating descriptor set layout...\n");
            const auto binding = vk::DescriptorSetLayoutBinding{ 0, type, 1, vk::ShaderStageFlagBits::eVertex };
            descriptorSetLayout = { device, { {}, binding } };

            fmt::print("Creating descriptor pool...\n");
            const auto poolSize = vk::DescriptorPoolSize{type, 1};
            descriptorPool = {device, { vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSize }};

            descriptorSet = std::move(vk::raii::DescriptorSets{ device, { *descriptorPool, *descriptorSetLayout }}.front());
            descriptorBufferInfo = vk::DescriptorBufferInfo{ *uniformBuffer, 0, sizeof(glm::mat4x4)};
            const auto writeDescriptorSet = vk::WriteDescriptorSet{
                    *descriptorSet,
                    0,
                    0,
                    type,
                    {},
                    descriptorBufferInfo
            };
            device.updateDescriptorSets(writeDescriptorSet, nullptr);
        }

        vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
        vk::raii::DescriptorPool descriptorPool = nullptr;
        vk::DescriptorBufferInfo descriptorBufferInfo;
        vk::raii::DescriptorSet descriptorSet = nullptr;
    };

    struct VertexBufferData {
        VertexBufferData(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice, auto geometry) {
            fmt::print("Creating vertex buffer...\n");
            vertexBuffer = vk::raii::Buffer{ device, {{}, sizeof(geometry), vk::BufferUsageFlagBits::eVertexBuffer }};

            const auto memoryRequirements = vertexBuffer.getMemoryRequirements();
            const auto typeIndex = Utils::findMemoryType(physicalDevice.getMemoryProperties(), memoryRequirements.memoryTypeBits,
                                                         vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            const auto memoryAllocInfo = vk::MemoryAllocateInfo{memoryRequirements.size, typeIndex};
            vertexMemory = vk::raii::DeviceMemory{device, memoryAllocInfo};
            auto data = static_cast<uint8_t *>( vertexMemory.mapMemory(0, memoryRequirements.size ) );
            memcpy( data, &geometry, sizeof(geometry) );
            vertexMemory.unmapMemory();
            vertexBuffer.bindMemory(*vertexMemory, 0);
        }

        vk::raii::Buffer vertexBuffer = nullptr;
        vk::raii::DeviceMemory vertexMemory = nullptr;
    };

    struct PipelineData {
        PipelineData(const vk::raii::Device& device,
                     const vk::raii::DescriptorSetLayout& descriptorSetLayout,
                     const vk::raii::ShaderModule& vertexShaderModule,
                     const vk::SpecializationInfo* vertexInfo,
                     const vk::raii::ShaderModule& fragmentShaderModule,
                     const vk::SpecializationInfo* fragmentInfo,
                     uint32_t vertexStride,
                     const std::vector<std::pair<vk::Format, uint32_t>>& vertexInputAttributes,
                     vk::FrontFace front,
                     bool depthBuffered,
                     const vk::raii::RenderPass& renderPass
        ) : pipelineCache(device, {}), pipelineLayout(device,{ {}, *descriptorSetLayout})
        {
            fmt::print("Creating graphics pipeline...\n");
            const auto shaderStageCreateInfos = std::array {
                vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eVertex, *vertexShaderModule, "main", vertexInfo },
                vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eFragment, *fragmentShaderModule, "main", fragmentInfo }
            };
            const auto vertexInputBindingDescription = vk::VertexInputBindingDescription(0, vertexStride);

            std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
            if(vertexStride > 0) {
                vertexInputAttributeDescriptions.reserve(vertexInputAttributes.size());
                for(auto i = 0u; const auto& attr : vertexInputAttributes) {
                    vertexInputAttributeDescriptions.emplace_back(i++, 0, attr.first, attr.second);
                }
            }
            const auto pipelineVertexInputStateCreateInfo =
                    vk::PipelineVertexInputStateCreateInfo { {}, vertexInputBindingDescription, vertexInputAttributeDescriptions };

            const auto pipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo { {}, vk::PrimitiveTopology::eTriangleList };
            const auto pipelineViewportStateCreateInfo = vk::PipelineViewportStateCreateInfo{ {}, 1, nullptr, 1, nullptr };
            const auto pipelineRasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo {
                    {},
                    false,
                    false,
                    vk::PolygonMode::eFill,
                    vk::CullModeFlagBits::eBack,
                    front,
                    false,
                    0.0f,
                    0.0f,
                    0.0f,
                    1.0f
            };

            const auto pipelineMultiSampleCreateInfo = vk::PipelineMultisampleStateCreateInfo { {}, vk::SampleCountFlagBits::e1 };
            const auto stencilOpState = vk::StencilOpState {vk::StencilOp::eKeep,vk::StencilOp::eKeep,vk::StencilOp::eKeep,vk::CompareOp::eAlways };
            const auto pipelineDepthStencilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo {
                    {},
                    depthBuffered,
                    depthBuffered,
                    vk::CompareOp::eLessOrEqual,
                    false,
                    false,
                    stencilOpState,
                    stencilOpState
            };

            const auto pipelineColorBlendAttachmentState = vk::PipelineColorBlendAttachmentState {
                false,
                vk::BlendFactor::eZero,
                vk::BlendFactor::eZero,
                vk::BlendOp::eAdd,
                vk::BlendFactor::eZero,
                vk::BlendFactor::eZero,
                vk::BlendOp::eAdd,
                { vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA }
            };
            const auto pipelineColorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo {
                    {}, false, vk::LogicOp::eNoOp, pipelineColorBlendAttachmentState, { {1.0f, 1.0f, 1.0f, 1.0f} }
            };
            const auto dynamicStates = std::array { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
            const auto pipelineDynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo{ {}, dynamicStates };

            pipeline = { device, nullptr, {
                    {},
                    shaderStageCreateInfos,
                    &pipelineVertexInputStateCreateInfo,
                    &pipelineInputAssemblyStateCreateInfo,
                    nullptr,
                    &pipelineViewportStateCreateInfo,
                    &pipelineRasterizationStateCreateInfo,
                    &pipelineMultiSampleCreateInfo,
                    &pipelineDepthStencilStateCreateInfo,
                    &pipelineColorBlendStateCreateInfo,
                    &pipelineDynamicStateCreateInfo,
                    *pipelineLayout,
                    *renderPass
            }};
        }

        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::PipelineCache pipelineCache;
        vk::raii::Pipeline pipeline = nullptr;
    };
}
#endif //VULKAN_TEST_UTILS_H
