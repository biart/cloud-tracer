#include "device.h"

#include <unordered_set>

#include <vulkan/instance.h>


namespace ct
{
namespace vulkan
{

const std::array<QueueType, 3>& AllQueueTypes()
{
    static constexpr std::array<QueueType, 3> all_queue_types = {
        ComputeQueue,
        GraphicsQueue,
        PresentQueue
    };
    return all_queue_types;
}


Device::Device(
    const Instance&             vk_instance,
    const VkPhysicalDevice      physical_device,
    QueueFlags                  requested_queue_flags,
    const VkSurfaceKHR          surface) :
    physical_device(physical_device),
    surface(surface)
{
    assert(physical_device != VK_NULL_HANDLE);
    assert(((requested_queue_flags & PresentQueue) == 0u) == (surface == VK_NULL_HANDLE));
    requested_queue_flags &= ~PresentQueue;

    std::uint32_t queue_family_count = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    std::uint32_t current_queue_index = 0u;
    for (const VkQueueFamilyProperties& queue_family : queue_families)
    {
        if (queue_family.queueCount > 0)
        {
            if (requested_queue_flags & queue_family.queueFlags & GraphicsQueue)
                queues_info.graphics_queue_family_index = current_queue_index;
            if (requested_queue_flags & queue_family.queueFlags & ComputeQueue)
                queues_info.compute_queue_family_index = current_queue_index;
            if (surface != VK_NULL_HANDLE)
            {
                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(
                    physical_device,
                    current_queue_index,
                    surface,
                    &present_support);
                if (present_support)
                {
                    queues_info.present_queue_family_index = current_queue_index;
                }
            }
        }
        ++current_queue_index;
    }

    if (queues_info.compute_queue_family_index == ~0u && (requested_queue_flags & ComputeQueue))
        throw Exception("Compute queue was requested, but it is not supported by the device");
    if (queues_info.graphics_queue_family_index == ~0u && (requested_queue_flags & GraphicsQueue))
        throw Exception("Graphics queue was requested, but it is not supported by the device");
    if (queues_info.present_queue_family_index == ~0u && surface != VK_NULL_HANDLE)
        throw Exception("Present queue was requested, but it is not supported by the device");

    std::unordered_set<std::uint32_t> queue_indices = {
        queues_info.compute_queue_family_index,
        queues_info.graphics_queue_family_index,
        queues_info.present_queue_family_index
    };
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    for (std::uint32_t queue_index : queue_indices)
    {
        if (queue_index == ~0u) continue;
        VkDeviceQueueCreateInfo queue_create_info = {};
        float queue_priority = 1.0f;
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_index;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = static_cast<std::uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures = &deviceFeatures;
    if (queues_info.present_queue_family_index != ~0u)
    {
        create_info.enabledExtensionCount = static_cast<std::uint32_t>(present_mode_extensions.size());
        create_info.ppEnabledExtensionNames = present_mode_extensions.data();
    }
    else
    {
        create_info.enabledExtensionCount = 0;
    }
    if (vk_instance.validation_layer_enabled)
    {
        create_info.enabledLayerCount = static_cast<std::uint32_t>(vk_instance.validation_layer_names.size());
        create_info.ppEnabledLayerNames = vk_instance.validation_layer_names.data();
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device, &create_info, nullptr, &handle) != VK_SUCCESS)
    {
        throw Exception("Failed to create logical device");
    }

    if (queues_info.graphics_queue_family_index != ~0u)
        vkGetDeviceQueue(handle, queues_info.graphics_queue_family_index, 0, &queues_info.graphics_queue);
    if (queues_info.present_queue_family_index != ~0u)
        vkGetDeviceQueue(handle, queues_info.present_queue_family_index, 0, &queues_info.present_queue);
    if (queues_info.compute_queue_family_index != ~0u)
        vkGetDeviceQueue(handle, queues_info.compute_queue_family_index, 0, &queues_info.compute_queue);

    if (queues_info.present_queue_family_index != ~0u)
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);
    }
}


Device::Device(Device&& other) :
    Object<VkDevice>(std::move(other)),
    physical_device(other.physical_device),
    surface(other.surface),
    present_modes(other.present_modes),
    surface_formats(std::move(other.surface_formats)),
    surface_capabilities(other.surface_capabilities),
    queues_info(other.queues_info)
{
}


VkPhysicalDevice Device::GetPhysicalDevice() const
{
    return physical_device;
}


VkSurfaceKHR Device::GetSurface() const
{
    return surface;
}


bool Device::Supports(const QueueType type) const
{
    switch (type)
    {
    case GraphicsQueue:
        return queues_info.graphics_queue != VK_NULL_HANDLE;
    case ComputeQueue:
        return queues_info.compute_queue != VK_NULL_HANDLE;
    case PresentQueue:
        return queues_info.present_queue != VK_NULL_HANDLE;
    default:
        assert(false);
        return false;
    }
}


VkQueue Device::GetQueue(const QueueType type) const
{
    assert(Supports(type));
    switch (type)
    {
    case GraphicsQueue:
        return queues_info.graphics_queue;
    case ComputeQueue:
        return queues_info.compute_queue;
    case PresentQueue:
        return queues_info.present_queue;
    default:
        assert(false);
        return VK_NULL_HANDLE;
    }
}


std::uint32_t Device::GetQueueFamilyIndex(const QueueType type) const
{
    assert(Supports(type));
    switch (type)
    {
    case GraphicsQueue:
        return queues_info.graphics_queue_family_index;
    case ComputeQueue:
        return queues_info.compute_queue_family_index;
    case PresentQueue:
        return queues_info.present_queue_family_index;
    default:
        assert(false);
        return InvalidQueueFamilyIndex;
    }
}


const VkSurfaceCapabilitiesKHR& Device::GetSurfaceCapabilities() const
{
    assert(Supports(PresentQueue));
    return surface_capabilities;
}


const std::vector<VkSurfaceFormatKHR>& Device::GetSurfaceFormats() const
{
    assert(Supports(PresentQueue));
    if (surface_formats.empty())
    {
        std::uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device,
            surface,
            &format_count,
            nullptr);
        if (format_count != 0)
        {
            surface_formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device,
                surface,
                &format_count,
                surface_formats.data());
        }
    }

    return surface_formats;
}


const std::vector<VkPresentModeKHR>& Device::GetPresentModes() const
{
    assert(Supports(PresentQueue));
    if (present_modes.empty())
    {
        std::uint32_t format_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &format_count, nullptr);
        if (format_count != 0)
        {
            present_modes.resize(format_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device,
                surface,
                &format_count,
                present_modes.data());
        }
    }

    return present_modes;
}


Device::~Device()
{
    if (handle != VK_NULL_HANDLE)
    {
        for (QueueType queue_type : AllQueueTypes())
        {
            if (Supports(queue_type)) vkQueueWaitIdle(GetQueue(queue_type));
        }
        vkDestroyDevice(handle, nullptr);
    }
}

}
}
