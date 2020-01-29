#include "swapchain.h"

#include <cassert>

#include <vulkan/device.h>
#include <vulkan/exception.h>
#include <vulkan/semaphore.h>


namespace ct
{
namespace vulkan
{

Swapchain::Swapchain(const Device& device, const std::uint32_t width, const std::uint32_t height) :
    device(device)
{
    if (!device.Supports(PresentQueue))
    {
        throw Exception("Cannot create swapchain for the device that does not support present mode");
    }

    const VkSurfaceCapabilitiesKHR& surface_capabilities = device.GetSurfaceCapabilities();
    const std::vector<VkSurfaceFormatKHR>& surface_formats = device.GetSurfaceFormats();
    const std::vector<VkPresentModeKHR>& present_modes = device.GetPresentModes();

    // Check if the suitable surface format is supported.
    auto surface_format_iter = std::find_if(surface_formats.cbegin(), surface_formats.cend(),
        [](const VkSurfaceFormatKHR& format)
    {
        return
            format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    });
    if (surface_format_iter == surface_formats.cend())
    {
        throw Exception("The requested surface format (B8G8R8A8_UNORM, SRGB_NONLINEAR) is not supported by the device");
    }
    surface_format = *surface_format_iter;

    // Determine the best available present mode.
    auto present_mode_iter =
        std::find(present_modes.cbegin(), present_modes.cend(), VK_PRESENT_MODE_MAILBOX_KHR);
    present_mode = (present_mode_iter == present_modes.cend()) ?
        VK_PRESENT_MODE_FIFO_KHR :
        VK_PRESENT_MODE_MAILBOX_KHR;

    // Compute supported swapchain image extents.
    swapchain_extent = surface_capabilities.currentExtent;
    if (swapchain_extent.width == std::numeric_limits<std::uint32_t>::max())
    {
        swapchain_extent =
        {
            Clamp(width,  surface_capabilities.minImageExtent.width,  surface_capabilities.maxImageExtent.width),
            Clamp(height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height)
        };
    }

    std::uint32_t image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0)
        image_count = std::min(surface_capabilities.maxImageCount, image_count);

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = device.GetSurface();

    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = swapchain_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    std::vector<std::uint32_t> unique_queue_family_indices; unique_queue_family_indices.reserve(AllQueueTypes().size());
    for (auto queue_type : AllQueueTypes())
    {
        if (device.Supports(queue_type))
            PushBackUnique(unique_queue_family_indices, device.GetQueueFamilyIndex(queue_type));
    }
    assert(unique_queue_family_indices.size() > 0);
    if (unique_queue_family_indices.size() > 1)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = static_cast<std::uint32_t>(unique_queue_family_indices.size());
        create_info.pQueueFamilyIndices = unique_queue_family_indices.data();
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device.GetHandle(), &create_info, nullptr, &handle) != VK_SUCCESS)
    {
        throw Exception("Failed to create swapchain");
    }

    vkGetSwapchainImagesKHR(device.GetHandle(), handle, &image_count, nullptr);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(device.GetHandle(), handle, &image_count, swapchain_images.data());
}

const std::vector<VkImage>& Swapchain::GetImages() const
{
    return swapchain_images;
}

VkSurfaceFormatKHR Swapchain::GetSurfaceFormat() const
{
    return surface_format;
}

VkPresentModeKHR Swapchain::GetPresentMode() const
{
    return present_mode;
}

VkExtent2D Swapchain::GetExtent() const
{
    return swapchain_extent;
}

std::uint32_t Swapchain::AcquireNextImageIndex(const Semaphore& semaphore)
{
    std::uint32_t image_index = ~0u;
    const VkResult status = vkAcquireNextImageKHR(
        device.GetHandle(),
        handle,
        std::numeric_limits<std::uint64_t>::max(),
        semaphore.GetHandle(),
        VK_NULL_HANDLE,
        &image_index);
    return image_index;
}

Swapchain::~Swapchain()
{
    vkDestroySwapchainKHR(device.GetHandle(), handle, nullptr);
}

}
}