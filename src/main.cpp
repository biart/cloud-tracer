#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace ct
{
    namespace vulkan
    {
        class Exception : public std::runtime_error
        {
        public:
            explicit Exception(const char* msg) :
                std::runtime_error(msg)
            {}

            explicit Exception(const std::string& msg) :
                std::runtime_error(msg)
            {}
        };

        class Instance
        {
        public:
            Instance(const std::string& app_name, const std::string& eng_name, bool enable_validation_layer) :
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

                auto extensions = GetRequiredExtensions();
                createInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
                createInfo.ppEnabledExtensionNames = extensions.data();
                VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
                createInfo.enabledLayerCount = 0;
                createInfo.pNext = nullptr;
                if (vkCreateInstance(&createInfo, nullptr, &vk_instance) != VK_SUCCESS)
                {
                    throw Exception("vkCreateInstance failed");
                }
            }

            ~Instance()
            {
                vkDestroyInstance(vk_instance, nullptr);
            }

            Instance(const Instance& other) = delete;
            Instance(Instance&& other) = delete;

            VkInstance GetHandler() const
            {
                return vk_instance;
            }

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

            bool CheckValidationLayerSupport()
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

            std::vector<const char*> GetRequiredExtensions()
            {
                uint32_t glfw_extensions_count = 0;
                const char** glfw_extensions;
                glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);
                std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extensions_count);
                if (validation_layer_enabled)
                    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                return extensions;
            }

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


        class DebugMessenger
        {
        public:
            DebugMessenger(const Instance& vk_instance, PFN_vkDebugUtilsMessengerCallbackEXT callback) :
                vk_instance(vk_instance)
            {
                assert(vk_instance.validation_layer_enabled);

                VkDebugUtilsMessengerCreateInfoEXT createInfo;

                createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                createInfo.messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                createInfo.pfnUserCallback = callback;

                if (vk_instance.CreateDebugUtilsMessenger(&createInfo, nullptr, &vk_debug_messenger) != VK_SUCCESS)
                {
                    throw Exception("Failed to create debug messenger");
                }
            }

            ~DebugMessenger()
            {
                vk_instance.DestroyDebugUtilsMessenger(vk_debug_messenger, nullptr);
            }

        private:
            VkDebugUtilsMessengerEXT    vk_debug_messenger;
            const Instance&             vk_instance;
        };
    };

    class Window
    {
    public:
        Window(const std::string& name, int default_width, int default_height)
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            window = glfwCreateWindow(default_width, default_height, name.c_str(), nullptr, nullptr);
            if (window == nullptr)
            {
                throw std::runtime_error("Unable to create window");
            }
        }

        bool ShouldClose() const
        {
            return glfwWindowShouldClose(window);
        }

        ~Window()
        {
            glfwDestroyWindow(window);
        }

    private:
        GLFWwindow* window;
    };

    class Application
    {
    public:
        Application(const std::string& name, const bool debug = true) :
            name(name), debug(debug)
        {
        }

        void Run()
        {
            if (IsRunning())
                return;

            glfwInit();
            window.reset(new Window(name, DefaultWidth, DefaultHeight));
            vk_instance.reset(new vulkan::Instance(name, "", debug));
            vk_debug_messenger.reset(debug ? new vulkan::DebugMessenger(*vk_instance, DebugCallback) : nullptr);

            Start();
            while (!window->ShouldClose())
            {
                Update();
                glfwPollEvents();
            }
            Destroy();
        }

        bool IsRunning()
        {
            return window != nullptr;
        }

        const std::string& GetName() const
        {
            return name;
        }

        enum
        {
            DefaultWidth = 1024,
            DefaultHeight = 768,
        };

    protected:
        virtual void Start() = 0;
        virtual void Update() = 0;
        virtual void Destroy() = 0;

    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT          message_severity,
            VkDebugUtilsMessageTypeFlagsEXT                 message_type,
            const VkDebugUtilsMessengerCallbackDataEXT*     callback_data,
            void* user_data)
        {
            std::cout << "Vulkan validation layer: " << callback_data->pMessage << std::endl;

            return VK_FALSE;
        }

        const std::string                           name;
        const bool                                  debug;
        std::unique_ptr<vulkan::Instance>           vk_instance;
        std::unique_ptr<vulkan::DebugMessenger>     vk_debug_messenger;
        std::unique_ptr<Window>                     window;
    };


    class CloudTracerApplication : public Application
    {
    public:
        CloudTracerApplication() : Application("Cloud Tracer") {}

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

int main()
{
    ct::CloudTracerApplication application;

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