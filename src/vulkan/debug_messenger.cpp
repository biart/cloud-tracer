#include "debug_messenger.h"


namespace ct
{
namespace vulkan
{

DebugMessenger::DebugMessenger(const Instance& vk_instance, PFN_vkDebugUtilsMessengerCallbackEXT callback) :
    vk_instance(vk_instance)
{
    assert(vk_instance.validation_layer_enabled);

    VkDebugUtilsMessengerCreateInfoEXT createInfo;

    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = callback;

    if (vk_instance.CreateDebugUtilsMessenger(&createInfo, nullptr, &vk_debug_messenger) != VK_SUCCESS)
    {
        throw Exception("Failed to create debug messenger");
    }
}


DebugMessenger::~DebugMessenger()
{
    vk_instance.DestroyDebugUtilsMessenger(vk_debug_messenger, nullptr);
}

}
}
