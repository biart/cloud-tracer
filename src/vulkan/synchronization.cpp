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
    if (handle != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(device.GetHandle(), handle, nullptr);
    }
}



Fence::Fence(const Device& device, const bool initially_signaled) : device(device)
{
    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = initially_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u;
    if (vkCreateFence(device.GetHandle(), &fence_info, nullptr, &handle) != VK_SUCCESS)
    {
        throw Exception("Failed to create fence");
    }
}

Fence::Fence(Fence&& other) :
    Object<VkFence>(std::move(other)),
    device(other.device)
{
}

Fence::~Fence()
{
    if (handle != VK_NULL_HANDLE)
    {
        vkDestroyFence(device.GetHandle(), handle, nullptr);
    }
}

bool Fence::IsSignaled() const
{
    const auto status = vkGetFenceStatus(device.GetHandle(), handle);
    if (status == VK_ERROR_DEVICE_LOST)
    {
        throw Exception("Device has been lost");
    }
    return status == VK_SUCCESS;
}

bool Fence::WaitOnce(const float timeout) const
{
    if (!IsSignaled())
    {
        const auto timeout_microseconds = static_cast<uint64_t>(1.0e+9 * timeout);
        const auto status = vkWaitForFences(device.GetHandle(), 1u, &handle, true, timeout);
        switch (status)
        {
        case VK_TIMEOUT:
            return false;
        case VK_SUCCESS:
            return true;
        default:
            throw Exception("Failed to wait for a fence");
        }
    }
    return true;
}

void Fence::Wait() const
{
    do {} while (!WaitOnce(1.0f));
}

void Fence::Reset()
{
    vkResetFences(device.GetHandle(), 1u, &handle);
}

} // namespace vulkan
} // namespace ct
