#include "application.h"


#include <utils/ignore_unused.h>
#include <vulkan/command_pool.h>
#include <vulkan/memory.h>
#include <vulkan/synchronization.h>


namespace ct
{

Application::Application(const ct::vulkan::Instance& vk_instance, const std::string& name) :
    name(name),
    vk_instance(vk_instance),
    window(name, DefaultWidth, DefaultHeight),
    surface(vk_instance, window),
    vk_device(
        vk_instance,
        vk_instance.GetPhysicalDevices()[0],
        ct::vulkan::PresentQueue | ct::vulkan::ComputeQueue | ct::vulkan::GraphicsQueue,
        surface.GetHandler()),
    frame_number(0u),
    is_running(false)
{
}


void Application::Run()
{
    if (is_running)
        return;
    is_running = true;

    vulkan::Swapchain vk_swapchain(vk_device, DefaultWidth, DefaultHeight);

    vulkan::Semaphore vk_semaphores[2] = {
        vulkan::Semaphore(vk_device),
        vulkan::Semaphore(vk_device)
    };
    std::uint32_t next_semaphore_index = 0u;

    ct::vulkan::CommandPool command_pool(vk_device, ct::vulkan::GraphicsQueue);

    ct::vulkan::StagingBuffer<std::uint8_t> staging_buffer(vk_device, DefaultWidth * DefaultHeight * 4);
    {
        auto memory_map = ct::vulkan::MapMemory(staging_buffer);
        for (size_t i = 0; i < memory_map.GetCount(); i += 4)
        {
            memory_map[i] = 0xFF;
        }
    }

    std::vector<ct::vulkan::CommandBuffer> blit_command_buffers;
    const std::size_t images_count = vk_swapchain.GetImages().size();
    blit_command_buffers.reserve(images_count);
    for (size_t i = 0; i != images_count; ++i)
    {
        ct::vulkan::CommandBuffer initialize_image_layout_command_buffer(command_pool);
        {
            ct::vulkan::CommandRecorder recorder(initialize_image_layout_command_buffer);
            recorder.ImageMemoryBarrier(
                vk_swapchain.GetImages()[i],
                ct::vulkan::ImageLayout::Undefined, ct::vulkan::TopOfPipeStage,
                ct::vulkan::ImageLayout::PresentSource, ct::vulkan::BottomOfPipeStage);
        }

        ct::vulkan::SubmitCommands(initialize_image_layout_command_buffer);
        command_pool.WaitForQueue();

        ct::vulkan::CommandBuffer blit_command_buffer(command_pool);
        {
            ct::vulkan::CommandRecorder recorder(blit_command_buffer);
            recorder.Blit(
                staging_buffer,
                vk_swapchain.GetImages()[i],
                DefaultWidth, DefaultHeight);
        }
        blit_command_buffers.push_back(std::move(blit_command_buffer));
    }

    Start();
    while (!window.ShouldClose())
    {
        glfwPollEvents();
        Update();

        // Acquire next swapchain image index.
        std::uint32_t swapchain_image_index;
        {
            auto& signal_semaphore = vk_semaphores[next_semaphore_index];
            swapchain_image_index = vk_swapchain.AcquireNextImageIndex(signal_semaphore);
        }

        // Blit host buffer into the swapchain image.
        if (true) {
            auto& wait_semaphore = vk_semaphores[next_semaphore_index];
            next_semaphore_index = (next_semaphore_index + 1) % 2;
            auto& signal_semaphore = vk_semaphores[next_semaphore_index];
            vulkan::SubmitCommands(
                blit_command_buffers[swapchain_image_index],
                vulkan::ColorAttachmentOutputStage,
                wait_semaphore,
                &signal_semaphore);
        }

        // Present.
        {
            auto& wait_semaphore = vk_semaphores[next_semaphore_index];
            next_semaphore_index = (next_semaphore_index + 1) % 2;
            VkPresentInfoKHR present_info = {};
            present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = &wait_semaphore.GetHandle();
            present_info.swapchainCount = 1;
            present_info.pSwapchains = &vk_swapchain.GetHandle();
            present_info.pImageIndices = &swapchain_image_index;
            if (vkQueuePresentKHR(vk_device.GetQueue(vulkan::PresentQueue), &present_info) != VK_SUCCESS)
            {
                throw vulkan::Exception("Failed to present the frame");
            }
        }

        ++frame_number;
    }
    Destroy();
    is_running = false;
    frame_number = 0u;
}


const std::string& Application::GetName() const
{
    return name;
}

}
