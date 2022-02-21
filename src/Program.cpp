//
// Created by sans1shashlik on 1/22/22.
//

#include "Program.h"
#include <iostream>

namespace Game {

    void Program::finish() {
        glfwDestroyWindow(Renderer_.getWindow());
        glfwTerminate();
        fmt::print("Exiting...");
    }

    void Program::mainLoop() {
        glfwSetWindowUserPointer(Renderer_.getWindow(), this);

        auto multiplier = 1.0f;
        auto t = 0.f;
        const auto dt = 1 / 300.0f;
        auto accumulator = 0.f;
        auto currentTime = std::chrono::system_clock::now();

        while (!glfwWindowShouldClose(Renderer_.getWindow())) {
            auto newTime = std::chrono::system_clock::now();
            auto frameTime = std::chrono::duration<float>{newTime - currentTime}.count();
            currentTime = newTime;

            //input
            glfwPollEvents();
            glfwSetKeyCallback(Renderer_.getWindow(), [](GLFWwindow* window, int key, int scancode, int action, int mods){
                if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }

                auto self = static_cast<Program*>(glfwGetWindowUserPointer(window));
                if(action == GLFW_PRESS || action == GLFW_REPEAT) {
                    if(key == GLFW_KEY_Z) self->movVector.y = 1.f;
                    else if(key == GLFW_KEY_X) self->movVector.y = -1.f;

                    if(key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) self->movVector.x = 1.f;
                    else if(key == GLFW_KEY_A || key == GLFW_KEY_LEFT) self->movVector.x = -1.f;

                    if(key == GLFW_KEY_UP || key == GLFW_KEY_W) self->movVector.z = 1.f;
                    else if(key == GLFW_KEY_DOWN || key == GLFW_KEY_S) self->movVector.z = -1.f;
                }
            });


            const auto speed = 25.f;
            accumulator += frameTime;
            while(accumulator >= dt) {
                if (movVector.x != 0.f || movVector.y != 0.f || movVector.z != 0.f) {
                    Renderer_.camera.move(movVector.x * speed * dt,
                                          movVector.y * speed * dt,
                                          movVector.z * speed * dt);
                }
                movVector = {0.f, 0.f, 0.f};

                cube.rotation.x += 0.1f * dt;
                cube.rotation.z += 0.1f * dt;

                cube.scale += 0.1f * multiplier * dt;
                if (cube.scale.x >= 2.f || cube.scale.x <= 0.5f)
                    multiplier *= -1.f;

                /*IT'S BEEN SO LONG
                SINCE I LAST HAVE SEEN MY SON,
                LOST TO THIS MONSTER,
                TO THE MAN BEHIND THE SLAUGHTER.*/

                //end update
                accumulator -= dt;
                t += dt;
            }
            //draw
            Renderer_.beginFrame();
            cube.draw(Renderer_);
            Renderer_.presentFrame();
        }
    }

    void Program::run() {
        mainLoop();
        finish();
    }
}