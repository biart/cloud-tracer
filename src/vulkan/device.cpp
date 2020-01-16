#include "device.h"

#include <unordered_set>


namespace ct
{
namespace vulkan
{

Device::Device(
    const Instance&             vk_instance,
    const VkPhysicalDevice      physical_device,
    VkQueueFlags                requested_queue_flags,
    const VkSurfaceKHR          surface) :
    physical_device(physical_device),
    surface(surface)
{
    assert(physical_device != VK_NULL_HANDLE);

    std::uint32_t queue_family_count = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    std::uint32_t current_queue_index = 0u;
    for (const VkQueueFamilyProperties& queue_family : queue_families)
    {
        if (queue_family.queueCount > 0)
        {
            if (requested_queue_flags & queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphics_queue_family_index = current_queue_index;
            if (requested_queue_flags & queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)
                compute_queue_family_index = current_queue_index;
            if (surface != VK_NULL_HANDLE)
            {
                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, current_queue_index, surface, &present_support);
                if (present_support) present_queue_family_index = current_queue_index;
            }
        }
        ++current_queue_index;
    }

    if (compute_queue_family_index == ~0u && (requested_queue_flags & VK_QUEUE_COMPUTE_BIT))
        throw Exception("Compute queue was requested, but it is not supported by the device");
    if (graphics_queue_family_index == ~0u && (requested_queue_flags & VK_QUEUE_COMPUTE_BIT))
        throw Exception("Graphics queue was requested, but it is not supported by the device");
    if (present_queue_family_index == ~0u && surface != VK_NULL_HANDLE)
        throw Exception("Present queue was requested, but it is not supported by the device");

    std::unordered_set<std::uint32_t> queue_indices = {
        compute_queue_family_index,
        graphics_queue_family_index,
        present_queue_family_index
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
    if (present_queue_family_index != ~0u)
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

    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS)
    {
        throw Exception("Failed to create logical device");
    }

    if (graphics_queue_family_index != ~0u) vkGetDeviceQueue(device, graphics_queue_family_index, 0, &graphics_queue);
    if (present_queue_family_index != ~0u) vkGetDeviceQueue(device, present_queue_family_index, 0, &present_queue);
    if (compute_queue_family_index != ~0u) vkGetDeviceQueue(device, compute_queue_family_index, 0, &compute_queue);

    if (present_queue_family_index != ~0u)
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);
    }
}


VkDevice Device::GetHandler() const
{
    return device;
}


VkPhysicalDevice Device::GetPhysicalDevice() const
{
    return physical_device;
}


VkSurfaceKHR Device::GetSurface() const
{
    return surface;
}


bool Device::SupportsGraphics() const
{
    return graphics_queue != VK_NULL_HANDLE;
}


VkQueue Device::GetGraphicsQueue() const
{
    assert(SupportsGraphics());
    return graphics_queue;
}


std::uint32_t Device::GetGraphicsQueueFamilyIndex() const
{
    assert(SupportsGraphics());
    return graphics_queue_family_index;
}


bool Device::SupportsPresent() const
{
    return present_queue != VK_NULL_HANDLE;
}


VkQueue Device::GetPresentQueue() const
{
    assert(SupportsPresent());
    return present_queue;
}


std::uint32_t Device::GetPresentQueueFamilyIndex() const
{
    assert(SupportsPresent());
    return present_queue_family_index;
}


bool Device::SupportsCompute() const
{
    return compute_queue != VK_NULL_HANDLE;
}


VkQueue Device::GetComputeQueue() const
{
    assert(SupportsCompute());
    return compute_queue;
}


std::uint32_t Device::GetComputeQueueFamilyIndex() const
{
    assert(SupportsCompute());
    return compute_queue_family_index;
}


const VkSurfaceCapabilitiesKHR& Device::GetSurfaceCapabilities() const
{
    assert(SupportsPresent());
    return surface_capabilities;
}


const std::vector<VkSurfaceFormatKHR>& Device::GetSurfaceFormats() const
{
    assert(SupportsPresent());
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
    assert(SupportsPresent());
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
    vkDestroyDevice(device, nullptr);
}

}
}
