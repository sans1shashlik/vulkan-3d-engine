#include "Program.h"
#include <fmt/format.h>

int main() {
    try {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        //glfw has to be initialised before the vulkan renderer

        auto game = Game::Program();
        game.run();
    } catch(vk::SystemError &error) {
        fmt::print("[ERROR] An error occured!: {}\n", error.what());
        exit(-1);
    } catch(std::exception &error) {
        fmt::print("[ERROR] An error occured!: {}\n", error.what());
        exit(-1);
    } catch(...) {
        fmt::print("[ERROR] An unknown error occured!\n");
        exit(-1);
    }
    return 0;
}
