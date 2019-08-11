#include "instance.h"


namespace ct
{
namespace vulkan
{

Instance::Instance(
    const std::string&                  app_name,
    const std::string&                  eng_name,
    std::vector<const char*>            extensions,
    const bool                          enable_validation_layer) :
    validation_layer_enabled(enable_validation_layer)
{
    if (validation_layer_enabled && !CheckValidationLayerSupport())
        throw Exception("Validation layer is not supported");

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = app_name.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = eng_name.c_str();
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (validation_layer_enabled)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    createInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
    if (vkCreateInstance(&createInfo, nullptr, &vk_instance) != VK_SUCCESS)
    {
        throw Exception("vkCreateInstance failed");
    }
}


Instance::~Instance()
{
    vkDestroyInstance(vk_instance, nullptr);
}


VkInstance Instance::GetHandler() const
{
    return vk_instance;
}


bool Instance::CheckValidationLayerSupport()
{
    uint32_t layers_count;
    vkEnumerateInstanceLayerProperties(&layers_count, nullptr);
    std::vector<VkLayerProperties> available_layers(layers_count);
    vkEnumerateInstanceLayerProperties(&layers_count, available_layers.data());

    for (const char* layer_name : validation_layer_names)
    {
        const auto it = std::find_if(available_layers.cbegin(), available_layers.cend(),
            [layer_name](const VkLayerProperties& p)
        {
            return std::strcmp(layer_name, p.layerName) == 0;
        });

        if (it == available_layers.cend()) {
            return false;
        }
    }

    return true;
}

}
}
