#include "application.h"


#include <utils/ignore_unused.h>
#include <vulkan/command_pool.h>
#include <vulkan/memory.h>
#include <vulkan/semaphore.h>


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

    ct::vulkan::CommandPool command_pool(vk_device, ct::vulkan::ComputeQueue);

    std::vector<ct::vulkan::CommandBuffer> blit_command_buffers;
    const std::size_t images_count = vk_swapchain.GetImages().size();
    blit_command_buffers.reserve(images_count);
    for (size_t i = 0; i != images_count; ++i)
    {
        ct::vulkan::CommandBuffer command_buffer(command_pool);
        ct::vulkan::CommandRecorder recorder(command_buffer);
        /*
        recorder.Blit(
            staging_buffer, vk_swapchain->GetImages()[i],
            vk_swapchain->GetExtent().width, vk_swapchain->GetExtent().height);
        blit_command_buffers.push_back(std::move(command_buffer));
        */
    }

    Start();
    while (!window.ShouldClose())
    {
        glfwPollEvents();
        Update();

        const std::uint32_t swapchain_image_index = vk_swapchain.AcquireNextImageIndex(vk_semaphores[frame_number % 2u]);

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &vk_semaphores[(frame_number + 1u) % 2u].GetHandle();
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &vk_swapchain.GetHandle();
        present_info.pImageIndices = &swapchain_image_index;
        if (vkQueuePresentKHR(vk_device.GetQueue(vulkan::PresentQueue), &present_info) != VK_SUCCESS)
        {
            throw vulkan::Exception("Unable to present");
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
