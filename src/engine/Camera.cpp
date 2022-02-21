//
// Created by sans1shashlik on 2/11/22.
//

#include "Camera.h"

namespace Game::Engine {
    glm::mat4x4 Camera::cameraProjectionMatrix(const vk::Extent2D& extent) {
        auto fieldOfView =
                ( extent.width > extent.height )
                ? fov * (static_cast<float>( extent.height ) / static_cast<float>( extent.width ))
                : fov;

        glm::mat4x4 model = glm::mat4x4 {1.0f};
        glm::mat4x4 view =
                glm::lookAt( position, position+glm::vec3(0.0f,0.0f,10.0f), glm::vec3( 0.0f, -1.0f, 0.0f ) );
        glm::mat4x4 projection = glm::perspective( fieldOfView, 1.0f, 0.1f, 100.0f );
        glm::mat4x4 clip = glm::mat4x4( 1.0f,  0.0f, 0.0f, 0.0f,
                                        0.0f, -1.0f, 0.0f, 0.0f,
                                        0.0f,  0.0f, 0.5f, 0.0f,
                                        0.0f,  0.0f, 0.5f, 1.0f );
        // vulkan clip space has inverted y and half z !
        return clip * projection * view * model;
    }
}