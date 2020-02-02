#pragma once


#include <vulkan/object.h>
#include <algorithm>


namespace ct
{
    namespace vulkan
    {
        class Device;


        class Semaphore : public Object<VkSemaphore>
        {
        public:
            explicit Semaphore(const Device& device);
            Semaphore(Semaphore&& other);
            ~Semaphore();
        private:
            const Device& device;
        };
    }
}
