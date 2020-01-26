#pragma once


#include <cassert>

#include <vulkan/vulkan.h>


namespace ct
{
    namespace vulkan
    {
        template <typename T>
        class Object
        {
        public:
            const T& GetHandle() const
            {
                assert(handle != VK_NULL_HANDLE);
                return handle;
            }
        protected:
            T handle = VK_NULL_HANDLE;
        };


        template <typename T, typename Delete>
        class Scoped : public Object<T>
        {
        public:
            Scoped() : T(VK_NULL_HANDLE)
            {
            }

            template <typename D>
            Scoped(const T handle, D&& delete_func) :
                delete_func(std::forward<D>(delete_func))
            {
                Object<T>::handle = handle;
            }

            Scoped(const Scoped& other) = delete;
            Scoped& operator=(const Scoped& other) = delete;
            Scoped(Scoped&& other) = default;
            Scoped& operator=(Scoped&& other) = default;

            void Release()
            {
                handle = VK_NULL_HANDLE;
            }

            ~Scoped()
            {
                if (handle != VK_NULL_HANDLE)
                {
                    delete_func(handle);
                }
            }

        protected:
            Delete  delete_func;
        };


        template <typename T, typename Delete>
        auto MakeScoped(const T handle, Delete&& delete_func)
        {
            return Scoped<T, Delete>(handle, std::forward<Delete>(delete_func));
        }
    }
}