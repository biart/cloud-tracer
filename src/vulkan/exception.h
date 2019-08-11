#pragma once

#include <stdexcept>


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

}
}
