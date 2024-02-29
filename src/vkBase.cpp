#include "vkBase.hpp"
#define STB_IMAGE_IMPLEMENTATION       
#include "stb_image.h"

namespace vulkan2d{

VkBase* VkBase::m_self_instance = nullptr;
#ifdef NDEBUG
    bool VkBase::m_enableValidationLayer = false;
#else
    bool VkBase::m_enableValidationLayer = true;
#endif

void VkBase::init(const std::vector<const char*>& extensions_, std::function<vk::SurfaceKHR(vk::Instance)> getSurfaceCallback)
{
    m_self_instance = new VkBase(extensions_, getSurfaceCallback);
}

VkBase::VkBase(const std::vector<const char*>& extensions_, std::function<vk::SurfaceKHR(vk::Instance)> getSurfaceCallback)
{
    m_getSurfaceCallback = getSurfaceCallback;
    m_extensions = extensions_;
    if(m_enableValidationLayer)
    {
        m_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        m_layers.push_back("VK_LAYER_KHRONOS_validation");
    }
    /*1.创建vulkan实例*/
    instance = createInstance();
    if(!instance)
        throw std::runtime_error("[ Instance ]: Can't create vulkan instance!");
#ifndef NDEBUG
    /*2.创建debuger*/
    m_debugMessenger = createDebugMessenger();
    if(!m_debugMessenger)
        throw std::runtime_error("[ DebugMessenger ]: Can't create  debug messenger!");
#endif
    /*3.获取物理设备*/
    physicalDevice = pickPhysicalDevice();
    if(!physicalDevice)
        throw std::runtime_error("[ PhysicalDevice ]: Can't create  physical device!");
    /*4.创建逻辑设备*/
    device = createLogicalDevice();
    if(!device)
        throw std::runtime_error("[ LogicalDevice ]: Can't create  logical device!");
    /*5.获取设备队列*/
    if(queueFamilyIndex.graphicsIndex.has_value())
        graphicsQueue = device.getQueue(queueFamilyIndex.graphicsIndex.value(), 0);
    if(queueFamilyIndex.presentIndex.has_value())
        presentQueue = device.getQueue(queueFamilyIndex.presentIndex.value(), 0);
    if(queueFamilyIndex.computeIndex.has_value())
        computeQueue = device.getQueue(queueFamilyIndex.computeIndex.value(), 0);
}

void VkBase::destroy()
{
    delete m_self_instance;
}

VkBase::~VkBase()
{
    for(int i=0; i<renderer->getFlightCount(); i++)
        uniformBuffers[i].reset();
    renderer.reset();
    indexBuffer.reset();
    vertexBuffer.reset();
    commandManager.reset();
    device.destroyPipeline(renderProcess->graphicsPipeline_triangle);
    renderProcess.reset();
    shader.reset();
    swapchain.reset();
    descriptorManager.reset();
    device.destroy();
#ifndef NDEBUG
    instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, loadInstanceDynamicLoader());
#endif
    instance.destroy();
}

vk::Instance VkBase::createInstance()
{
    vk::ApplicationInfo appInfo = {};
    appInfo.setPEngineName("vulkan2D")
           .setPNext(nullptr)
           .setPApplicationName("vulkan2D")
           .setApplicationVersion(VK_VERSION_1_0)
           .setEngineVersion(VK_VERSION_1_0)
           .setApiVersion(VK_API_VERSION_1_3);

    vk::InstanceCreateInfo createInfo = {};
    createInfo.setPNext(nullptr)
              .setEnabledExtensionCount(uint32_t(m_extensions.size()))
              .setPEnabledExtensionNames(m_extensions)
              .setEnabledLayerCount(m_layers.size())
              .setPEnabledLayerNames(m_layers)
              .setPApplicationInfo(&appInfo);

    return vk::createInstance(createInfo);
}

vk::DispatchLoaderDynamic VkBase::loadInstanceDynamicLoader()
{
    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr pGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    if(pGetInstanceProcAddr==nullptr)
    {
        std::cout << "[ Kernel ]: Failed to load dynamic instance functions!" << std::endl;
        abort();
    }
    vk::DispatchLoaderDynamic dispatcher(instance, pGetInstanceProcAddr);

    return dispatcher;
}

