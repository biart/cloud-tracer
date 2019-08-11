#include <algorithm>
#include <cassert>
#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "vulkan/exception.h"


namespace ct
{
namespace vulkan
{

class Instance
{
public:
    Instance(
        const std::string&                  app_name,
        const std::string&                  eng_name,
        std::vector<const char*>            extensions,
        const bool                          enable_validation_layer);

    ~Instance();

    Instance(const Instance& other) = delete;
    Instance(Instance&& other) = delete;

    VkInstance GetHandler() const;

    const bool validation_layer_enabled;

private:
    template <typename Proc>
    struct GetProcAddrHelperNothrow
    {
    public:
        explicit GetProcAddrHelperNothrow(const char* name, const VkInstance& vk_instance) :
            name(name), vk_instance(vk_instance) {}

        template <typename ...Args>
        void operator()(Args... args) const
        {
            if (proc == nullptr)
                proc = (Proc)vkGetInstanceProcAddr(vk_instance, name);
            if (proc != nullptr)
                proc(vk_instance, args...);
        }

    private:
        const char* name;
        mutable Proc proc = nullptr;
        const VkInstance& vk_instance;
    };

    template <typename Proc>
    struct GetProcAddrHelper
    {
    public:
        explicit GetProcAddrHelper(const char* name, const VkInstance& vk_instance) :
            name(name), vk_instance(vk_instance) {}

        template <typename ...Args>
        auto operator()(Args... args) const
        {
            if (proc == nullptr)
                proc = (Proc)vkGetInstanceProcAddr(vk_instance, name);
            if (proc == nullptr)
                throw Exception(std::string("Unable to get ") + name + " procedure address");
            return proc(vk_instance, args...);
        }

    private:
        const char* name;
        mutable Proc proc = nullptr;
        const VkInstance& vk_instance;
    };

    bool CheckValidationLayerSupport();

    VkInstance      vk_instance;

    mutable PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
    mutable PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;

    const char* validation_layer_names[1] =
    {
        "VK_LAYER_KHRONOS_validation"
    };

public:
    GetProcAddrHelper<PFN_vkCreateDebugUtilsMessengerEXT> CreateDebugUtilsMessenger =
        GetProcAddrHelper<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT", vk_instance);
    GetProcAddrHelper<PFN_vkDestroyDebugUtilsMessengerEXT> DestroyDebugUtilsMessenger =
        GetProcAddrHelper<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT", vk_instance);
};

}
}
