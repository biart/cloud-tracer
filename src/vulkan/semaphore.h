#pragma once


#include <vulkan/object.h>


namespace ct
{
    namespace vulkan
    {
        class Device;


        class Semaphore : public Object<VkSemaphore>
        {
        public:
            explicit Semaphore(const Device& device);
            ~Semaphore();
        private:
            const Device& device;
        };
    }
}