vk::DebugUtilsMessengerEXT VkBase::createDebugMessenger()
{
    PFN_vkDebugUtilsMessengerCallbackEXT debugCallback = [] (
                VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity, 
                VkDebugUtilsMessageTypeFlagsEXT             messageType, 
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                void*                                       pUserData) -> vk::Bool32 
                { std::cerr<<"validation layer: "<<pCallbackData->pMessage<<std::endl; return VK_FALSE;};

    vk::DebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.setPNext(nullptr)
              .setPUserData(nullptr)
              .setPfnUserCallback(debugCallback)
              .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
              .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    
    /*动态加载vkCreateDebugUtilsMessengerEXT*/
    return instance.createDebugUtilsMessengerEXT(createInfo, nullptr, loadInstanceDynamicLoader());
}

vk::PhysicalDevice VkBase::pickPhysicalDevice()
{
    /*选择独立显卡且支持几何着色器*/
    auto isDeviceSuitableFunc = [](vk::PhysicalDevice device) -> bool
    {
        vk::PhysicalDeviceProperties properties = device.getProperties();
        vk::PhysicalDeviceFeatures features = device.getFeatures();
        std::cout << "physical-device: " << properties.deviceName << std::endl;
        return (properties.deviceType==vk::PhysicalDeviceType::eDiscreteGpu) && features.geometryShader;
    };

    std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
    for(auto dev: physicalDevices)
    {
        if(isDeviceSuitableFunc(dev))
        {
            return physicalDevices[0];
        }
    }
    
    return nullptr;
}

struct QueueFamilyIndex VkBase::queryQueueFamilyIndex(bool enableGraphicsQueue, bool enablePresentQueue, bool enableComputQueue)
{
    struct QueueFamilyIndex res;
    std::vector<vk::QueueFamilyProperties> qfProperties = physicalDevice.getQueueFamilyProperties();
    for(int i=0; i<qfProperties.size(); i++)
    {
        if(enableGraphicsQueue && (qfProperties[i].queueFlags&vk::QueueFlagBits::eGraphics))
            res.graphicsIndex = i;
        if(enablePresentQueue && physicalDevice.getSurfaceSupportKHR(i, m_surface))
            res.presentIndex = i;
        if(enableComputQueue && (qfProperties[i].queueFlags&vk::QueueFlagBits::eCompute))
            res.computeIndex = i;

        if( !((!res.graphicsIndex.has_value())&&enableGraphicsQueue ||
             (!res.presentIndex.has_value())&&enablePresentQueue   ||
             (!res.computeIndex.has_value())&&enableComputQueue) )
            break;
    }

    if((!res.graphicsIndex.has_value())&&enableGraphicsQueue || (!res.presentIndex.has_value())&&enablePresentQueue || (!res.computeIndex.has_value())&&enableComputQueue) 
    {
        std::cout << "ERROR: The physical device doesn't support graphics/present/compute queue-family!" << std::endl;   
        abort();
    }
    return res;
}

