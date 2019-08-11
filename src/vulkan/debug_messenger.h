#pragma once

#include <cassert>

#include "vulkan/exception.h"
#include "vulkan/instance.h"


namespace ct
{
namespace vulkan
{

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
