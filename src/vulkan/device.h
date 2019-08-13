#pragma once

#include <vulkan/instance.h>


namespace ct
{
namespace vulkan
{

class Device
{
public:
    Device(
        const Instance&             vk_instance,
        const VkPhysicalDevice      physical_device,
        VkQueueFlags                requested_queue_flags,
        const VkSurfaceKHR          surface = VK_NULL_HANDLE);

    VkDevice GetHandler() const;

    VkQueue GetGraphicsQueue() const;
    VkQueue GetPresentQueue() const;
    VkQueue GetComputeQueue() const;

    ~Device();

private:
    VkDevice device;
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue present_queue = VK_NULL_HANDLE;
    VkQueue compute_queue = VK_NULL_HANDLE;
};

}
}
