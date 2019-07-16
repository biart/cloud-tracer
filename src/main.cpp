#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


class Window
{

};


class Application
{
public:
    Application(const std::string& name) :
        window(nullptr),
        name(name) 
    {
    }

    void Run()
    {
        if (IsRunning())
            return;

        glfwInit();

        CreateWindow();
        InitializeVulkan();

        Start();
        while (!glfwWindowShouldClose(window))
        {
            Update();
            glfwPollEvents();
        }
        Destroy();

        vkDestroyInstance(vk_instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
        window = nullptr;
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

    void CreateWindow()
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(DefaultWidth, DefaultHeight, name.c_str(), nullptr, nullptr);
        if (window == nullptr)
        {
            glfwTerminate();
            throw std::exception("Unable to create window");
        }
    }

    void InitializeVulkan()
    {

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Cloud Tracer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
        appInfo.pEngineName = "Cloud Tracer";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &vk_instance) != VK_SUCCESS)
        {
            glfwDestroyWindow(window);
            glfwTerminate();
            window = nullptr;
            throw std::runtime_error("Failed to create Vulkan instance");
        }
    }

    std::string     name;
    GLFWwindow*     window;
    VkInstance      vk_instance;
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

int main()
{
    CloudTracerApplication application;

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
