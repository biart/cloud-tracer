#include "window.h"


namespace ct
{

Window::Window(const std::string& name, int default_width, int default_height)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(default_width, default_height, name.c_str(), nullptr, nullptr);
    if (window == nullptr)
    {
        throw std::runtime_error("Unable to create window");
    }
}


bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(window);
}


Window::~Window()
{
    glfwDestroyWindow(window);
}

}