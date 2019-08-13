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
    const VkSurfaceKHR          surface)
{
    assert(physical_device != VK_NULL_HANDLE);

    std::uint32_t queue_family_count = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    std::uint32_t current_queue_index = 0u;
    std::uint32_t graphics_queue_index = ~0u;
    std::uint32_t compute_queue_index = ~0u;
    std::uint32_t present_queue_index = ~0u;
    for (const VkQueueFamilyProperties& queue_family : queue_families)
    {
        if (queue_family.queueCount > 0)
        {
            if (requested_queue_flags & queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphics_queue_index = current_queue_index;
            if (requested_queue_flags & queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)
                compute_queue_index = current_queue_index;
            if (surface != VK_NULL_HANDLE)
            {
                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, current_queue_index, surface, &present_support);
                if (present_support) present_queue_index = current_queue_index;
            }
        }
        ++current_queue_index;
    }

    if (compute_queue_index == ~0u && (requested_queue_flags & VK_QUEUE_COMPUTE_BIT))
        throw Exception("Compute queue was requested, but it is not supported by the device");
    if (graphics_queue_index == ~0u && (requested_queue_flags & VK_QUEUE_COMPUTE_BIT))
        throw Exception("Graphics queue was requested, but it is not supported by the device");
    if (present_queue_index == ~0u && surface != VK_NULL_HANDLE)
        throw Exception("Present queue was requested, but it is not supported by the device");

    std::unordered_set<std::uint32_t> queue_indices = { compute_queue_index, graphics_queue_index, present_queue_index };
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
    create_info.enabledExtensionCount = 0;
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

    if (graphics_queue_index != ~0u) vkGetDeviceQueue(device, graphics_queue_index, 0, &graphics_queue);
    if (present_queue_index != ~0u) vkGetDeviceQueue(device, present_queue_index, 0, &present_queue);
    if (compute_queue_index != ~0u) vkGetDeviceQueue(device, compute_queue_index, 0, &compute_queue);
}


VkDevice Device::GetHandler() const
{
    return device;
}


VkQueue Device::GetGraphicsQueue() const
{
    assert(graphics_queue != VK_NULL_HANDLE);
    return graphics_queue;
}


VkQueue Device::GetPresentQueue() const
{
    assert(present_queue != VK_NULL_HANDLE);
    return present_queue;
}


VkQueue Device::GetComputeQueue() const
{
    assert(compute_queue != VK_NULL_HANDLE);
    return compute_queue;
}


Device::~Device()
{
    vkDestroyDevice(device, nullptr);
}

}
}
