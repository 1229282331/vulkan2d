#pragma once

#include <memory>
#include <string>
#include <vector>
#include <array>
#include <optional>
#include <functional>
#include <chrono>

#include "vulkan/vulkan.hpp"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


#include "utils.hpp"
#include "app.hpp"
#include "swapchain.hpp"
#include "shader.hpp"
#include "render_process.hpp"
#include "command_manager.hpp"
#include "descriptor_manager.hpp"
#include "renderer.hpp"
#include "myMath.hpp"
#include "buffer.hpp"
#include "commander.hpp"

namespace vulkan2d{

constexpr int max_frames_in_flight = 2;

struct UniformBufferObject{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct QueueFamilyIndex{
        std::optional<uint32_t> graphicsIndex;
        std::optional<uint32_t> presentIndex;
        std::optional<uint32_t> computeIndex;
};

class VkBase{
public:
    friend void initial();

public:
    vk::Instance                         instance;
    vk::PhysicalDevice                   physicalDevice;
    vk::Device                           device;
    vk::Queue                            graphicsQueue;
    vk::Queue                            presentQueue;
    vk::Queue                            computeQueue;
    QueueFamilyIndex                     queueFamilyIndex;
    std::unique_ptr<Swapchain>           swapchain;
    std::unique_ptr<Shader>              shader;
    std::unique_ptr<RenderProcess>       renderProcess;
    std::unique_ptr<Buffer>              vertexBuffer;
    std::unique_ptr<Buffer>              indexBuffer;
    std::vector<std::unique_ptr<Buffer>> uniformBuffers;
    std::unique_ptr<CommandManager>      commandManager;
    std::unique_ptr<DescriptorManager>   descriptorManager;
    std::unique_ptr<Renderer>            renderer;

    static VkBase& self() { return *m_self_instance; }
    static void init(const std::vector<const char*>& extensions_, std::function<vk::SurfaceKHR(vk::Instance)> getSurfaceCallback);
    static void destroy();

    vk::DispatchLoaderDynamic loadInstanceDynamicLoader();
    void initSwapchain();
    void initShaderModules(const std::string& vertexFile, const std::string& fragmentFile);
    void initRenderProcess();
    void initPipeline();
    void initCommandManager();
    void initDescriptorManager();
    void initVertexBuffer();
    void initIndexBuffer();
    void initUniformBuffers();
    void updateUniformBuffers(int currentFrame);
    void initRenderer();
    void recreateSwapchain();

    void createTextureImage();
    vk::Image textureImage;
    vk::DeviceMemory textureImageMemory;


private:
    static VkBase* m_self_instance;  /*单例*/
    static bool    m_enableValidationLayer;

    std::vector<const char*>   m_extensions;
    std::vector<const char*>   m_layers;
    vk::DebugUtilsMessengerEXT m_debugMessenger;
    vk::SurfaceKHR             m_surface;
    

    std::function<vk::SurfaceKHR(vk::Instance)> m_getSurfaceCallback;

    VkBase(const std::vector<const char*>& extensions_, std::function<vk::SurfaceKHR(vk::Instance)> getSurfaceCallback);
    ~VkBase();


    vk::Instance createInstance();
    vk::DebugUtilsMessengerEXT createDebugMessenger();
    vk::PhysicalDevice pickPhysicalDevice();
    struct QueueFamilyIndex queryQueueFamilyIndex(bool enableGraphicsQueue=true, bool enablePresentQueue=true, bool enableComputQueue=false);
    vk::Device createLogicalDevice();

};

}