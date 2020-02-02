#include "command_pool.h"


#include <vulkan/device.h>
#include <vulkan/exception.h>
#include <vulkan/synchronization.h>


namespace ct
{
namespace vulkan
{

CommandPool::CommandPool(const Device& device, QueueType queue_type) :
    device(device),
    queue_type(queue_type)
{
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.queueFamilyIndex = device.GetQueueFamilyIndex(queue_type);

    if (vkCreateCommandPool(device.GetHandle(), &pool_info, nullptr, &handle) != VK_SUCCESS)
        throw Exception("Failed to create command pool");
}

const Device& CommandPool::GetDevice() const
{
    return device;
}

QueueType CommandPool::GetQueueType() const
{
    return queue_type;
}

VkQueue CommandPool::GetQueue() const
{
    return device.GetQueue(queue_type);
}

void CommandPool::WaitForQueue() const
{
    if (vkQueueWaitIdle(GetQueue()) != VK_SUCCESS)
    {
        throw Exception("Failed to wait for queue");
    }
}

void CommandPool::Trim() const
{
    vkTrimCommandPool(device.GetHandle(), handle, 0u);
}

CommandPool::~CommandPool()
{
    vkDestroyCommandPool(device.GetHandle(), handle, nullptr);
}



CommandBuffer::CommandBuffer(const CommandPool& command_pool) :
    status(Initial),
    command_pool(command_pool)
{
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = command_pool.GetHandle();
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1u; // TODO: Allow allocating N instances at once
    if (vkAllocateCommandBuffers(command_pool.GetDevice().GetHandle(), &allocate_info, &handle) != VK_SUCCESS)
    {
        throw Exception("Failed to allocate command buffers");
    }
}

CommandBuffer::~CommandBuffer()
{
    if (handle != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(
            command_pool.GetDevice().GetHandle(),
            command_pool.GetHandle(),
            1u,
            &handle);
    }
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) :
    Object<VkCommandBuffer>(std::move(other)),
    command_pool(other.command_pool),
    status(other.status)
{
}

const CommandPool& CommandBuffer::GetPool() const
{
    return command_pool;
}

void CommandBuffer::Reset()
{
    assert(status != Pending);
    vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    status = Initial;
}

CommandBuffer::Status CommandBuffer::GetStatus() const
{
    return status;
}

void CommandBuffer::StartRecording()
{
    if (status == Initial)
    {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = 0;
        info.pInheritanceInfo = nullptr;
        if (vkBeginCommandBuffer(handle, &info) != VK_SUCCESS)
        {
            throw Exception("Failed to begin recording the command buffer");
        }
        status = Recording;
    }
    else
    {
        throw Exception("Failed to begin recording the command buffer: it has been already recorded");
    }
}

void CommandBuffer::StopRecording()
{
    if (status == Recording && vkEndCommandBuffer(handle) == VK_SUCCESS)
    {
        status = Executable;
    }
    else
    {
        assert(false);
        Reset();
    }
}



CommandRecorder::CommandRecorder(CommandBuffer& command_buffer) : command_buffer(command_buffer)
{
    command_buffer.StartRecording();
}

CommandRecorder::~CommandRecorder()
{
    command_buffer.StopRecording();
}
        

void DoSubmitCommands(
    VkSubmitInfo&           submit_info,
    const CommandBuffer&    command_buffer,
    const Semaphore*        signal_semaphore_ptr)
{
    if (command_buffer.GetStatus() != CommandBuffer::Executable)
    {
        throw Exception("Failed to submit command buffers for execution: they are not in an executable state");
    }
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.commandBufferCount = 1u;
    submit_info.pCommandBuffers = &(command_buffer.GetHandle());
    submit_info.pSignalSemaphores = (signal_semaphore_ptr == nullptr) ? nullptr : &signal_semaphore_ptr->GetHandle();
    submit_info.signalSemaphoreCount = (signal_semaphore_ptr == nullptr) ? 0u : 1u;
    VkQueue queue = command_buffer.GetPool().GetQueue();
    if (vkQueueSubmit(queue, 1u, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw Exception("Failed to submit command buffers for execution");
    }
}


void SubmitCommands(
    const CommandBuffer&    command_buffer,
    const Semaphore*        signal_semaphore_ptr)
{
    VkSubmitInfo submit_info = {};
    DoSubmitCommands(submit_info, command_buffer, signal_semaphore_ptr);
}


void SubmitCommands(
    const CommandBuffer&    command_buffer,
    const PipelineStageMask wait_stage_mask,
    const Semaphore&        wait_semaphore,
    const Semaphore*        signal_semaphore_ptr)
{
    VkSubmitInfo submit_info = {};
    submit_info.pWaitDstStageMask = &wait_stage_mask;
    submit_info.pWaitSemaphores = &wait_semaphore.GetHandle();
    submit_info.waitSemaphoreCount = 1u;
    DoSubmitCommands(submit_info, command_buffer, signal_semaphore_ptr);
}

}
}
