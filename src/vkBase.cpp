#include "vkBase.hpp"

namespace vulkan2d{

VkBase* VkBase::m_self_instance = nullptr;
#ifdef NDEBUG
    bool VkBase::m_enableValidationLayer = false;
#else
    bool VkBase::m_enableValidationLayer = true;
#endif

void VkBase::init(const std::vector<const char*>& extensions_)
{
    m_self_instance = new VkBase(extensions_);
}

VkBase::VkBase(const std::vector<const char*>& extensions_)
{
    extensions = extensions_;
    if(m_enableValidationLayer)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }
    /*1.创建vulkan实例*/
    instance = createInstance();
    if(!instance)
        throw std::runtime_error("[ Instance ]: Can't create vulkan instance!");
#ifndef NDEBUG
    /*2.创建debuger*/
    debugMessenger = createDebugMessenger();
    if(!debugMessenger)
        throw std::runtime_error("[ DebugMessenger ]: Can't create  debug messenger!");
#endif
    /*3.创建物理设备*/
    physicalDevice = pickPhysicalDevice();
    if(!physicalDevice)
        throw std::runtime_error("[ physicalDevice ]: Can't create  physical device!");

}

void VkBase::destroy()
{
    delete m_self_instance;
}

VkBase::~VkBase()
{

    
#ifndef NDEBUG
    instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, loadInstanceDynamicLoader());
#endif
    instance.destroy();
}

vk::Instance VkBase::createInstance()
{
    vk::ApplicationInfo appInfo = {};
    appInfo.setPEngineName("vulkan2D")
           .setPNext(nullptr)
           .setPApplicationName("vulkan2D")
           .setApplicationVersion(vk::makeVersion(1, 0, 0))
           .setEngineVersion(vk::makeVersion(1, 0, 0))
           .setApiVersion(VK_API_VERSION_1_3);

    vk::InstanceCreateInfo createInfo = {};
    createInfo.setPNext(nullptr)
              .setEnabledExtensionCount(uint32_t(extensions.size()))
              .setPEnabledExtensionNames(extensions)
              .setEnabledLayerCount(layers.size())
              .setPEnabledLayerNames(layers)
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


}

