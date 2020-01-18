#pragma once


#include <algorithm>
#include <array>
#include <cassert>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan/exception.h>


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

    const std::vector<VkPhysicalDevice>& GetPhysicalDevices() const
    {
        if (physical_devices.empty())
        {
            uint32_t device_count = 0;
            vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr);
            physical_devices.resize(device_count);
            vkEnumeratePhysicalDevices(vk_instance, &device_count, physical_devices.data());
        }

        return physical_devices;
    }

    const bool validation_layer_enabled;
    const std::array<const char*, 1> validation_layer_names =
    {
        "VK_LAYER_KHRONOS_validation"
    };

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

    mutable std::vector<VkPhysicalDevice> physical_devices;

    mutable PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
    mutable PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;

public:
    GetProcAddrHelper<PFN_vkCreateDebugUtilsMessengerEXT> CreateDebugUtilsMessenger =
        GetProcAddrHelper<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT", vk_instance);
    GetProcAddrHelper<PFN_vkDestroyDebugUtilsMessengerEXT> DestroyDebugUtilsMessenger =
        GetProcAddrHelper<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT", vk_instance);
};

}
}
