#pragma once


#include <algorithm>
#include <vector>

#include <vulkan/object.h>


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
        class Device;
        class Semaphore;


        class Swapchain : public Object<VkSwapchainKHR>
        {
        public:
            explicit Swapchain(const Device& device, const std::uint32_t width, const std::uint32_t height);
            Swapchain(Swapchain&& swapchain);

            const std::vector<VkImage>& GetImages() const;
            VkSurfaceFormatKHR GetSurfaceFormat() const;
            VkPresentModeKHR GetPresentMode() const;
            VkExtent2D GetExtent() const;
            std::uint32_t AcquireNextImageIndex(const Semaphore& semaphore);

            ~Swapchain();

        private:
            const Device&           device;
            std::vector<VkImage>    images;
            VkSurfaceFormatKHR      surface_format;
            VkPresentModeKHR        present_mode;
            VkExtent2D              extent;
        };
    }
}
