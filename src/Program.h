//
// Created by sans1shashlik on 1/22/22.
//

#ifndef VULKAN_TEST_PROGRAM_H
#define VULKAN_TEST_PROGRAM_H
#include "engine/includes.h"
#include "engine/VertexObject.h"
#include "engine/VulkanRenderer.h"
#include "engine/Camera.h"

namespace Game {
    class Program {
    public:
        void run();

    private:
        void mainLoop();
        void finish();

        glm::vec3 movVector {0.f, 0.f, 0.f};
        Engine::VulkanRenderer Renderer_{};
        Engine::VertexObject cube{Renderer_, Engine::Utils::Geometry::cubeData, {1.0f, 1.0f, 1.0f}};
    };
};


#endif //VULKAN_TEST_PROGRAM_H
