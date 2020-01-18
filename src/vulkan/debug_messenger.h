#pragma once


#include <cassert>

#include <vulkan/vulkan.h>


namespace ct
{
namespace vulkan
{

class Instance;


class DebugMessenger
{
public:
    DebugMessenger(const Instance& vk_instance, PFN_vkDebugUtilsMessengerCallbackEXT callback);
    ~DebugMessenger();

private:
    VkDebugUtilsMessengerEXT    vk_debug_messenger;
    const Instance&             vk_instance;
};

}
}
