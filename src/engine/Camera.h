//
// Created by sans1shashlik on 2/11/22.
//

#ifndef VULKAN_TEST_CAMERA_H
#define VULKAN_TEST_CAMERA_H

#include "includes.h"
#include "utils/utils.h"

namespace Game::Engine {
    class Camera {
    public:
        explicit Camera(const glm::vec3 _pos={0.0f,0.0f,0.0f},
               const glm::vec4 _rot={0.0f, 0.0f, 0.0f, 0.0f},
               const float _fov=45.0f
        ) : position(_pos), rotation(_rot), fov(_fov)
        {}

        glm::vec3 position;
        glm::vec4 rotation;
        float fov;

        glm::mat4x4 cameraProjectionMatrix(const vk::Extent2D&);

        void move(float x, float y, float z) { position += glm::vec3{x,y,z}; }
        void move(glm::vec3 newPos) { position += newPos; }

        auto getPos() { return std::tuple {position.x, position.y, position.z}; }
    private:

    };
}

#endif //VULKAN_TEST_CAMERA_H
