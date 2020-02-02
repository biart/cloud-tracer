#pragma once


#include "synchronization.h"


#include <vulkan/device.h>
#include <vulkan/exception.h>


namespace ct
{
namespace vulkan
{

Semaphore::Semaphore(const Device& device) : device(device)
{
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(device.GetHandle(), &semaphore_info, nullptr, &handle) != VK_SUCCESS)
    {
        throw Exception("Failed to create semaphore");
    }
}

Semaphore::Semaphore(Semaphore&& other) :
    Object<VkSemaphore>(std::move(other)),
    device(other.device)
{
}

Semaphore::~Semaphore()
{
    vkDestroySemaphore(device.GetHandle(), handle, nullptr);
}

} // namespace vulkan
} // namespace ct
