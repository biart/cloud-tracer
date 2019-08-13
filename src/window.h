#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/instance.h>


namespace ct
{

class Window
{
public:
    Window(const std::string& name, int default_width, int default_height);

    struct Surface
    {
    public:
        Surface(const vulkan::Instance& vk_instance, const Window& window) :
            vk_instance(vk_instance)
        {
            if (glfwCreateWindowSurface(vk_instance.GetHandler(), window.window, nullptr, &vk_surface) != VK_SUCCESS)
            {
                throw vulkan::Exception("Failed to create window surface");
            }
        }

        VkSurfaceKHR GetHandler() const
        {
            return vk_surface;
        }

        ~Surface()
        {
            vkDestroySurfaceKHR(vk_instance.GetHandler(), vk_surface, nullptr);
        }

    private:
        const vulkan::Instance&     vk_instance;
        VkSurfaceKHR                vk_surface;
    };

    bool ShouldClose() const;

    ~Window();

private:
    GLFWwindow* window;
};

}
