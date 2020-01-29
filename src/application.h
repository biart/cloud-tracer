#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <vulkan/debug_messenger.h>
#include <vulkan/device.h>
#include <vulkan/instance.h>
#include <vulkan/swapchain.h>

#include <window.h>


namespace ct
{

class Application
{
public:
    Application(const ct::vulkan::Instance& vk_instance, const std::string& name);

    void Run();
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
    const std::string           name;

    const vulkan::Instance&     vk_instance;

    const Window                window;
    const Window::Surface       surface;
    const vulkan::Device        vk_device;

    std::size_t     frame_number;
    bool            is_running;
};

}
