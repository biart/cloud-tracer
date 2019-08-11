#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace ct
{

class Window
{
public:
    Window(const std::string& name, int default_width, int default_height);

    bool ShouldClose() const;

    ~Window();

private:
    GLFWwindow* window;
};

}
