#include "application.h"


#include <utils/ignore_unused.h>
#include <vulkan/command_pool.h>
#include <vulkan/memory.h>


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
        ct::vulkan::PresentQueue | ct::vulkan::ComputeQueue | ct::vulkan::GraphicsQueue,
        surface->GetHandler()));
    vk_swapchain.reset(new vulkan::Swapchain(*vk_device, DefaultWidth, DefaultHeight));

    {
        ct::vulkan::Buffer<float, ct::vulkan::DeviceMemory, true, false> stagingBuffer(*vk_device, 1024);
        ct::vulkan::Buffer<float, ct::vulkan::DeviceMemory, false, true> buffer(*vk_device, 1024);

        ct::vulkan::CommandPool command_pool(*vk_device, ct::vulkan::ComputeQueue);

        // Command buffer: copy from host to device
        ct::vulkan::CommandBuffer copy_command_buffer(command_pool);
        {
            ct::vulkan::CommandRecorder recorder(copy_command_buffer);
            recorder.Transfer(stagingBuffer, buffer);
        }

        Start();
        while (!window->ShouldClose())
        {
            Update();
            ct::vulkan::SubmitCommands(copy_command_buffer);
            command_pool.WaitForQueue();
            glfwPollEvents();
        }
        Destroy();
    }

    vk_swapchain.release();
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
    void*                                           user_data)
{
    utils::IgnoreUnused(message_severity, message_type, user_data);

    std::cout << "Vulkan validation layer: " << callback_data->pMessage << std::endl;

    return VK_FALSE;
}

}