vk::Device VkBase::createLogicalDevice()
{
    /*1.获取显示窗口对应的surface*/
    m_surface = m_getSurfaceCallback(instance);
    if(!m_surface)
        throw std::runtime_error("[ Surface ]: Can't create  surface from external API !");

    /*2.指定逻辑设备所需队列*/
    queueFamilyIndex = queryQueueFamilyIndex(true, true, false);
    std::vector<float> queuePriorties = {1.f, 1.f, 1.f};
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    if(queueFamilyIndex.graphicsIndex.has_value())
    {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setPNext(nullptr)
                       .setPQueuePriorities(&queuePriorties[0])
                       .setQueueCount(1)
                       .setQueueFamilyIndex(queueFamilyIndex.graphicsIndex.value());
        queueCreateInfos.push_back(queueCreateInfo);
    }
    if(queueFamilyIndex.presentIndex.has_value() && (queueFamilyIndex.presentIndex.value()!=queueFamilyIndex.graphicsIndex.value()))
    {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setPNext(nullptr)
                       .setPQueuePriorities(&queuePriorties[1])
                       .setQueueCount(1)
                       .setQueueFamilyIndex(queueFamilyIndex.presentIndex.value());
        queueCreateInfos.push_back(queueCreateInfo);
    }
    if(queueFamilyIndex.computeIndex.has_value() && (queueFamilyIndex.computeIndex.value()!=queueFamilyIndex.presentIndex.value()) && (queueFamilyIndex.computeIndex.value()!=queueFamilyIndex.graphicsIndex.value()))
    {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setPNext(nullptr)
                       .setPQueuePriorities(&queuePriorties[2])
                       .setQueueCount(1)
                       .setQueueFamilyIndex(queueFamilyIndex.computeIndex.value());
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    /*3.指定逻辑设备所需的物理设备特性（使用所有特性）*/
    vk::PhysicalDeviceFeatures deviceFeatures = physicalDevice.getFeatures();
    
    /*4.指定逻辑设备所需拓展*/
    std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    /*5.指定逻辑设备所需层（使用与实例相同的验证层）*/
    
    /*创建逻辑设备*/
    vk::DeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.setPNext(nullptr)
                    .setQueueCreateInfoCount(queueCreateInfos.size())
                    .setQueueCreateInfos(queueCreateInfos)
                    .setPEnabledFeatures(&deviceFeatures)
                    .setEnabledExtensionCount(deviceExtensions.size())
                    .setPEnabledExtensionNames(deviceExtensions)
                    .setEnabledLayerCount(m_layers.size())
                    .setPEnabledLayerNames(m_layers);
    return physicalDevice.createDevice(deviceCreateInfo);
}

void VkBase::initSwapchain()
{
    swapchain = std::make_unique<Swapchain>(m_surface); 
}

void VkBase::initShaderModules(const std::string& vertexFile, const std::string& fragmentFile)
{
    std::vector<char> vertexSource = utils::readFile(vertexFile);
    std::vector<char> fragmentSource = utils::readFile(fragmentFile);
    shader = std::make_unique<Shader>(vertexSource, fragmentSource);
}

void VkBase::initRenderProcess()
{
    renderProcess = std::make_unique<RenderProcess>();
}

void VkBase::initPipeline()
{
    renderProcess->graphicsPipeline_triangle = renderProcess->createGraphicsPipeline(*shader, vk::PrimitiveTopology::eTriangleList);
}

void VkBase::initCommandManager()
{
    commandManager = std::make_unique<CommandManager>();
}

void VkBase::initDescriptorManager()
{
    descriptorManager = std::make_unique<DescriptorManager>(swapchain->images.size());
}


void VkBase::initVertexBuffer()
{
    size_t bufferSize = sizeof(vertices[0])*vertices.size();
    /*1.创建临时CPU内存，并将顶点数据通过映射保存到该临时内存中*/
    Buffer tempBuffer(vk::BufferUsageFlagBits::eTransferSrc, bufferSize, 
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);
    memcpy(tempBuffer.data, (void *)vertices.data(), tempBuffer.size);    /*保存顶点数据至映射内存中*/
    device.unmapMemory(tempBuffer.memory);
    /*2.创建顶点内存（gpu高效内存）*/
    vertexBuffer = std::make_unique<Buffer>(vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eTransferDst, bufferSize,
                                            vk::MemoryPropertyFlagBits::eDeviceLocal);
    /*3.拷贝临时内存至顶点内存*/
    Commander().copyBuffer(tempBuffer.buffer, vertexBuffer->buffer, bufferSize);
}

void VkBase::initIndexBuffer()
{
    size_t bufferSize = sizeof(indices[0])*indices.size();
    /*1.创建临时CPU内存，并将顶点数据通过映射保存到该临时内存中*/
    Buffer tempBuffer(vk::BufferUsageFlagBits::eTransferSrc, bufferSize, 
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);
    memcpy(tempBuffer.data, (void *)indices.data(), tempBuffer.size);    /*保存顶点数据至映射内存中*/
    device.unmapMemory(tempBuffer.memory);
    /*2.创建顶点内存（gpu高效内存）*/
    indexBuffer = std::make_unique<Buffer>(vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst, bufferSize,
                                            vk::MemoryPropertyFlagBits::eDeviceLocal);
    /*3.拷贝临时内存至顶点内存*/
    Commander().copyBuffer(tempBuffer.buffer, indexBuffer->buffer, bufferSize);
}

void VkBase::initUniformBuffers()
{
    int swapchianSize = swapchain->images.size();
    uniformBuffers.resize(swapchianSize);
    for(int i=0; i<swapchianSize; i++)
        uniformBuffers[i] = std::make_unique<Buffer>(vk::BufferUsageFlagBits::eUniformBuffer, sizeof(UniformBufferObject), vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);
}

void VkBase::updateUniformBuffers(int currentFrame)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(1.0f), time*glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
    ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapchain->getExtent().width/(float)swapchain->getExtent().height, 0.1f, 20.0f);
    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffers[currentFrame]->data, (void*)&ubo, sizeof(ubo));
}


