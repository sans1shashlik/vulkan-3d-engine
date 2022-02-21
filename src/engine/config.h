//
// Created by sans1shashlik on 1/22/22.
//

#ifndef VULKAN_TEST_CONFIG_H
#define VULKAN_TEST_CONFIG_H

namespace Game::Config {
    constexpr auto WindowWidth = 800u;
    constexpr auto WindowHeight = 600u;
    constexpr auto WindowTitle = "Vulkan Test";

    constexpr auto AppName = "Vulkan Test";
    constexpr auto EngineName = "No Engine";
    constexpr auto AppVersion = 1;
    constexpr auto EngineVersion = 1;

    constexpr auto ImageWidth = 64u;
    constexpr auto ImageHeight = 64u;

    constexpr auto RequiredExtensions = std::array {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    constexpr auto Timeout = uint64_t{10000000};
}

#endif //VULKAN_TEST_CONFIG_H
