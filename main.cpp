#include <stdexcept>
#include "vulkan/vulkan.hpp"

#include "vkBase.hpp"
#include "app.hpp"


int main()
{
    vulkan2d::initial();

    try
    {
        vulkan2d::run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }


    vulkan2d::cleanup();
    return 0;
}