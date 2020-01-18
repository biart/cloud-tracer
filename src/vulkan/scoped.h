#pragma once


#include <memory>


namespace ct
{
    namespace vulkan
    {
        template <typename T, typename Delete>
        class Scoped
        {
        public:
            Scoped() : T(VK_NULL_HANDLE)
            {
            }

            template <typename D>
            Scoped(const T handle, D&& delete_func) :
                handle(handle),
                delete_func(std::forward<D>(delete_func))
            {
            }

            Scoped(const Scoped& other) = delete;
            Scoped& operator=(const Scoped& other) = delete;
            Scoped(Scoped&& other) = default;
            Scoped& operator=(Scoped&& other) = default;

            T GetHandle()
            {
                return handle;
            }

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

        private:
            T       handle;
            Delete  delete_func;
        };

        template <typename T, typename Delete>
        auto MakeScoped(const T handle, Delete&& delete_func)
        {
            return Scoped<T, Delete>(handle, std::forward<Delete>(delete_func));
        }
    }
}