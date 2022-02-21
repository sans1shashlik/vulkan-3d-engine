//
// Created by sans1shashlik on 2/19/22.
//
#include "VertexObject.h"

namespace Game::Engine {
    void VertexObject::draw(Engine::VulkanRenderer &renderer) {
        auto camProj = renderer.camera.cameraProjectionMatrix(renderer.getSurfaceExtent());
        uniformBufferData.updateBuffer(renderer.getSurfaceExtent(), camProj*modelMatrix());
        renderer.draw(pipelineData, vertexBufferData.vertexBuffer, descriptorData, [](const vk::raii::CommandBuffer& commandBuffer){
            commandBuffer.draw(12*3, 1, 0, 0);
        });
    }
}


