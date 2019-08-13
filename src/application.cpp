#include "application.h"


namespace ct
{

Application::Application(const std::string& name, const bool debug) :
    name(name), debug(debug)
{
}


void Application::Run()
{
    if (IsRunning())
        return;

    glfwInit();
    window.reset(new Window(name, DefaultWidth, DefaultHeight));
    std::uint32_t glfw_ext_count;
    const char** glfw_ext = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
    vk_instance.reset(new vulkan::Instance(name, "", std::vector<const char*>(glfw_ext, glfw_ext + glfw_ext_count), debug));
    vk_debug_messenger.reset(debug ? new vulkan::DebugMessenger(*vk_instance, DebugCallback) : nullptr);
    surface.reset(new Window::Surface(*vk_instance, *window));
    vk_device.reset(new vulkan::Device(
        *vk_instance,
        vk_instance->GetPhysicalDevices()[0],
        VK_QUEUE_GRAPHICS_BIT,
        surface->GetHandler()));

    Start();
    while (!window->ShouldClose())
    {
        Update();
        glfwPollEvents();
    }
    Destroy();

    vk_device.release();
    surface.release();
    vk_debug_messenger.release();
    vk_instance.release();
    window.release();
}


bool Application::IsRunning()
{
    return window != nullptr;
}


const std::string& Application::GetName() const
{
    return name;
}


VKAPI_ATTR VkBool32 VKAPI_CALL Application::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT          message_severity,
    VkDebugUtilsMessageTypeFlagsEXT                 message_type,
    const VkDebugUtilsMessengerCallbackDataEXT*     callback_data,
    void* user_data)
{
    std::cout << "Vulkan validation layer: " << callback_data->pMessage << std::endl;

    return VK_FALSE;
}

}
