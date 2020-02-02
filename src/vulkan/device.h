#pragma once


#include <array>
#include <vector>

#include <vulkan/object.h>


namespace ct
{
namespace vulkan
{

class Instance;


enum QueueType : std::uint32_t
{
    ComputeQueue = VK_QUEUE_COMPUTE_BIT,
    GraphicsQueue = VK_QUEUE_GRAPHICS_BIT,
    PresentQueue = VK_QUEUE_PROTECTED_BIT
};


const std::array<QueueType, 3>& AllQueueTypes();


using QueueFlags = std::uint32_t;


class Device : public Object<VkDevice>
{
public:
    explicit Device(
        const Instance&             vk_instance,
        const VkPhysicalDevice      physical_device,
        QueueFlags                  requested_queue_flags,
        const VkSurfaceKHR          surface = VK_NULL_HANDLE);
    Device(Device&& other);

    enum : std::uint32_t
    {
        InvalidQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED
    };

    VkPhysicalDevice GetPhysicalDevice() const;
    VkSurfaceKHR GetSurface() const;

    bool Supports(const QueueType type) const;

    VkQueue GetQueue(const QueueType type) const;
    std::uint32_t GetQueueFamilyIndex(const QueueType type) const;

    const VkSurfaceCapabilitiesKHR& GetSurfaceCapabilities() const;
    const std::vector<VkSurfaceFormatKHR>& GetSurfaceFormats() const;
    const std::vector<VkPresentModeKHR>& GetPresentModes() const;

    ~Device();

private:
    const VkPhysicalDevice      physical_device;
    const VkSurfaceKHR          surface;

    mutable std::vector<VkPresentModeKHR>       present_modes;
    mutable std::vector<VkSurfaceFormatKHR>     surface_formats;
    VkSurfaceCapabilitiesKHR                    surface_capabilities;

    struct QueuesInfo
    {
        VkQueue graphics_queue = VK_NULL_HANDLE;
        std::uint32_t graphics_queue_family_index = InvalidQueueFamilyIndex;
        VkQueue present_queue = VK_NULL_HANDLE;
        std::uint32_t present_queue_family_index = InvalidQueueFamilyIndex;
        VkQueue compute_queue = VK_NULL_HANDLE;
        std::uint32_t compute_queue_family_index = InvalidQueueFamilyIndex;
    };
    QueuesInfo queues_info;

    const std::array<const char*, 1> present_mode_extensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
};

}
}
