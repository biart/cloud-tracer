#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <vulkan/debug_messenger.h>
#include <vulkan/device.h>
#include <vulkan/instance.h>

#include <window.h>


namespace ct
{

class Application
{
public:
    Application(const std::string& name, const bool debug = true);

    void Run();
    bool IsRunning();
    const std::string& GetName() const;

    enum
    {
        DefaultWidth = 1024,
        DefaultHeight = 768,
    };

protected:
    virtual void Start() = 0;
    virtual void Update() = 0;
    virtual void Destroy() = 0;

private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT          message_severity,
        VkDebugUtilsMessageTypeFlagsEXT                 message_type,
        const VkDebugUtilsMessengerCallbackDataEXT*     callback_data,
        void*                                           user_data);

    const std::string                           name;
    const bool                                  debug;
    std::unique_ptr<vulkan::Instance>           vk_instance;
    std::unique_ptr<vulkan::DebugMessenger>     vk_debug_messenger;
    std::unique_ptr<vulkan::Device>             vk_device;
    std::unique_ptr<Window>                     window;
    std::unique_ptr<Window::Surface>            surface;
};

}
