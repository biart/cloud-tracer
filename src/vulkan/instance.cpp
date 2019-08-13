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

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = eng_name.c_str();
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    if (validation_layer_enabled)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    create_info.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();
    create_info.enabledLayerCount = 0;
    create_info.pNext = nullptr;
    if (vkCreateInstance(&create_info, nullptr, &vk_instance) != VK_SUCCESS)
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
