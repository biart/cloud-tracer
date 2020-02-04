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


        class Fence : public Object<VkFence>
        {
        public:
            explicit Fence(const Device& device, const bool initially_signaled = false);
            Fence(Fence&& other);
            ~Fence();

            bool IsSignaled() const;
            bool WaitOnce(const float timeout) const;
            void Wait() const;
            void Reset();
        private:
            const Device& device;
        };
    }
}
