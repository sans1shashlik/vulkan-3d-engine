//
// Created by sans1shashlik on 2/10/22.
//

#ifndef VULKAN_TEST_VERTEXOBJECT_H
#define VULKAN_TEST_VERTEXOBJECT_H

#include "VulkanRenderer.h"

namespace Game::Engine {
    struct VertexObject {
    public:
        VertexObject(VulkanRenderer& _renderer,
                     std::array<Utils::Geometry::VertexPC, 36> geometry,
                     glm::vec3 _pos = {.0f, .0f, .0f},
                     glm::vec3 _rot = {.0f, .0f, .0f},
                     glm::vec3 _scale = {1.0f, 1.0f, 1.0f}
                     )
        : position(_pos), rotation(_rot), scale(_scale),
          uniformBufferData(_renderer.getPhysicalDevice(),
                            _renderer.getDevice(),
                            _renderer.getSurfaceExtent(),
                            _renderer.camera.cameraProjectionMatrix(_renderer.getSurfaceExtent())),
          descriptorData(_renderer.getDevice(), vk::DescriptorType::eUniformBuffer, uniformBufferData.uniformBuffer),
          vertexBufferData(_renderer.getDevice(), _renderer.getPhysicalDevice(), geometry),
          pipelineData(_renderer.createPipeline(geometry, descriptorData.descriptorSetLayout))
        {}

        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
        Utils::UniformBufferData uniformBufferData;
        Utils::DescriptorData descriptorData;
        Utils::VertexBufferData vertexBufferData;
        Utils::PipelineData pipelineData;

        void draw(Engine::VulkanRenderer& renderer);
        auto getPos() { return std::tuple{ position.x, position.y, position.z }; }
        auto getRot() { return std::tuple{ rotation.x, rotation.y, rotation.z }; }
    private:
        auto modelMatrix() {
            auto transform = glm::mat4x4{1.0f};
            transform = glm::translate(transform, position);
            transform = glm::rotate(transform, rotation.y, {0.0f, 1.0f, 0.f});
            transform = glm::rotate(transform, rotation.x, {1.0f, 0.0f, 0.f});
            transform = glm::rotate(transform, rotation.z, {0.0f, 0.0f, 1.f});
            transform = glm::scale(transform, scale);
            return transform;
        };
    };
}

#endif //VULKAN_TEST_VERTEXOBJECT_H
