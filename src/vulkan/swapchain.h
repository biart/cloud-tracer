#pragma once


#include <vulkan/device.h>


namespace
{
    template <typename T>
    void PushBackUnique(std::vector<T>& container, const T& element)
    {
        if (std::find(container.begin(), container.end(), element) == container.end())
            container.push_back(element);
    }
}


namespace ct
{
    template <typename T>
    T Clamp(const T& value, const T& min, const T& max)
    {
        return std::max(std::min(value, max), min);
    }

    namespace vulkan
    {
        class Swapchain
        {
        public:
            Swapchain(const Device& device, const std::uint32_t width, const std::uint32_t height);

            const std::vector<VkImage>& GetImages() const;
            VkSurfaceFormatKHR GetSurfaceFormat() const;
            VkPresentModeKHR GetPresentMode() const;
            VkExtent2D GetExtent() const;

            ~Swapchain();

        private:
            const Device&           device;
            VkSwapchainKHR          vk_swapchain;
            std::vector<VkImage>    swapchain_images;
            VkSurfaceFormatKHR      surface_format;
            VkPresentModeKHR        present_mode;
            VkExtent2D              swapchain_extent;
        };
    }
}