void VkBase::initRenderer()
{
    renderer = std::make_unique<Renderer>(5);
}

void VkBase::recreateSwapchain()
{
    /*0.等待设备处于空闲状态*/
    device.waitIdle();

    /*1.在重建前，销毁有关对象*/
    commandManager->freeCommandBuffers();
    swapchain.reset();
    device.destroyPipeline(renderProcess->graphicsPipeline_triangle);
    renderProcess.reset();

    /*2.重建交换链相关对象*/
    m_surface = m_getSurfaceCallback(instance);
    initSwapchain();
    initRenderProcess();
    initPipeline();
    swapchain->initFramebuffers();
    auto& commandBuffers = renderer->getCommandBuffers();
    commandBuffers = commandManager->allocateCommandBuffers(renderer->getFlightCount());
}


void VkBase::createTextureImage()
{
    /*1.使用stb_image载入纹理图像*/
    int texW, texH, texCh;
    stbi_uc* pixels = stbi_load("C:/VSCode_files/vulkan2D/texture/f27-2.jpg", &texW, &texH, &texCh, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texW * texH * 4;
    if(!pixels)
        throw std::runtime_error("failed to load texture image!");
    /*2.暂存缓冲*/
    Buffer tempBuffer(vk::BufferUsageFlagBits::eTransferSrc, imageSize, 
                        vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);
    memcpy(tempBuffer.data, pixels, static_cast<size_t>(tempBuffer.size));
    device.unmapMemory(tempBuffer.memory);
    stbi_image_free(pixels);
    /*3.创建纹理图像对象*/
    vk::Extent3D extent = {static_cast<uint32_t>(texW), static_cast<uint32_t>(texH), 1};
    vk::ImageCreateInfo createInfo = {};
    createInfo.setImageType(vk::ImageType::e2D)                 /*设置图像对象类型为2D*/
              .setExtent(extent)                       /*设置图像大小（2D深度为1）*/
              .setMipLevels(1)                                  /*mipmap仅自身*/
              .setArrayLayers(1)                                /*纹理数组仅自身*/
              .setFormat(vk::Format::eR8G8B8A8Snorm)            /*设置图像格式与stbi_load一致*/
              .setTiling(vk::ImageTiling::eOptimal)             /*设置像素如何排列*/
              .setInitialLayout(vk::ImageLayout::eUndefined)    /*设置初始化布局*/
              .setUsage(vk::ImageUsageFlagBits::eTransferDst|vk::ImageUsageFlagBits::eSampled)  /*设置图像对象用途*/
              .setSharingMode(vk::SharingMode::eExclusive)      /*设置共享模式为队列独有*/
              .setSamples(vk::SampleCountFlagBits::e1);         /*设置采样数为1*/
    textureImage = device.createImage(createInfo);
    if(!textureImage)
        throw std::runtime_error("failed to create image!");
    /*4.向纹理图像对象分配并绑定内存*/
    vk::MemoryRequirements requirements = device.getImageMemoryRequirements(textureImage);
    vk::MemoryAllocateInfo allocateInfo = {};
    allocateInfo.setMemoryTypeIndex(findMemoryType(requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal))
                .setAllocationSize(requirements.size);
    textureImageMemory = device.allocateMemory(allocateInfo);
    if(!textureImageMemory)
        throw std::runtime_error("failed to allocate image memory!");
    device.bindImageMemory(textureImage, textureImageMemory, 0);
    /*5.图像对象布局变换*/
    Commander cmder;
    cmder.transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Snorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    /*6.复制内存缓冲数据至图像对象*/
    cmder.copyBuffer(tempBuffer.buffer, textureImage, static_cast<uint32_t>(texW), static_cast<uint32_t>(texH));
    /*7.将图像对象布局转换成在shader中能够采样的布局*/
    cmder.transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Snorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}


}

