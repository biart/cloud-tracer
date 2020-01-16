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

    enum
    {
        InvalidQueueFamilyIndex = ~0u
    };

    VkDevice GetHandler() const;
    VkPhysicalDevice GetPhysicalDevice() const;
    VkSurfaceKHR GetSurface() const;

    bool SupportsGraphics() const;
    bool SupportsPresent() const;
    bool SupportsCompute() const;

    VkQueue GetGraphicsQueue() const;
    std::uint32_t GetGraphicsQueueFamilyIndex() const;
    VkQueue GetPresentQueue() const;
    std::uint32_t GetPresentQueueFamilyIndex() const;
    VkQueue GetComputeQueue() const;
    std::uint32_t GetComputeQueueFamilyIndex() const;

    const VkSurfaceCapabilitiesKHR& GetSurfaceCapabilities() const;
    const std::vector<VkSurfaceFormatKHR>& GetSurfaceFormats() const;
    const std::vector<VkPresentModeKHR>& GetPresentModes() const;

    ~Device();

private:
    VkDevice                    device;
    const VkPhysicalDevice      physical_device;
    const VkSurfaceKHR          surface;

    mutable std::vector<VkPresentModeKHR>       present_modes;
    mutable std::vector<VkSurfaceFormatKHR>     surface_formats;
    VkSurfaceCapabilitiesKHR                    surface_capabilities;

    VkQueue graphics_queue = VK_NULL_HANDLE;
    std::uint32_t graphics_queue_family_index = ~0;
    VkQueue present_queue = VK_NULL_HANDLE;
    std::uint32_t present_queue_family_index = ~0;
    VkQueue compute_queue = VK_NULL_HANDLE;
    std::uint32_t compute_queue_family_index = ~0;

    const std::array<const char*, 1> present_mode_extensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
};

}
}
