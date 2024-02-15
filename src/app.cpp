#include "app.hpp"

namespace vulkan2d{

void initial(bool enableValidationLayer)
{
    /*初始化glfw*/
    Window::init();

    /*获取支持的vulkan拓展*/
    uint32_t extensionCount = 0;
    const char** extensions_c = nullptr;
    extensions_c = glfwGetRequiredInstanceExtensions(&extensionCount);
    std::vector<const char*> extensions(extensions_c, extensions_c+extensionCount);

    /*初始化vulkan*/
    VkBase::init(extensions);
    std::cout<< "vkInstance extension list: " << VkBase::self().extensions << std::endl;
    std::cout<< "vkInstance layers list: " << VkBase::self().layers << std::endl;
}

void run()
{
    while(!glfwWindowShouldClose(Window::self().window))
    {
        glfwPollEvents();
    }
}

void cleanup()
{
    VkBase::destroy();
    Window::destroy();
}


}