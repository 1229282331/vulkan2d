#include "app.hpp"


namespace vulkan2d{

void initial()
{
    /*初始化glfw*/
    Window::init();

    /*获取支持的vulkan拓展*/
    uint32_t extensionCount = 0;
    const char** extensions_c = nullptr;
    extensions_c = glfwGetRequiredInstanceExtensions(&extensionCount);
    std::vector<const char*> extensions(extensions_c, extensions_c+extensionCount);

    /*初始化vulkan*/ 
    auto getSurfaceCallback = [](vk::Instance instance) -> vk::SurfaceKHR
    {
        VkSurfaceKHR surface;
        glfwCreateWindowSurface(instance, Window::self().window, nullptr, &surface);
        return vk::SurfaceKHR(surface);
    };
    VkBase::init(extensions, getSurfaceCallback);

    /*初始化VkBase实例的交换链*/
    VkBase::self().initSwapchain();

    /*初始化着色器模组*/
    VkBase::self().initShaderModules("C:/VSCode_files/vulkan2D/shader/generated/shader.vert.spv", "C:/VSCode_files/vulkan2D/shader/generated/shader.frag.spv");
  
    /*初始化渲染流程*/
    VkBase::self().initRenderProcess();

    /*初始化渲染管线*/
    VkBase::self().initPipeline();

    /*初始化framebuffer*/
    VkBase::self().swapchain->initFramebuffers();

    /*初始化命令池*/
    VkBase::self().initCommandManager();

    /*初始化描述符集池*/
    VkBase::self().initDescriptorManager();

    VkBase::self().createTextureImage();

    /*初始化顶点缓冲*/
    VkBase::self().initVertexBuffer();

    /*初始化顶点索引缓冲*/
    VkBase::self().initIndexBuffer();

    /*初始化uniform缓冲*/
    VkBase::self().initUniformBuffers();

    /****初始化渲染器****/
    VkBase::self().initRenderer();

    /*更新并绑定描述符缓冲*/
    VkBase::self().renderer->updateDescriptorSets(VkBase::self().uniformBuffers);

}

void run()
{
    while(!glfwWindowShouldClose(Window::self().window))
    {
        glfwPollEvents();
        VkBase::self().renderer->drawFrame();
        Window::self().titleFPS();
    }

    VkBase::self().device.waitIdle();
}

void cleanup()
{
    VkBase::destroy();
    Window::destroy();
}


}