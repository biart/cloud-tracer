#include <application.h>


#include <utils/ignore_unused.h>



namespace ct
{
    class CloudTracerApplication : public Application
    {
    public:
        explicit CloudTracerApplication(const ct::vulkan::Instance& vk_instance) :
            Application(vk_instance, "Cloud Tracer") {}

    protected:
        virtual void Start() override
        {

        }

        virtual void Update() override
        {

        }

        virtual void Destroy() override
        {

        }
    };
}


VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT          message_severity,
    VkDebugUtilsMessageTypeFlagsEXT                 message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    ct::utils::IgnoreUnused(message_severity, message_type, user_data);

    std::cout << "Vulkan validation layer: " << callback_data->pMessage << std::endl;

    return VK_FALSE;
}


int main()
{
    glfwInit();
    std::uint32_t glfw_ext_count;
    const char** glfw_ext = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
    ct::vulkan::Instance vk_instance("Cloud Tracer", "", std::vector<const char*>(glfw_ext, glfw_ext + glfw_ext_count), true);
    ct::vulkan::DebugMessenger vk_debug_messenger(vk_instance, DebugCallback);

    ct::CloudTracerApplication application(vk_instance);

    try
    {
        application.Run();
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception occured: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}